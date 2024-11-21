#include "font.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <glad.h>
#include <stb_image_write.h>

Font font;

void font_init(void)
{
    long size;

    font.bitmap = calloc(BITMAP_WIDTH * BITMAP_HEIGHT, sizeof(unsigned char));

    FILE* font_file = fopen("assets/times.ttf", "rb");
    fseek(font_file, 0, SEEK_END);
    size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);
    font.font_buffer = malloc(size);
    fread(font.font_buffer, size, 1, font_file);
    fclose(font_file);
    
    stbtt_InitFont(&font.info, font.font_buffer, 0);

    stbtt_PackBegin(&font.spc, font.bitmap, BITMAP_WIDTH, BITMAP_HEIGHT, 0, 1, NULL);

    font.fontRange.font_size = 32.0f;         
    font.fontRange.first_unicode_codepoint_in_range = CHAR_OFFSET; 
    font.fontRange.array_of_unicode_codepoints = NULL;
    font.fontRange.num_chars = NUM_CHARS;       
    font.fontRange.chardata_for_range = font.packedChars;

    stbtt_PackFontRanges(&font.spc, font.font_buffer, 0, &font.fontRange, 1);
    stbtt_PackEnd(&font.spc);

    stbi_write_png("data/out.png", BITMAP_WIDTH, BITMAP_HEIGHT, 1, font.bitmap, BITMAP_WIDTH);

}

void font_info(FontID id, i32 font_size, i32* ascent, i32* descent, i32* line_gap)
{
    f32 scale = stbtt_ScaleForPixelHeight(&font.info, font_size);
    stbtt_GetFontVMetrics(&font.info, ascent, descent, line_gap);
    *ascent   = roundf(*ascent   * scale);
    *descent  = roundf(*descent  * scale);
    *line_gap = roundf(*line_gap * scale);
}

void font_char_hmetrics(FontID id, i32 font_size, char character, i32* advance, i32* left_side_bearing)
{
    f32 scale = stbtt_ScaleForPixelHeight(&font.info, font_size);
    stbtt_GetCodepointHMetrics(&font.info, character, advance, left_side_bearing);
    *advance = roundf(*advance * scale);
    *left_side_bearing = roundf(*left_side_bearing * scale);
}

void font_char_bbox(FontID id, i32 font_size, char character, i32* bbox_x1, i32* bbox_y1, i32* bbox_x2, i32* bbox_y2)
{
    f32 scale = stbtt_ScaleForPixelHeight(&font.info, font_size);
    stbtt_GetCodepointBitmapBox(&font.info, character, scale, scale, bbox_x1, bbox_y1, bbox_x2, bbox_y2);
}

void font_char_bmap(FontID id, i32 font_size, char character, f32* bmap_u1, f32* bmap_v1, f32* bmap_u2, f32* bmap_v2)
{
    stbtt_packedchar b = font.packedChars[character-CHAR_OFFSET];
    *bmap_u1 = (f32)b.x0 / BITMAP_WIDTH;
    *bmap_v1 = (f32)b.y0 / BITMAP_HEIGHT;
    *bmap_u2 = (f32)b.x1 / BITMAP_WIDTH;
    *bmap_v2 = (f32)b.y1 / BITMAP_HEIGHT;
}

void font_char_kern(FontID id, i32 font_size, char character, char next_character, i32* kern)
{
    f32 scale = stbtt_ScaleForPixelHeight(&font.info, font_size);
    *kern = stbtt_GetCodepointKernAdvance(&font.info, character, next_character);
    *kern = roundf(*kern * scale);
}

void font_destroy(void)
{
    free(font.bitmap);
    free(font.font_buffer);
}
