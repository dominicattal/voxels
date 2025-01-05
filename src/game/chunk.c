#include "chunk.h"
#include "block.h"
#include "../camera/camera.h"
#include "../renderer/renderer.h"
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>

#define RENDER_DISTANCE 4

#define NEGX 0
#define POSX 1
#define NEGY 2
#define POSY 3
#define NEGZ 4
#define POSZ 5

typedef struct {
    Block blocks[32768];
    u16 num_blocks;
    i32 x, y, z;
    u32 order_idx;
    u32 mesh_idx;
    u32 num_faces;
    u32 face_counts[6];
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
    Chunk** chunks_swap;
    Chunk** chunk_mesh_order;
    struct {
        i32 x, y, z;
    } center;
    i32 render_distance;
    i32 side;
    i32 num_chunks;
    sem_t mutex;
    pthread_t thread_id;
    bool kill_thread;
} ChunkState;

static ChunkState state;

static i32 axis_dx[6] = {-1, 1, 0, 0, 0, 0};
static i32 axis_dy[6] = {0, 0, -1, 1, 0, 0};
static i32 axis_dz[6] = {0, 0, 0, 0, -1, 1};

#define block_idx(x, y, z) \
    ((x) + ((y) << 5) + ((z) << 10))

#define block_in_bounds(x, y, z) \
    (x >= 0 && x < 32 && y >= 0 && y < 32 && z >= 0 && z < 32)

#define chunk_idx(x, y, z) \
    ((x) + ((cy) * state.side) + ((cz) * state.side * state.side))

#define chunk_in_bounds(x, y, z) \
    (x >= 0 && x < state.side && y >= 0 && y < state.side && z >= 0 && z < state.side)

#define chunk_exists(x, y, z) \
    (state.chunks[chunk_idx(x, y, z)] != NULL)

static Chunk* load_chunk(i32 x, i32 y, i32 z)
{
    i32 cx, cy, cz;
    cx = x - state.center.x + state.render_distance;
    cy = y - state.center.y + state.render_distance;
    cz = z - state.center.z + state.render_distance;
    if (chunk_in_bounds(cx, cy, cz) && chunk_exists(cx, cy, cz))
        return state.chunks[chunk_idx(cx, cy, cz)];

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
    i32 bx, by, bz, cx, cy, cz;
    bx = axis_dx[axis] + (idx & 31);
    by = axis_dy[axis] + ((idx >> 5) & 31);
    bz = axis_dz[axis] + ((idx >> 10) & 31);
    if (block_in_bounds(bx, by, bz))
        return chunk->blocks[block_idx(bx, by, bz)] != 0;

    bx &= 31;
    by &= 31;
    bz &= 31;
    cx = chunk->x - state.center.x + state.render_distance + axis_dx[axis];
    cy = chunk->y - state.center.y + state.render_distance + axis_dy[axis];
    cz = chunk->z - state.center.z + state.render_distance + axis_dz[axis];
    if (chunk_in_bounds(cx, cy, cz))
        return state.chunks[chunk_idx(cx, cy, cz)]->blocks[block_idx(bx, by, bz)] != 0;
    
    return FALSE;
}

static void build_chunk_mesh(Chunk* chunk)
{
    if (chunk->num_blocks == 0)
        return;

    chunk->num_faces = 0;
    for (i32 axis = 0; axis < 6; axis++)
        chunk->face_counts[axis] = 0;

    for (i32 i = 0; i < 32768; i++) {
        if (chunk->blocks[i] != 0) {
            for (i32 axis = 0; axis < 6; axis++) {
                if (!opaque_block(chunk, i, axis)) {
                    chunk->face_counts[axis]++;
                    chunk->num_faces++;
                }
            }
        }
    }

    i32 indirect_idx, world_pos_idx;
    indirect_idx = state.chunk_indirect_length;
    world_pos_idx = state.chunk_world_pos_length;

    chunk->mesh_idx = state.chunk_mesh_length;
    state.chunk_mesh_length += chunk->num_faces;

    if (state.chunk_mesh_buffer == NULL)
        state.chunk_mesh_buffer = malloc(state.chunk_mesh_length * sizeof(u32));
    else
        state.chunk_mesh_buffer = realloc(state.chunk_mesh_buffer, state.chunk_mesh_length * sizeof(u32));

    for (i32 axis = 0; axis < 6; axis++) {

        if (chunk->face_counts[axis] == 0)
            continue;

        state.chunk_indirect_buffer[indirect_idx++] = 4;
        state.chunk_indirect_buffer[indirect_idx++] = chunk->face_counts[axis];
        state.chunk_indirect_buffer[indirect_idx++] = 0;
        state.chunk_indirect_buffer[indirect_idx++] = state.total_face_count;
        state.chunk_world_pos_buffer[world_pos_idx++] = 32 * chunk->x;
        state.chunk_world_pos_buffer[world_pos_idx++] = 32 * chunk->y;
        state.chunk_world_pos_buffer[world_pos_idx++] = 32 * chunk->z;
        state.chunk_world_pos_buffer[world_pos_idx++] = axis;
        state.total_face_count += chunk->face_counts[axis];
        state.chunk_indirect_length += 4;
        state.chunk_world_pos_length += 4;
    }

    u32 prefix[6];
    prefix[0] = chunk->mesh_idx;
    for (i32 i = 1; i < 6; i++)
        prefix[i] = prefix[i-1] + chunk->face_counts[i-1];

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
                    state.chunk_mesh_buffer[(idxs[axis]++)+prefix[axis]] = info;
        }
    }
}

static void* chunk_worker_threads(void* vargp)
{
    #pragma omp parallel
    {
        i32 num_threads = omp_get_num_threads();
        i32 thread_num = omp_get_thread_num();
    }
}

void chunk_init(void)
{
    sem_init(&state.mutex, 0, 1);
    pthread_create(&state.thread_id, NULL, chunk_worker_threads, NULL);   
    state.total_face_count = 0;
    state.chunk_mesh_length = 0;
    state.chunk_indirect_length = 0;
    state.chunk_world_pos_length = 0;
    state.chunk_mesh_buffer = NULL;
    state.render_distance = RENDER_DISTANCE;
    state.center.x = state.center.y = state.center.z = 0;
    state.side = state.render_distance * 2 + 1;
    i32 side = state.side;
    state.num_chunks = side * side * side;

    state.chunk_indirect_buffer = malloc(6 * 4 * side * side * side * sizeof(u32));
    state.chunk_world_pos_buffer = malloc(6 * 4 * side * side * side * sizeof(i32));
    state.chunks = calloc(state.num_chunks, sizeof(Chunk*));
    state.chunks_swap = calloc(state.num_chunks, sizeof(Chunk*));
    state.chunk_mesh_order = calloc(state.num_chunks, sizeof(Chunk*));

    dibo_bind(DIBO_GAME);
    dibo_malloc(DIBO_GAME, 6 * 4 * side * side * side * sizeof(u32), GL_STATIC_DRAW);
    ssbo_bind(SSBO_GAME);
    ssbo_malloc(SSBO_GAME, 6 * 4 * side * side * side * sizeof(i32), GL_STATIC_DRAW);

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
    dibo_bind(DIBO_GAME);
    dibo_update(DIBO_GAME, 0, state.chunk_indirect_length * sizeof(u32), state.chunk_indirect_buffer);
    ssbo_bind(SSBO_GAME);
    ssbo_update(SSBO_GAME, 0, state.chunk_world_pos_length * sizeof(u32), state.chunk_world_pos_buffer);
}

void chunk_update(void)
{
    sem_wait(&state.mutex);
    sem_post(&state.mutex);
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
    free(state.chunks_swap);
    free(state.chunk_mesh_order);
    free(state.chunk_mesh_buffer);
    free(state.chunk_indirect_buffer);
    free(state.chunk_world_pos_buffer);
    sem_destroy(&state.mutex);
}