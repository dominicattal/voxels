#ifndef RENDERER_H
#define RENDERER_H

#include "../util/util.h"

void renderer_init(void);
void renderer_render(void);
void renderer_destroy(void);
void renderer_create_font_bitmap(i32 width, i32 height, unsigned char* pixels);
f64  renderer_dt(void);

void renderer_uniform_update_view(f32* mat);
void renderer_uniform_update_proj(f32* mat);

#endif