#include "font.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <glad.h>
#include <stb_rect_pack.h>
#include <stb_truetype.h>

#define BITMAP_WIDTH    1024
#define BITMAP_HEIGHT   1024

#define CHAR_OFFSET     32
#define NUM_CHARS       96

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

u32 font_init(void)
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
    
    if (ENV_EXPORT_TEXTURE_ATLASES)
        stbi_write_png("data/font_bitmap.png", BITMAP_WIDTH, BITMAP_HEIGHT, 1, bitmap, 0);

    free(bitmap);
    return tex;
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

void font_destroy(void)
{
}
