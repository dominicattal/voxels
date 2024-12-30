#include "chunk.h"
#include "../renderer/renderer.h"
#include <stdlib.h>

#define NUM_BLOCKS 120

Chunk* chunk_create(f32 x, f32 y, f32 z)
{
    Chunk* chunk = malloc(sizeof(Chunk));
    chunk->position = vec3_create(x, y, z);
    chunk->data.vbo_max_length = NUM_BLOCKS;
    chunk->data.vbo_buffer = malloc(NUM_BLOCKS * sizeof(u32));
    chunk->data.vbo_length = 0;

    for (i32 i = 0; i < NUM_BLOCKS; i++) {
        i32 x = rand() % 32;
        i32 y = rand() % 32;
        i32 z = rand() % 32;
        i32 id = 3 + rand() % 4;
        chunk->blocks[x][y][z] = id;
        u32 info = 0;
        info |= x & 31;
        info |= (y & 31) << 5;
        info |= (z & 31) << 10;
        info |= (id & 31) << 15;
        chunk->data.vbo_buffer[chunk->data.vbo_length++] = info;
    }

    return chunk;
}

void chunk_draw(Chunk* chunk)
{
    glUniform3f(shader_get_uniform_location(SHADER_GAME, "ChunkPos"), chunk->position.x, chunk->position.y, chunk->position.z);
    vbo_bind(VBO_GAME_INSTANCE);
    vbo_malloc(VBO_GAME_INSTANCE, chunk->data.vbo_max_length * sizeof(u32), GL_STATIC_DRAW);
    vbo_update(VBO_GAME_INSTANCE, 0, chunk->data.vbo_length * sizeof(u32), chunk->data.vbo_buffer);
    glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, NUM_BLOCKS);
}

void chunk_destroy(Chunk* chunk)
{
    free(chunk->data.vbo_buffer);
    free(chunk);
}