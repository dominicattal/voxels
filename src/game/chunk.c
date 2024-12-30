#include "chunk.h"
#include "block.h"
#include "../renderer/renderer.h"
#include <stdlib.h>

#define NUM_BLOCKS 120
#define NUM_CHUNKS 16 * 16 * 16

typedef struct {
    Block blocks[32][32][32];
    struct {
        u32 vbo_length, vbo_max_length;
        u32* vbo_buffer;
    } data;
    vec3 position;
} Chunk;

static u32* chunk_buffer;
static Chunk** chunks;

static Chunk* chunk_create(f32 x, f32 y, f32 z)
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


void chunk_init(void)
{
    chunks = malloc(NUM_CHUNKS * sizeof(Chunk));
    for (i32 i = 0; i < NUM_CHUNKS; i++)
        chunks[i] = chunk_create(32 * (i & 15), 32 * ((i>>4)&15), 32 * ((i>>8)&15));

}

void chunk_draw(void)
{
    for (i32 i = 0; i < NUM_CHUNKS; i++) {
        Chunk* chunk = chunks[i];
        glUniform3f(shader_get_uniform_location(SHADER_GAME, "ChunkPos"), chunk->position.x, chunk->position.y, chunk->position.z);
        vbo_bind(VBO_GAME_INSTANCE);
        vbo_malloc(VBO_GAME_INSTANCE, chunk->data.vbo_max_length * sizeof(u32), GL_STATIC_DRAW);
        vbo_update(VBO_GAME_INSTANCE, 0, chunk->data.vbo_length * sizeof(u32), chunk->data.vbo_buffer);
        glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, NUM_BLOCKS);
    }
}

void chunk_destroy(void)
{
    for (i32 i = 0; i < NUM_CHUNKS; i++) {
        free(chunks[i]->data.vbo_buffer);
        free(chunks[i]);
    }
    free(chunks);
}