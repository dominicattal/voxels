#include "font.h"
#include "../renderer/renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stb_image_write.h>
#include <stb_rect_pack.h>
#include <stb_truetype.h>

#define BITMAP_WIDTH    (1<<9)
#define BITMAP_HEIGHT   (1<<9)
#define CHAR_OFFSET     32
#define NUM_CHARS       96

typedef struct {
    unsigned char* font_buffer;
    stbtt_fontinfo info;
    stbtt_pack_range font_range;
    stbtt_packedchar chars[NUM_CHARS];
} Font;

static Font fonts[NUM_FONTS];
static stbtt_pack_context spc;

static void load_font(FontID id, const char* ttf_path)
{
    long size;
    FILE* font_file = fopen(ttf_path, "rb");
    fseek(font_file, 0, SEEK_END);
    size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);
    fonts[id].font_buffer = malloc(size);
    fread(fonts[id].font_buffer, size, 1, font_file);
    fclose(font_file);
    
    stbtt_InitFont(&fonts[id].info, fonts[id].font_buffer, 0);

    fonts[id].font_range.font_size = 24;         
    fonts[id].font_range.first_unicode_codepoint_in_range = CHAR_OFFSET; 
    fonts[id].font_range.array_of_unicode_codepoints = NULL;
    fonts[id].font_range.num_chars = NUM_CHARS;       
    fonts[id].font_range.chardata_for_range = fonts[id].chars;

    stbtt_PackFontRanges(&spc, fonts[id].font_buffer, 0, &fonts[id].font_range, 1);
}

void font_init(void)
{
    unsigned char* bitmap = calloc(BITMAP_WIDTH * BITMAP_HEIGHT, sizeof(unsigned char));
    stbtt_PackBegin(&spc, bitmap, BITMAP_WIDTH, BITMAP_HEIGHT, 0, 1, NULL);

    load_font(FONT_DEFAULT, "assets/mojangles.ttf");
    load_font(FONT_TWO, "assets/HelvetiPixel.ttf");

    stbtt_PackEnd(&spc);
    stbi_write_png("data/out.png", BITMAP_WIDTH, BITMAP_HEIGHT, 1, bitmap, BITMAP_WIDTH);
    renderer_create_font_bitmap(BITMAP_WIDTH, BITMAP_HEIGHT, bitmap);
    free(bitmap);
}

void font_info(FontID id, i32 font_size, i32* ascent, i32* descent, i32* line_gap)
{
    f32 scale = stbtt_ScaleForPixelHeight(&fonts[id].info, font_size);
    stbtt_GetFontVMetrics(&fonts[id].info, ascent, descent, line_gap);
    *ascent   = roundf(*ascent   * scale);
    *descent  = roundf(*descent  * scale);
    *line_gap = roundf(*line_gap * scale);
}

void font_char_hmetrics(FontID id, i32 font_size, char character, i32* advance, i32* left_side_bearing)
{
    f32 scale = stbtt_ScaleForPixelHeight(&fonts[id].info, font_size);
    stbtt_GetCodepointHMetrics(&fonts[id].info, character, advance, left_side_bearing);
    *advance = roundf(*advance * scale);
    *left_side_bearing = roundf(*left_side_bearing * scale);
}

void font_char_bbox(FontID id, i32 font_size, char character, i32* bbox_x1, i32* bbox_y1, i32* bbox_x2, i32* bbox_y2)
{
    f32 scale = stbtt_ScaleForPixelHeight(&fonts[id].info, font_size);
    stbtt_GetCodepointBitmapBox(&fonts[id].info, character, scale, scale, bbox_x1, bbox_y1, bbox_x2, bbox_y2);
}

void font_char_bmap(FontID id, i32 font_size, char character, f32* bmap_u1, f32* bmap_v1, f32* bmap_u2, f32* bmap_v2)
{
    stbtt_packedchar b = fonts[id].chars[character-CHAR_OFFSET];
    *bmap_u1 = (f32)(b.x0) / BITMAP_WIDTH;
    *bmap_v1 = (f32)(b.y0) / BITMAP_HEIGHT;
    *bmap_u2 = (f32)(b.x1) / BITMAP_WIDTH;
    *bmap_v2 = (f32)b.y1 / BITMAP_HEIGHT;
}

void font_char_kern(FontID id, i32 font_size, char character, char next_character, i32* kern)
{
    f32 scale = stbtt_ScaleForPixelHeight(&fonts[id].info, font_size);
    *kern = stbtt_GetCodepointKernAdvance(&fonts[id].info, character, next_character);
    *kern = roundf(*kern * scale);
}

void font_destroy(void)
{
    for (i32 i = 0; i < NUM_FONTS; i++)
        free(fonts[i].font_buffer);
}
