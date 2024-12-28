#include "texture.h"
#include "../font/font.h"
#include "shader.h"
#include "ssbo.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <stb_rect_pack.h>

#define NUM_TEXTURE_UNITS  16

#define BITMAP_WIDTH  1024
#define BITMAP_HEIGHT 1024
#define PADDING 0

typedef struct {
    Texture tex;
    char* path;
} Image;

typedef struct {
    f32 u, v, w, h;
    i32 location;
} TEX;

#define NUM_IMAGES_TO_PACK 4
static Image images[NUM_IMAGES_TO_PACK] = {
    (Image) { TEX_OBJECT1, "assets/textures/objects/object1.png" },
    (Image) { TEX_OBJECT2, "assets/textures/objects/object2.png" },
    (Image) { TEX_OBJECT3, "assets/textures/objects/object3.png" },
    (Image) { TEX_OBJECT4, "assets/textures/objects/object4.png" }
};

static u32 texture_units[NUM_TEXTURE_UNITS];

static void create_font_textures(i32* tex_unit_location) 
{
    texture_units[*tex_unit_location] = font_init();
    glActiveTexture(GL_TEXTURE0 + *tex_unit_location);
    glBindTexture(GL_TEXTURE_2D, texture_units[*tex_unit_location]);
    glUniform1i(shader_get_uniform_location(SHADER_GUI, "TEX_ID_FONT"), *tex_unit_location);
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
        }
    }
    *num_rects_rgb_out  = num_rects_rgb;
    *num_rects_rgba_out = num_rects_rgba;
}

static void pack_textures(i32* tex_unit_location, TEX* textures, unsigned char** image_data, stbrp_rect* rects, i32 num_rects, i32 num_channels)
{
    i32 num_nodes, num_rects_packed;
    i32 y, x, c, data_idx, bitmap_idx;
    i32 location, uv_idx, new_rect_idx;
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
        // height, width, channels, index in data, index in bitmap
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

    uv_idx = 0;

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
        ++uv_idx;
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
    TEX textures[NUM_TEXTURES];

    i32 tex_unit_location;
    tex_unit_location = 0;

    create_font_textures(&tex_unit_location);
    
    stbrp_rect* rects_rgb;
    stbrp_rect* rects_rgba;
    unsigned char** image_data;
    i32 num_rects_rgb, num_rects_rgba;

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
    ssbo_malloc(SSBO_TEXTURE, sizeof(textures), GL_STATIC_DRAW);
    ssbo_update(SSBO_TEXTURE, 0, sizeof(textures), textures);
    ssbo_bind_buffer_base(SSBO_TEXTURE, 1);
}

void texture_destroy(void)
{
    font_destroy();
    glDeleteTextures(NUM_TEXTURE_UNITS, texture_units);
}