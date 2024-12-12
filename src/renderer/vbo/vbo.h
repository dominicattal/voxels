#ifndef VBO_H
#define VBO_H

#include "../../type.h"

typedef struct {
    u32 id, length, max_length;
} VBO;

VBO vbo_create(void);
void vbo_bind(VBO vbo);
void vbo_destroy(VBO vbo);

void vbo_malloc(VBO* vbo, u32 length, GLenum usage);
void vbo_update(VBO* vbo, u32 offset, u32 length, f32* buffer);

#endif