#ifndef RENDERER_H
#define RENDERER_H

#include "../type.h"

#define NUM_TEXTURES 3

typedef enum {
    TEX_NONE = 0,
    TEX_COLOR = 1,
    TEX_BITMAP = 2
} TextureID;

void renderer_init(void);
void renderer_render(void);
void renderer_destroy(void);
void renderer_create_font_bitmap(i32 width, i32 height, unsigned char* pixels);
f64 renderer_dt(void);

#endif