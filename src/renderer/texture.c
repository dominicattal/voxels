#include "texture.h"
#include "shader.h"
#include "ssbo.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <stb_rect_pack.h>
#include <stb_truetype.h>
#include <math.h>
#include <string.h>
#include <glad.h>

#define NUM_TEXTURE_UNITS  16

#define CHAR_OFFSET 32
#define NUM_CHARS   96
#define BITMAP_WIDTH  1024
#define BITMAP_HEIGHT 1024
#define PADDING 0

typedef struct {
    i16 font_size;
    i32 ascent, descent, line_gap;
    struct {
        i32 advance, left_side_bearing;
        i32 kern[NUM_CHARS];
        i32 x1, y1, x2, y2;
        u16 u1, v1, u2, v2;
    } chars[NUM_CHARS];
} FontInfo;

static FontInfo fonts[NUM_FONTS];

typedef struct {
    Texture tex;
    char* path;
} Image;

typedef struct {
    f32 u, v, w, h;
    i32 location;
} TEX;

#define NUM_IMAGES_TO_PACK 5
static Image images[NUM_IMAGES_TO_PACK] = {
    (Image) { TEX_COLOR, "assets/textures/color.png" },
    (Image) { TEX_GRASS_TOP, "assets/textures/blocks/grass_top.png" },
    (Image) { TEX_GRASS_SIDE, "assets/textures/blocks/grass_side.png" },
    (Image) { TEX_GRASS_BOTTOM, "assets/textures/blocks/grass_bottom.png" },
    (Image) { TEX_STONE, "assets/textures/blocks/stone.png" }
};

static u32 texture_units[NUM_TEXTURE_UNITS];

static void load_font(stbtt_pack_context* spc, Font font, i32 font_size, const char* ttf_path)
{
    unsigned char* font_buffer;
    stbtt_fontinfo info;
    stbtt_pack_range font_range;
    stbtt_packedchar chars[NUM_CHARS];
    i64 size;
    f32 scale;

    FILE* font_file = fopen(ttf_path, "rb");
    fseek(font_file, 0, SEEK_END);
    size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);
    font_buffer = malloc(size);
    fread(font_buffer, size, 1, font_file);
    fclose(font_file);
    
    stbtt_InitFont(&info, font_buffer, 0);

    font_range.font_size = font_size;         
    font_range.first_unicode_codepoint_in_range = CHAR_OFFSET; 
    font_range.array_of_unicode_codepoints = NULL;
    font_range.num_chars = NUM_CHARS;       
    font_range.chardata_for_range = chars;

    stbtt_PackFontRanges(spc, font_buffer, 0, &font_range, 1);

    scale = stbtt_ScaleForPixelHeight(&info, font_size);
    stbtt_GetFontVMetrics(&info, &fonts[font].ascent, &fonts[font].descent, &fonts[font].line_gap);
    fonts[font].ascent = roundf(fonts[font].ascent * scale);
    fonts[font].descent = roundf(fonts[font].descent * scale);
    fonts[font].line_gap = roundf(fonts[font].line_gap * scale);
    fonts[font].font_size = font_size;
    for (i32 i = 0; i < NUM_CHARS; i++) {
        fonts[font].chars[i].u1 = chars[i].x0;
        fonts[font].chars[i].v1 = chars[i].y0;
        fonts[font].chars[i].u2 = chars[i].x1;
        fonts[font].chars[i].v2 = chars[i].y1;
        stbtt_GetCodepointHMetrics(&info, i+CHAR_OFFSET, 
                                   &fonts[font].chars[i].advance, 
                                   &fonts[font].chars[i].left_side_bearing);
        fonts[font].chars[i].advance = roundf(fonts[font].chars[i].advance * scale);
        fonts[font].chars[i].left_side_bearing = roundf(fonts[font].chars[i].left_side_bearing * scale);
        stbtt_GetCodepointBitmapBox(&info, i+CHAR_OFFSET, scale, scale, 
                                    &fonts[font].chars[i].x1,  
                                    &fonts[font].chars[i].y1,  
                                    &fonts[font].chars[i].x2,  
                                    &fonts[font].chars[i].y2);
        for (i32 j = 0; j < NUM_CHARS; j++) {
            fonts[font].chars[i].kern[j] = stbtt_GetCodepointKernAdvance(&info, i+CHAR_OFFSET, j+CHAR_OFFSET);
            fonts[font].chars[i].kern[j] = roundf(fonts[font].chars[i].kern[j] * scale);
        }
    }

    free(font_buffer);
}

void font_info(Font font, i32 font_size, i32* ascent, i32* descent, i32* line_gap)
{
    f32 scale = (f32)font_size / fonts[font].font_size;
    *ascent   = roundf(fonts[font].ascent   * scale);
    *descent  = roundf(fonts[font].descent  * scale);
    *line_gap = roundf(fonts[font].line_gap * scale);
}

void font_char_hmetrics(Font font, i32 font_size, char character, i32* advance, i32* left_side_bearing)
{
    f32 scale = (f32)font_size / fonts[font].font_size;
    *advance = roundf(fonts[font].chars[character-CHAR_OFFSET].advance * scale);
    *left_side_bearing = roundf(fonts[font].chars[character-CHAR_OFFSET].left_side_bearing * scale);
}

void font_char_bbox(Font font, i32 font_size, char character, i32* bbox_x1, i32* bbox_y1, i32* bbox_x2, i32* bbox_y2)
{
    f32 scale = (f32)font_size / fonts[font].font_size;
    *bbox_x1 = roundf(fonts[font].chars[character-CHAR_OFFSET].x1 * scale);
    *bbox_y1 = roundf(fonts[font].chars[character-CHAR_OFFSET].y1 * scale);
    *bbox_x2 = roundf(fonts[font].chars[character-CHAR_OFFSET].x2 * scale);
    *bbox_y2 = roundf(fonts[font].chars[character-CHAR_OFFSET].y2 * scale);
}

void font_char_bmap(Font font, i32 font_size, char character, f32* bmap_u1, f32* bmap_v1, f32* bmap_u2, f32* bmap_v2)
{
    *bmap_u1 = (f32)(fonts[font].chars[character-CHAR_OFFSET].u1) / BITMAP_WIDTH;
    *bmap_v1 = (f32)(fonts[font].chars[character-CHAR_OFFSET].v1) / BITMAP_HEIGHT;
    *bmap_u2 = (f32)(fonts[font].chars[character-CHAR_OFFSET].u2) / BITMAP_WIDTH;
    *bmap_v2 = (f32)(fonts[font].chars[character-CHAR_OFFSET].v2) / BITMAP_HEIGHT;
}

void font_char_kern(Font font, i32 font_size, char character, char next_character, i32* kern)
{
    f32 scale = (f32)font_size / fonts[font].font_size;
    *kern = roundf(fonts[font].chars[character-CHAR_OFFSET].kern[next_character-CHAR_OFFSET] * scale);
}

static void create_font_textures(i32* tex_unit_location, TEX* textures) 
{
    stbtt_pack_context spc;
    unsigned char* bitmap;
    u32 tex;

    bitmap = calloc(BITMAP_WIDTH * BITMAP_HEIGHT, sizeof(unsigned char));
    stbtt_PackBegin(&spc, bitmap, BITMAP_WIDTH, BITMAP_HEIGHT, 0, 1, NULL);

    load_font(&spc, FONT_DEFAULT, 16, "assets/fonts/mojangles.ttf");
    load_font(&spc, FONT_TWO, 16, "assets/fonts/times.ttf");
    load_font(&spc, FONT_MONOSPACE, 16, "assets/fonts/consola.ttf");

    stbtt_PackEnd(&spc);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, BITMAP_WIDTH, BITMAP_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    i32 swizzle_mask[] = {GL_ZERO, GL_ZERO, GL_ZERO, GL_RED};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle_mask);

    texture_units[*tex_unit_location] = tex;
    glActiveTexture(GL_TEXTURE0 + *tex_unit_location);
    glBindTexture(GL_TEXTURE_2D, texture_units[*tex_unit_location]);

    i32 idx;
    for (i32 font = 0; font < NUM_FONTS; font++) {
        for (i32 i = 0; i < NUM_CHARS; i++) {
            idx = NUM_TEXTURES + font * NUM_CHARS + i;
            textures[idx].u = (f32)(fonts[font].chars[i].u1) / BITMAP_WIDTH;
            textures[idx].v = (f32)(fonts[font].chars[i].v1) / BITMAP_WIDTH;
            textures[idx].w = (f32)(fonts[font].chars[i].u2 - fonts[font].chars[i].u1) / BITMAP_WIDTH;
            textures[idx].h = (f32)(fonts[font].chars[i].v2 - fonts[font].chars[i].v1) / BITMAP_WIDTH;
            textures[idx].location = *tex_unit_location;
        }
    }

    if (ENV_EXPORT_TEXTURE_ATLASES) {
        char path[512];
        sprintf(path, "data/packed%d.png", *tex_unit_location);
        stbi_write_png(path, BITMAP_WIDTH, BITMAP_HEIGHT, 1, bitmap, 0);
    }

    free(bitmap);
    (*tex_unit_location)++;
}

static void initialize_rects(TEX* textures, stbrp_rect* rects_rgb, stbrp_rect* rects_rgba, unsigned char** image_data, i32* num_rects_rgb_out, i32* num_rects_rgba_out)
{
    register i32 num_rects_rgb, num_rects_rgba, i;
    i32 width, height, num_channels;
    num_rects_rgb = num_rects_rgba = 0;
    for (i = 0; i < NUM_IMAGES_TO_PACK; i++) {
        image_data[i] = stbi_load(images[i].path, &width, &height, &num_channels, 0);
        if (num_channels == 3) {
            load_rgb:
            rects_rgb[num_rects_rgb].id = images[i].tex;
            rects_rgb[num_rects_rgb].w  = PADDING + width;
            rects_rgb[num_rects_rgb].h  = PADDING + height;
            textures[images[i].tex].location = i;
            ++num_rects_rgb;
        } else if (num_channels == 4) {
            rects_rgba[num_rects_rgba].id = images[i].tex;
            rects_rgba[num_rects_rgba].w  = PADDING + width;
            rects_rgba[num_rects_rgba].h  = PADDING + height;
            textures[images[i].tex].location = i;
            ++num_rects_rgba;
        } else {
            printf("Unsupported number of channels (%d) for image %d: %s\n", num_channels, images[i].tex, images[i].path);
            printf("Defaulting to 3 channels\n");
            stbi_image_free(image_data[i]);
            image_data[i] = stbi_load(images[i].path, &width, &height, &num_channels, 3);
            goto load_rgb;
        }
    }
    *num_rects_rgb_out  = num_rects_rgb;
    *num_rects_rgba_out = num_rects_rgba;
}

static void pack_textures(i32* tex_unit_location, TEX* textures, unsigned char** image_data, stbrp_rect* rects, i32 num_rects, i32 num_channels)
{
    i32 num_nodes, num_rects_packed;
    i32 y, x, c, data_idx, bitmap_idx;
    i32 location, new_rect_idx;
    i32 all_rects_packed;
    u32 tex;
    unsigned char* bitmap;
    stbrp_context* context;
    stbrp_node* nodes;

    num_nodes = BITMAP_WIDTH;

    context  = malloc(sizeof(stbrp_context));
    nodes    = malloc(sizeof(stbrp_node) * num_nodes);

    stbrp_init_target(context, BITMAP_WIDTH, BITMAP_HEIGHT, nodes, num_nodes);
    all_rects_packed = stbrp_pack_rects(context, rects, num_rects);

    num_rects_packed = 0;
    location = *tex_unit_location;
    bitmap = calloc(BITMAP_WIDTH * BITMAP_HEIGHT * num_channels, sizeof(unsigned char));
    for (i32 i = 0; i < num_rects; ++i) {
        if (!rects[i].was_packed)
            continue;
        ++num_rects_packed;
        for (y = 0; y < rects[i].h - PADDING; ++y) {
            for (x = 0; x < rects[i].w - PADDING; ++x) {
                for (c = 0; c < num_channels; ++c) {
                    stbrp_rect rect = rects[i];

                    data_idx   =    y * num_channels * (rect.w - PADDING)
                                 +  x * num_channels 
                                 +  c;

                    bitmap_idx =   (y + rect.y) * num_channels * BITMAP_WIDTH
                                 + (x + rect.x) * num_channels
                                 +  c;

                    bitmap[bitmap_idx] = image_data[textures[rect.id].location][data_idx];
                }
            }
        }
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &tex);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(tex, 1, (num_channels == 3) ? GL_RGB8 : GL_RGBA8, BITMAP_WIDTH, BITMAP_HEIGHT);
    glTextureSubImage2D(tex, 0, 0, 0, BITMAP_WIDTH, BITMAP_HEIGHT, (num_channels == 3) ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
    glActiveTexture(GL_TEXTURE0 + location);
    glBindTexture(GL_TEXTURE_2D, tex);

    texture_units[location] = tex;
    new_rect_idx = 0;
    for (i32 i = 0; i < num_rects; ++i) {
        stbrp_rect rect = rects[i];
        if (!rect.was_packed) {
            rects[new_rect_idx++] = rect;
            continue;
        }
        textures[rect.id].u = (f32)rect.x / BITMAP_WIDTH;
        textures[rect.id].v = (f32)rect.y / BITMAP_HEIGHT;
        textures[rect.id].w = (f32)(rect.w-PADDING) / BITMAP_WIDTH;
        textures[rect.id].h = (f32)(rect.h-PADDING) / BITMAP_HEIGHT;
        textures[rect.id].location = location;
    }

    if (ENV_EXPORT_TEXTURE_ATLASES) {
        char path[512];
        sprintf(path, "data/packed%d.png", *tex_unit_location);
        stbi_write_png(path, BITMAP_WIDTH, BITMAP_HEIGHT, num_channels, bitmap, 0);
    }

    (*tex_unit_location)++;
    free(context);
    free(nodes);
    free(bitmap);

    if (!all_rects_packed) {
        if (*tex_unit_location == NUM_TEXTURE_UNITS) {
            puts("Out of texture units to pack to");
            exit(1);
        }
        pack_textures(tex_unit_location, textures, image_data, rects, new_rect_idx, num_channels);
    }
}


void texture_init(void)
{
    TEX* textures;
    size_t textures_size;
    i32 tex_unit_location;
    stbrp_rect* rects_rgb;
    stbrp_rect* rects_rgba;
    unsigned char** image_data;
    i32 num_rects_rgb, num_rects_rgba;

    textures_size = sizeof(TEX) * (NUM_TEXTURES + NUM_FONTS * NUM_CHARS);
    tex_unit_location = 0;
    
    textures = malloc(textures_size);
    create_font_textures(&tex_unit_location, textures);

    rects_rgb  = malloc(sizeof(stbrp_rect) * NUM_IMAGES_TO_PACK);
    rects_rgba = malloc(sizeof(stbrp_rect) * NUM_IMAGES_TO_PACK);
    image_data = malloc(sizeof(unsigned char*) * NUM_IMAGES_TO_PACK);

    initialize_rects(textures, rects_rgb, rects_rgba, image_data, &num_rects_rgb, &num_rects_rgba);

    pack_textures(&tex_unit_location, textures, image_data, rects_rgb, num_rects_rgb, 3);
    pack_textures(&tex_unit_location, textures, image_data, rects_rgba, num_rects_rgba, 4);

    for (i32 i = 0; i < NUM_IMAGES_TO_PACK; i++)
        stbi_image_free(image_data[i]);

    free(image_data);
    free(rects_rgb);
    free(rects_rgba);

    i32 texs[NUM_TEXTURE_UNITS];
    for (i32 i = 0; i < NUM_TEXTURE_UNITS; ++i)
        texs[i] = i;

    shader_use(SHADER_GUI);
    glUniform1iv(shader_get_uniform_location(SHADER_GUI, "textures"), NUM_TEXTURE_UNITS, texs);
    shader_use(SHADER_GAME);
    glUniform1iv(shader_get_uniform_location(SHADER_GAME, "textures"), NUM_TEXTURE_UNITS, texs);

    ssbo_bind(SSBO_TEXTURE);
    ssbo_malloc(SSBO_TEXTURE, textures_size, GL_STATIC_DRAW);
    ssbo_update(SSBO_TEXTURE, 0, textures_size, textures);
    ssbo_bind_buffer_base(SSBO_TEXTURE, 1);

    free(textures);
}

void texture_destroy(void)
{
    glDeleteTextures(NUM_TEXTURE_UNITS, texture_units);
}

u32 texture_font_id(Font font, char character)
{
    return NUM_TEXTURES + font * NUM_CHARS + character - CHAR_OFFSET;
}
