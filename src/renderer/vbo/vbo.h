#ifndef VBO_H
#define VBO_H

#include "../../type.h"

#define NUM_VBOS 2

typedef enum {
    VBO_GUI = 0,
    VBO_FONT = 1
} VBO;

void vbo_init(void);
void vbo_bind(VBO vbo);
void vbo_destroy(void);

void vbo_malloc(VBO vbo, u32 length, GLenum usage);
void vbo_update(VBO vbo, u32 offset, u32 length, f32* buffer);

#endif