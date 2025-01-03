#include "chunk.h"
#include "block.h"
#include "../camera/camera.h"
#include "../renderer/renderer.h"
#include <stdlib.h>
#include <stdio.h>

#define RENDER_DISTANCE 4

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
    struct {
        i32 x, y, z;
    } center;
    i32 render_distance;
    i32 num_chunks;
} ChunkState;

static ChunkState state;

#define block_idx(x, y, z) ((x) + ((y)<<5) + ((z)<<10))

static Chunk* load_chunk(i32 x, i32 y, i32 z)
{
    Chunk* chunk = calloc(1, sizeof(Chunk));
    chunk->x = x;
    chunk->y = y;
    chunk->z = z;

    if (y != 0)
        return chunk;

    for (i32 z = 0; z < 32; z++) {
        for (i32 x = 0; x < 32; x++) {
            chunk->blocks[block_idx(x, 0, z)] = 3;
            chunk->num_blocks++;
        }
    }

    return chunk;
}

static void unload_chunk(Chunk* chunk)
{
    free(chunk);
}

static bool opaque_block(Chunk* chunk, i32 idx, i32 axis)
{
    static i32 dx[6] = {-1, 1, 0, 0, 0, 0};
    static i32 dy[6] = {0, 0, -1, 1, 0, 0};
    static i32 dz[6] = {0, 0, 0, 0, -1, 1};
    i32 bx, by, bz, cx, cy, cz, side, new_chunk_idx;
    bx = dx[axis] + (idx & 31);
    by = dy[axis] + ((idx >> 5) & 31);
    bz = dz[axis] + ((idx >> 10) & 31);
    if (bx >= 0 && bx < 32 && by >= 0 && by < 32 && bz >= 0 && bz < 32)
        return chunk->blocks[block_idx(bx, by, bz)] != 0;

    bx &= 31;
    by &= 31;
    bz &= 31;
    cx = chunk->x - state.center.x + state.render_distance + dx[axis];
    cy = chunk->y - state.center.y + state.render_distance + dy[axis];
    cz = chunk->z - state.center.z + state.render_distance + dz[axis];
    side = state.render_distance * 2 + 1;
    if (cx >= 0 && cx < side && cy >= 0 && cy < side && cz >= 0 && cz < side) {
        new_chunk_idx = cx + cy * side + cz * side * side;
        return state.chunks[new_chunk_idx]->blocks[block_idx(bx, by, bz)] != 0;
    }
    return FALSE;
}

static void build_chunk_mesh(Chunk* chunk)
{
    if (chunk->num_blocks == 0)
        return;

    u32 total_faces = 0;
    u32 face_counts[6];
    for (i32 axis = 0; axis < 6; axis++)
        face_counts[axis] = 0;

    // first pass to figure out number of faces for each side
    for (i32 i = 0; i < 32768; i++) {
        if (chunk->blocks[i] != 0) {
            for (i32 axis = 0; axis < 6; axis++) {
                if (!opaque_block(chunk, i, axis)) {
                    face_counts[axis]++;
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
    if (state.chunk_mesh_buffer == NULL)
        state.chunk_mesh_buffer = malloc(state.chunk_mesh_length * sizeof(u32));
    else
        state.chunk_mesh_buffer = realloc(state.chunk_mesh_buffer, state.chunk_mesh_length * sizeof(u32));

    u32 count;
    u32 instance_count;
    u32 first_index;
    u32 base_instance;

    for (i32 axis = 0; axis < 6; axis++) {

        if (face_counts[axis] == 0)
            continue;

        count = 4;
        instance_count = face_counts[axis];
        first_index = 0;
        base_instance = state.total_face_count;
        state.chunk_indirect_buffer[idx2++] = count;
        state.chunk_indirect_buffer[idx2++] = instance_count;
        state.chunk_indirect_buffer[idx2++] = first_index;
        state.chunk_indirect_buffer[idx2++] = base_instance;
        state.chunk_world_pos_buffer[idx3++] = 32 * chunk->x;
        state.chunk_world_pos_buffer[idx3++] = 32 * chunk->y;
        state.chunk_world_pos_buffer[idx3++] = 32 * chunk->z;
        state.chunk_world_pos_buffer[idx3++] = axis;
        state.total_face_count += face_counts[axis];
        state.chunk_indirect_length += 4;
        state.chunk_world_pos_length += 4;
    }

    // fill chunk mesh buffer
    u32 prefix[6];
    prefix[0] = 0;
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
            for (i32 axis = 0; axis < 6; axis++)
                if (!opaque_block(chunk, i, axis))
                    state.chunk_mesh_buffer[idx1+(idxs[axis]++)+prefix[axis]] = info;
        }
    }
}

void chunk_init(void)
{
    state.total_face_count = 0;
    state.chunk_mesh_length = 0;
    state.chunk_indirect_length = 0;
    state.chunk_world_pos_length = 0;
    state.chunk_mesh_buffer = NULL;

    state.render_distance = RENDER_DISTANCE;
    state.center.x = state.center.y = state.center.z = 0;
    i32 side = state.render_distance * 2 + 1;

    state.chunk_indirect_buffer = malloc(4 * side * side * side * sizeof(u32));
    state.chunk_world_pos_buffer = malloc(4 * side * side * side * sizeof(i32));

    dibo_bind(DIBO_GAME);
    dibo_malloc(DIBO_GAME, 4 * side * side * side * sizeof(u32), GL_STATIC_DRAW);
    ssbo_bind(SSBO_GAME);
    ssbo_malloc(SSBO_GAME, 4 * side * side * side * sizeof(i32), GL_STATIC_DRAW);

    state.num_chunks = side * side * side;
    state.chunks = malloc(state.num_chunks * sizeof(Chunk*));
    for (i32 i = 0; i < state.num_chunks; i++) {
        i32 x, y, z;
        x = state.center.x - state.render_distance + (i % side);
        y = state.center.y - state.render_distance + ((i / side) % side);
        z = state.center.z - state.render_distance + (i / side / side);
        state.chunks[i] = load_chunk(x, y, z);
    }

    for (i32 i = 0; i < state.num_chunks; i++)
        build_chunk_mesh(state.chunks[i]);

    if (state.chunk_mesh_buffer == NULL)
        return;

    vbo_bind(VBO_GAME_INSTANCE);
    vbo_malloc(VBO_GAME_INSTANCE, state.chunk_mesh_length * sizeof(u32), GL_STATIC_DRAW);
    vbo_update(VBO_GAME_INSTANCE, 0, state.chunk_mesh_length * sizeof(u32), state.chunk_mesh_buffer);
    dibo_update(DIBO_GAME, 0, state.chunk_indirect_length * sizeof(u32), state.chunk_indirect_buffer);
    ssbo_update(SSBO_GAME, 0, state.chunk_world_pos_length * sizeof(u32), state.chunk_world_pos_buffer);
}

void chunk_update(void)
{
    // queue data building
    vec3 position = camera_position();
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

    glMultiDrawArraysIndirect(GL_TRIANGLE_STRIP, 0, state.chunk_indirect_length / 4, 0);
}

void chunk_destroy(void)
{
    free(state.chunks);
    free(state.chunk_mesh_buffer);
    free(state.chunk_indirect_buffer);
    free(state.chunk_world_pos_buffer);
}