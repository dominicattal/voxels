#ifndef RENDERER_H
#define RENDERER_H

#include "../type.h"


void renderer_init(void);
void renderer_render(void);
void renderer_destroy(void);
void renderer_create_font_bitmap(i32 width, i32 height, unsigned char* pixels);
f64 renderer_dt(void);

#endif