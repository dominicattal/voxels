#ifndef TEXTURE_H
#define TEXTURE_H

#include "../util/util.h"

#define NUM_TEXTURES 7

typedef enum {
    TEX_NONE = 0,
    TEX_COLOR = 1,
    TEX_BITMAP,
    TEX_GRASS_TOP,
    TEX_GRASS_SIDE,
    TEX_GRASS_BOTTOM,
    TEX_STONE
} Texture;

#define NUM_FONTS   3

typedef enum {
    FONT_DEFAULT = 0,
    FONT_TWO = 1,
    FONT_MONOSPACE = 2
} Font;

void texture_init(void);
void texture_destroy(void);
u32  texture_font_id(Font font, char character);

void font_info(Font id, i32 font_size, i32* ascent, i32* descent, i32* line_gap);
void font_char_hmetrics(Font id, i32 font_size, char character, i32* advance, i32* left_side_bearing);
void font_char_bbox(Font id, i32 font_size, char character, i32* bbox_x1, i32* bbox_y1, i32* bbox_x2, i32* bbox_y2);
void font_char_bmap(Font id, i32 font_size, char character, f32* bmap_u1, f32* bmap_v1, f32* bmap_u2, f32* bmap_v2);
void font_char_kern(Font id, i32 font_size, char character, char next_character, i32* kern);

#endif