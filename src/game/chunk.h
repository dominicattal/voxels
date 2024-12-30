#ifndef CHUCK_H
#define CHUCK_H
#include "block.h"

typedef struct {
    Block blocks[32][32][32];
    struct {
        u32 vbo_length, vbo_max_length;
        u32* vbo_buffer;
    } data;
} Chunk;

Chunk* chunk_create(void);
void chunk_draw(Chunk* chunk);
void chunk_destroy(Chunk* chunk);

#endif