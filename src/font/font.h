#ifndef FONT_H
#define FONT_H

#include "../util.h"
#include <stb_rect_pack.h>
#include <stb_truetype.h>

#define BITMAP_WIDTH    512
#define BITMAP_HEIGHT   512
#define CHAR_OFFSET     32
#define NUM_CHARS       96

#define MIN_FONT_SIZE 7
#define MAX_FONT_SIZE (MIN_FONT_SIZE + (1 << 6))

#define NUM_FONTS 1

typedef struct {
    stbtt_fontinfo info;
    stbtt_pack_context spc;
    stbtt_pack_range fontRange;
    stbtt_packedchar packedChars[NUM_CHARS];
    unsigned char* bitmap;
    unsigned char* font_buffer;
} Font;

typedef enum {
    FONT_DEFAULT = 0,
} FontID;

extern Font font;

void font_init(void);
void font_info(FontID id, i32 font_size, i32* ascent, i32* descent, i32* line_gap);
void font_char_hmetrics(FontID id, i32 font_size, char character, i32* advance, i32* left_side_bearing);
void font_char_bbox(FontID id, i32 font_size, char character, i32* bbox_x1, i32* bbox_y1, i32* bbox_x2, i32* bbox_y2);
void font_char_bmap(FontID id, i32 font_size, char character, f32* bmap_u1, f32* bmap_v1, f32* bmap_u2, f32* bmap_v2);
void font_char_kern(FontID id, i32 font_size, char character, char next_character, i32* kern);
void font_destroy(void);

#endif