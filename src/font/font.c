#include "font.h"
#include "../renderer/renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stb_image_write.h>
#include <stb_rect_pack.h>
#include <stb_truetype.h>

#define BITMAP_WIDTH    (1<<10)
#define BITMAP_HEIGHT   (1<<10)
#define CHAR_OFFSET     32
#define NUM_CHARS       96

#define MIN_FONT_SIZE 7
#define MAX_FONT_SIZE 100
#define FONT_SIZE_RANGE (MAX_FONT_SIZE - MIN_FONT_SIZE + 1)

#define NUM_FONT_SIZES 15

typedef struct {
    unsigned char* font_buffer;
    stbtt_fontinfo info;
    stbtt_pack_range font_range[NUM_FONT_SIZES];
    stbtt_packedchar chars[NUM_FONT_SIZES][NUM_CHARS];
} Font;

// font_size to bucket
static i32 font_size_map[FONT_SIZE_RANGE];

// bucket to font_size
static i32 font_sizes[NUM_FONT_SIZES];

static Font fonts[NUM_FONTS];
static stbtt_pack_context spc;

static void init_font_sizes(void);
static void init_font_size_map(void);
static int  map_font_size(i32 font_size);
static void load_font(FontID id, const char* ttf_path);

void font_init(void)
{
    init_font_sizes();
    init_font_size_map();

    unsigned char* bitmap = calloc(BITMAP_WIDTH * BITMAP_HEIGHT, sizeof(unsigned char));
    stbtt_PackBegin(&spc, bitmap, BITMAP_WIDTH, BITMAP_HEIGHT, 0, 1, NULL);

    load_font(FONT_DEFAULT, "assets/times.ttf");

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
    stbtt_packedchar b = fonts[id].chars[font_size_map[font_size-MIN_FONT_SIZE]][character-CHAR_OFFSET];
    *bmap_u1 = (f32)b.x0 / BITMAP_WIDTH;
    *bmap_v1 = (f32)b.y0 / BITMAP_HEIGHT;
    *bmap_u2 = (f32)b.x1 / BITMAP_WIDTH;
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

// ---------------------------

static void init_font_sizes(void)
{
    font_sizes[0] = 7;
    font_sizes[1] = 8;
    font_sizes[2] = 9;
    font_sizes[3] = 10;
    font_sizes[4] = 11;
    font_sizes[5] = 12;
    font_sizes[6] = 14;
    font_sizes[7] = 18;
    font_sizes[8] = 24;
    font_sizes[9] = 30;
    font_sizes[10] = 36;
    font_sizes[11] = 48;
    font_sizes[12] = 60;
    font_sizes[13] = 72;
    font_sizes[14] = 96;
}

static int map_font_size(i32 font_size)
{
    if (font_size >= 96) return 14;
    if (font_size >= 72) return 13;
    if (font_size >= 60) return 12;
    if (font_size >= 48) return 11;
    if (font_size >= 36) return 10;
    if (font_size >= 30) return 9;
    if (font_size >= 24) return 8;
    if (font_size >= 18) return 7;
    if (font_size >= 14) return 6;
    if (font_size >= 12) return 5;
    return font_size - MIN_FONT_SIZE;
}

static void init_font_size_map(void)
{
    for (i32 font_size = MIN_FONT_SIZE; font_size <= MAX_FONT_SIZE; font_size++)
        font_size_map[font_size-MIN_FONT_SIZE] = map_font_size(font_size);
}

static void load_font(FontID id, const char* ttf_path)
{
    long size;
    i32 idx;
    FILE* font_file = fopen(ttf_path, "rb");
    fseek(font_file, 0, SEEK_END);
    size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);
    fonts[id].font_buffer = malloc(size);
    fread(fonts[id].font_buffer, size, 1, font_file);
    fclose(font_file);
    
    stbtt_InitFont(&fonts[id].info, fonts[id].font_buffer, 0);

    for (idx = 0; idx <= NUM_FONT_SIZES; idx++) {
        fonts[id].font_range[idx].font_size = (f32)font_sizes[idx];         
        fonts[id].font_range[idx].first_unicode_codepoint_in_range = CHAR_OFFSET; 
        fonts[id].font_range[idx].array_of_unicode_codepoints = NULL;
        fonts[id].font_range[idx].num_chars = NUM_CHARS;       
        fonts[id].font_range[idx].chardata_for_range = fonts[id].chars[idx];
    }

    stbtt_PackFontRanges(&spc, fonts[id].font_buffer, 0, fonts[id].font_range, NUM_FONT_SIZES);
}