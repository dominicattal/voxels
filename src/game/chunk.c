#include "chunk.h"
#include "block.h"
#include "../renderer/renderer.h"
#include <stdlib.h>
#include <stdio.h>

#define NUM_BLOCKS 1
#define NUM_CHUNKS 16 * 16 * 16

typedef struct {
    Block blocks[32][32][32];
    vec3 position;
} Chunk;

static u32 total_block_count;
static u32 chunk_mesh_length;
static u32 chunk_indirect_length;
static u32 chunk_world_pos_length;
static u32* chunk_mesh_buffer;
static u32* chunk_indirect_buffer;
static f32* chunk_world_pos_buffer;
static Chunk** chunks;

static Chunk* chunk_create(f32 x, f32 y, f32 z)
{
    Chunk* chunk = malloc(sizeof(Chunk));
    chunk->position = vec3_create(x, y, z);

    i32 idx1, idx2;
    idx1 = chunk_mesh_length;
    idx2 = chunk_indirect_length;
    chunk_mesh_length += NUM_BLOCKS;
    chunk_indirect_length += 5;
    if (chunk_mesh_buffer == NULL) {
        chunk_mesh_buffer = malloc(chunk_mesh_length * sizeof(u32));
        chunk_indirect_buffer = malloc(2 * sizeof(u32));
    } else {
        chunk_mesh_buffer = realloc(chunk_mesh_buffer, chunk_mesh_length * sizeof(u32));
        chunk_indirect_buffer = realloc(chunk_indirect_buffer, chunk_indirect_length * sizeof(u32));
    }

    u32 count = 36;
    u32 instance_count = NUM_BLOCKS;
    u32 first_idx = 0;
    u32 base_vertex = 0;
    u32 base_instance = NUM_BLOCKS * (total_block_count++);
    chunk_indirect_buffer[idx2++] = count;
    chunk_indirect_buffer[idx2++] = instance_count;
    chunk_indirect_buffer[idx2++] = first_idx;
    chunk_indirect_buffer[idx2++] = base_vertex;
    chunk_indirect_buffer[idx2++] = base_instance;

    for (i32 i = 0; i < NUM_BLOCKS; i++) {
        i32 x = rand() % 32;
        i32 y = rand() % 32;
        i32 z = rand() % 32;
        i32 id = 3 + rand() % 3;
        chunk->blocks[x][y][z] = id;
        u32 info = 0;
        info |= x & 31;
        info |= (y & 31) << 5;
        info |= (z & 31) << 10;
        info |= (id & 31) << 15;
        chunk_mesh_buffer[idx1++] = info;
    }

    return chunk;
}


void chunk_init(void)
{
    total_block_count = 0;
    chunk_mesh_length = 0;
    chunk_indirect_length = 0;
    chunk_world_pos_length = 0;
    chunk_mesh_buffer = NULL;
    chunk_indirect_buffer = NULL;
    chunk_world_pos_buffer = NULL;
    chunks = malloc(NUM_CHUNKS * sizeof(Chunk));
    for (i32 i = 0; i < NUM_CHUNKS; i++)
        chunks[i] = chunk_create(32 * (i & 15), 32 * ((i>>4)&15), 32 * ((i>>8)&15));

    vbo_bind(VBO_GAME_INSTANCE);
    vbo_malloc(VBO_GAME_INSTANCE, chunk_mesh_length * sizeof(u32), GL_STATIC_DRAW);
    vbo_update(VBO_GAME_INSTANCE, 0, chunk_mesh_length * sizeof(u32), chunk_mesh_buffer);
    dibo_bind(DIBO_GAME);
    dibo_malloc(DIBO_GAME, chunk_indirect_length * sizeof(u32), GL_STATIC_DRAW);
    dibo_update(DIBO_GAME, 0, chunk_indirect_length * sizeof(u32), chunk_indirect_buffer);
}

void chunk_draw(void)
{
    shader_use(SHADER_GAME);
    vao_bind(VAO_GAME);
    ebo_bind(EBO_GAME);
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, NUM_BLOCKS * NUM_CHUNKS, 0);
    //for (i32 i = 0; i < NUM_CHUNKS; i++)
    //    glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, NUM_BLOCKS, 0, i * NUM_BLOCKS);
}

void chunk_destroy(void)
{
    for (i32 i = 0; i < NUM_CHUNKS; i++)
        free(chunks[i]);
    free(chunks);
}