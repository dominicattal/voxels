#ifndef VBO_H
#define VBO_H

#include "../util/util.h"

#define NUM_VBOS 4
typedef enum {
    VBO_GUI = 0,
    VBO_FONT = 1,
    VBO_GAME = 2,
    VBO_GAME_INSTANCE = 3
} VBO;

void vbo_init(void);
void vbo_bind(VBO vbo);
void vbo_malloc(VBO vbo, size_t size, GLenum usage);
void vbo_update(VBO vbo, size_t offset, size_t size, void* buffer);
void vbo_destroy(void);

#endif