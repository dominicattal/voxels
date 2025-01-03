#include "chunk.h"
#include "block.h"
#include "../renderer/renderer.h"
#include <stdlib.h>
#include <stdio.h>

#define NUM_CHUNKS 16 * 1 * 16
#define RENDER_DISTANCE 8
#define INDICES_PER_FACE 6

#define NEGX 0
#define POSX 1
#define NEGY 2
#define POSY 3
#define NEGZ 4
#define POSZ 5

typedef struct {
    Block blocks[32768];
    i32 x, y, z;
    u16 num_blocks;
} Chunk;

typedef struct {
    u32 total_face_count;
    u32 chunk_mesh_length;
    u32 chunk_indirect_length;
    u32 chunk_world_pos_length;
    u32* chunk_mesh_buffer;
    u32* chunk_indirect_buffer;
    i32* chunk_world_pos_buffer;
    Chunk** chunks;
} ChunkState;

static ChunkState state;

static Chunk* chunk_load(i32 x, i32 y, i32 z)
{
    Chunk* chunk = calloc(1, sizeof(Chunk));
    chunk->x = x;
    chunk->y = y;
    chunk->z = z;

    for (i32 i = 0; i < 32 * 32; i++) {
        chunk->blocks[i] = 4;
        chunk->num_blocks++;
    }

    return chunk;
}

static bool opaque_block(Chunk* chunk, i32 idx, i32 dir)
{
    i32 x, y, z, new_idx;
    x = idx & 31;
    y = (idx>>5) & 31;
    z = (idx>>10) & 31;
    switch (dir) {
        case NEGX:
            new_idx = (x-1) + (y<<5) + (z<<10);
            return (x > 0) && (chunk->blocks[new_idx] != 0);
        case POSX:
            new_idx = (x+1) + (y<<5) + (z<<10);
            return (x < 31) && (chunk->blocks[new_idx] != 0);
        case NEGY:
            new_idx = x + ((y-1)<<5) + (z<<10);
            return (y > 0) && (chunk->blocks[new_idx] != 0);
        case POSY:
            new_idx = x + ((y+1)<<5) + (z<<10);
            return (y < 31) && (chunk->blocks[new_idx] != 0);
        case NEGZ:
            new_idx = x + (y<<5) + ((z-1)<<10);
            return (z > 0) && (chunk->blocks[new_idx] != 0);
        case POSZ:
            new_idx = x + (y<<5) + ((z+1)<<10);
            return (z < 31) && (chunk->blocks[new_idx] != 0);
    }
    return FALSE;
}

static void chunk_build_mesh(Chunk* chunk)
{
    f64 start = get_time();
    u32 total_faces = 0;
    u32 face_counts[6];
    for (i32 i = 0; i < 6; i++)
        face_counts[i] = 0;

    // first pass to figure out number of faces for each side
    for (i32 i = 0; i < 32768; i++) {
        if (chunk->blocks[i] != 0) {
            for (i32 dir = 0; dir < 6; dir++) {
                if (!opaque_block(chunk, i, dir)) {
                    face_counts[dir]++;
                    total_faces++;
                }
            }
        }
    }

    i32 idx1, idx2, idx3;
    idx1 = state.chunk_mesh_length;
    idx2 = state.chunk_indirect_length;
    idx3 = state.chunk_world_pos_length;
    state.chunk_mesh_length += total_faces;
    state.chunk_indirect_length += 5 * 6;
    state.chunk_world_pos_length += 4 * 6;
    if (state.chunk_mesh_buffer == NULL) {
        state.chunk_mesh_buffer = malloc(state.chunk_mesh_length * sizeof(u32));
        state.chunk_indirect_buffer = malloc(state.chunk_indirect_length * sizeof(u32));
        state.chunk_world_pos_buffer = malloc(state.chunk_world_pos_length * sizeof(f32));
    } else {
        state.chunk_mesh_buffer = realloc(state.chunk_mesh_buffer, state.chunk_mesh_length * sizeof(u32));
        state.chunk_indirect_buffer = realloc(state.chunk_indirect_buffer, state.chunk_indirect_length * sizeof(u32));
        state.chunk_world_pos_buffer = realloc(state.chunk_world_pos_buffer, state.chunk_world_pos_length * sizeof(f32));
    }

    u32 count;
    u32 instance_count;
    u32 first_index;
    i32 base_vertex;
    u32 base_instance;

    for (i32 i = 0; i < 6; i++) {
        count = INDICES_PER_FACE;
        instance_count = face_counts[i];
        first_index = 0; // CW or CCW
        base_vertex = 0;
        base_instance = state.total_face_count;
        state.chunk_indirect_buffer[idx2++] = count;
        state.chunk_indirect_buffer[idx2++] = instance_count;
        state.chunk_indirect_buffer[idx2++] = first_index;
        state.chunk_indirect_buffer[idx2++] = base_vertex;
        state.chunk_indirect_buffer[idx2++] = base_instance;
        state.chunk_world_pos_buffer[idx3++] = chunk->x;
        state.chunk_world_pos_buffer[idx3++] = chunk->y;
        state.chunk_world_pos_buffer[idx3++] = chunk->z;
        state.chunk_world_pos_buffer[idx3++] = i;
        state.total_face_count += face_counts[i];
    }

    // fill chunk mesh buffer
    u32 prefix[6];
    for (i32 i = 0; i < 6; i++)
        prefix[i] = 0;
    for (i32 i = 1; i < 6; i++)
        prefix[i] = prefix[i-1] + face_counts[i-1];

    u32 idxs[6];
    for (i32 i = 0; i < 6; i++)
        idxs[i] = 0;

    u32 info;
    for (i32 i = 0; i < 32768; i++) {
        if (chunk->blocks[i] != 0) {
            info = 0;
            info |=  i & 31;
            info |= ((i>>5) & 31) << 5;
            info |= ((i>>10) & 31) << 10;
            info |= chunk->blocks[i] << 15;
            for (i32 dir = 0; dir < 6; dir++)
                if (!opaque_block(chunk, i, dir))
                    state.chunk_mesh_buffer[idx1+(idxs[dir]++)+prefix[dir]] = info;
        }
    }
    printf("%f\n", get_time() - start);
}

void chunk_init(void)
{
    state.total_face_count = 0;
    state.chunk_mesh_length = 0;
    state.chunk_indirect_length = 0;
    state.chunk_world_pos_length = 0;
    state.chunk_mesh_buffer = NULL;
    state.chunk_indirect_buffer = NULL;
    state.chunk_world_pos_buffer = NULL;

    state.chunks = malloc(NUM_CHUNKS * sizeof(Chunk));
    for (i32 i = 0; i < NUM_CHUNKS; i++) {
        state.chunks[i] = chunk_load(32 * (i & 15), 0, 32 * ((i>>4)&15));
        chunk_build_mesh(state.chunks[i]);
    }

    if (state.chunk_mesh_buffer == NULL)
        return;
    vbo_bind(VBO_GAME_INSTANCE);
    vbo_malloc(VBO_GAME_INSTANCE, state.chunk_mesh_length * sizeof(u32), GL_STATIC_DRAW);
    vbo_update(VBO_GAME_INSTANCE, 0, state.chunk_mesh_length * sizeof(u32), state.chunk_mesh_buffer);
    dibo_bind(DIBO_GAME);
    dibo_malloc(DIBO_GAME, state.chunk_indirect_length * sizeof(u32), GL_STATIC_DRAW);
    dibo_update(DIBO_GAME, 0, state.chunk_indirect_length * sizeof(u32), state.chunk_indirect_buffer);
    ssbo_bind(SSBO_GAME);
    ssbo_malloc(SSBO_GAME, state.chunk_world_pos_length * sizeof(u32), GL_STATIC_DRAW);
    ssbo_update(SSBO_GAME, 0, state.chunk_world_pos_length * sizeof(u32), state.chunk_world_pos_buffer);
}

void chunk_update(void)
{
    // queue data building
    // queue mesh building
    // queue data unloading
    // queue mesh unloading
}

void chunk_draw(void)
{
    if (state.chunk_mesh_buffer == NULL)
        return;

    shader_use(SHADER_GAME);
    vao_bind(VAO_GAME);
    ebo_bind(EBO_GAME);

    // for some reason cant use this and GL_LINE, so use for loop instead for that
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, 6 * NUM_CHUNKS, 0);
}

void chunk_destroy(void)
{
    
}