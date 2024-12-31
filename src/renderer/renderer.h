#ifndef RENDERER_H
#define RENDERER_H

#include "../util/util.h"
#include "vao.h"
#include "shader.h"
#include "texture.h"
#include "vbo.h"
#include "ebo.h"
#include "ssbo.h"
#include "ubo.h"
#include "dibo.h"

void renderer_init(void);
void renderer_render(void);
void renderer_destroy(void);
void renderer_create_font_bitmap(i32 width, i32 height, unsigned char* pixels);
f64  renderer_dt(void);

void renderer_uniform_update_view(f32* mat);
void renderer_uniform_update_proj(f32* mat);

void renderer_toggle_line_mode(void);

#endif