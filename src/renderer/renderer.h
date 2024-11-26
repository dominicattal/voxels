#ifndef RENDERER_H
#define RENDERER_H

#include "../util.h"

#define NUM_VAOS 1

typedef enum {
    VAO_GUI = 0
} VAOID;

void renderer_init(void);
void renderer_malloc(VAOID vao_index, u32 vbo_length, u32 ebo_length);
void renderer_update(VAOID vao_index, u32 vbo_offset, u32 vbo_length, f32* vbo_buffer, u32 ebo_offset, u32 ebo_length, u32* ebo_buffer);
void renderer_render(void);
void renderer_destroy(void);

void renderer_create_font_bitmap(i32 width, i32 height, unsigned char* pixels);

#endif