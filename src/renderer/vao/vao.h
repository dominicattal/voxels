#ifndef VAO_H
#define VAO_H

#include "../../util.h"
#include "../vbo/vbo.h"
#include "../ebo/ebo.h"

typedef struct {
    u32 id, length;
    GLenum usage, mode;
    VBO* vbo;
    EBO* ebo;
} VAO;

VAO vao_create(GLenum usage, GLenum mode, u32 length);
void vao_attr(VAO vao, u32 index, u32 length, u32 offset);
void vao_update(VAO vao, u32 vbo_offset, u32 vbo_length, f32* vbo_buffer, u32 ebo_offset, u32 ebo_length, u32* ebo_buffer);
void vao_malloc(VAO vao, u32 vbo_length, u32 ebo_length);
void vao_draw(VAO vao);
void vao_destroy(VAO vao);

#endif