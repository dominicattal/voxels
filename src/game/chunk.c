#include "chunk.h"
#include "block.h"
#include "../camera/camera.h"
#include "../renderer/renderer.h"
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>

#define RENDER_DISTANCE 15
#define CHUNKS_PER_UPDATE 50

typedef struct {
    i32 x, y, z;
    Block blocks[32768];
    u16 num_blocks;
    i32 order_idx;
    i32 mesh_idx;
    i32 num_faces;
    i32 face_counts[6];
} Chunk;

typedef struct {
    i32 mesh_length;
    i32 indirect_length;
    i32 world_pos_length;
    i32 chunk_order_length;
    u32* mesh_buffer;
    u32* indirect_buffer;
    i32* world_pos_buffer;
    Chunk** chunks;
    Chunk** chunk_order;
    i32 render_distance;
    i32 side;
    i32 num_chunks;
    i32* sorted_chunk_idx;
    struct {
        i32 x, y, z;
    } center;
    sem_t mutex;
    pthread_t thread_id;
    bool kill_thread;
} ChunkState;

static ChunkState state;

static i32 axis_dx[6] = {-1, 1, 0, 0, 0, 0};
static i32 axis_dy[6] = {0, 0, -1, 1, 0, 0};
static i32 axis_dz[6] = {0, 0, 0, 0, -1, 1};

#define block_idx(_x, _y, _z) \
    ((_x) + ((_y) << 5) + ((_z) << 10))

#define block_in_bounds(_x, _y, _z) \
    (_x >= 0 && _x < 32 && _y >= 0 && _y < 32 && _z >= 0 && _z < 32)

#define chunk_idx(_x, _y, _z) \
      ((_x - state.center.x + state.render_distance) \
    + ((_y - state.center.y + state.render_distance) * state.side) \
    + ((_z - state.center.z + state.render_distance) * state.side * state.side))

#define chunk_in_bounds(_x, _y, _z, _center_x, _center_y, _center_z) \
      ((_x - _center_x + state.render_distance) >= 0 \
    && (_x - _center_x + state.render_distance) < state.side \
    && (_y - _center_y + state.render_distance) >= 0 \
    && (_y - _center_y + state.render_distance) < state.side \
    && (_z - _center_z + state.render_distance) >= 0 \
    && (_z - _center_z + state.render_distance) < state.side)

#define chunk_exists(_x, _y, _z) \
      (chunk_in_bounds(_x, _y, _z, state.center.x, state.center.y, state.center.z) \
    && state.chunks[chunk_idx(_x, _y, _z)] != NULL)

#define chunk_axis_faces_center(_chunk, _axis) \
       ((axis == NEGX && chunk->x >= state.center.x)  \
    ||  (axis == POSX && chunk->x <= state.center.x)  \
    ||  (axis == NEGY && chunk->y >= state.center.y)  \
    ||  (axis == POSY && chunk->y <= state.center.y)  \
    ||  (axis == NEGZ && chunk->z >= state.center.z)  \
    ||  (axis == POSZ && chunk->z <= state.center.z))

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
    cx = axis_dx[axis] + chunk->x;
    cy = axis_dy[axis] + chunk->y;
    cz = axis_dz[axis] + chunk->z;
    
    if (chunk_exists(cx, cy, cz))
        return state.chunks[chunk_idx(cx, cy, cz)]->blocks[block_idx(bx, by, bz)] != 0;
    
    return FALSE;
}

static void generate_faces(Chunk* chunk)
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
}

static Chunk* load_chunk(i32 cx, i32 cy, i32 cz)
{
    Chunk* chunk = calloc(1, sizeof(Chunk));
    chunk->x = cx;
    chunk->y = cy;
    chunk->z = cz;
    chunk->mesh_idx = -1;
    chunk->order_idx = -1;

    if (cy != 0)
        return chunk;

    for (i32 bz = 0; bz < 32; bz++) {
        for (i32 bx = 0; bx < 32; bx++) {
            chunk->blocks[block_idx(bx, 0, bz)] = 3;
            chunk->num_blocks++;
        }
    }

    return chunk;
}

static void unload_chunk(Chunk* chunk)
{
    free(chunk);
}

static void load_chunk_mesh(Chunk* chunk, i32 new_mesh_idx, u32* mesh_buffer)
{
    // if mesh exists, then move mesh buffer data to swap buffer
    // if it doesnt, then generate it and put it in swap buffer

    if (chunk->mesh_idx != -1) {
        for (i32 i = 0; i < chunk->num_faces; i++)
            mesh_buffer[new_mesh_idx++] = state.mesh_buffer[chunk->mesh_idx + i];
        return;
    }

    u32 prefix[6];
    prefix[0] = new_mesh_idx;
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
                    mesh_buffer[(idxs[axis]++)+prefix[axis]] = info;
        }
    }
}

static void* chunk_worker_threads(void* vargp)
{
    vec3 position;
    i32 new_center_x, new_center_y, new_center_z;
    i32 side;
    i32 num_chunks_loaded;
    i32 chunk_order_length;
    i32 group_size;
    i32* mesh_lengths;
    i32* mesh_prefix;
    u32* mesh_buffer_swap;
    Chunk** chunks_swap;
    void* tmp;
    #pragma omp parallel
    {
        i32 num_threads, thread_num;
        i32 i, j, axis, cx, cy, cz;
        Chunk* chunk;
        
        num_threads = omp_get_num_threads();
        thread_num = omp_get_thread_num();

        #pragma omp master
        {
            mesh_lengths = malloc(num_threads * sizeof(i32));
            mesh_prefix = malloc((num_threads + 1) * sizeof(i32));
            chunks_swap = calloc(state.num_chunks, sizeof(Chunk*));
            mesh_buffer_swap = NULL;
        }

        while (!state.kill_thread)
        {
            #pragma omp master
            {
                position = camera_position();
                new_center_x = position.x / 32 - (position.x < 0);
                new_center_y = position.y / 32 - (position.y < 0);
                new_center_z = position.z / 32 - (position.z < 0);
                side = state.side;
                num_chunks_loaded = 0;
            }
            #pragma omp barrier

            for (i = thread_num; i < state.num_chunks; i += num_threads) {
                cx = state.center.x - state.render_distance + (i % side);
                cy = state.center.y - state.render_distance + ((i / side) % side);
                cz = state.center.z - state.render_distance + (i / side / side);
                chunk = state.chunks[i];
                if (!chunk_in_bounds(cx, cy, cz, new_center_x, new_center_y, new_center_z) && chunk != NULL) {
                    if (chunk->order_idx != -1)
                        state.chunk_order[chunk->order_idx] = NULL;
                    unload_chunk(chunk);
                    state.chunks[i] = NULL;
                }
            }
            #pragma omp barrier

            for (j = thread_num; j < state.num_chunks; j += num_threads) {
                i = state.sorted_chunk_idx[j];
                cx = new_center_x - state.render_distance + (i % side);
                cy = new_center_y - state.render_distance + ((i / side) % side);
                cz = new_center_z - state.render_distance + (i / side / side);
                if (chunk_exists(cx, cy, cz)) {
                    chunks_swap[i] = state.chunks[chunk_idx(cx, cy, cz)];
                } else {
                    if (num_chunks_loaded > CHUNKS_PER_UPDATE) {
                        chunks_swap[i] = NULL;
                    } else {
                        for (axis = 0; axis < 6; axis++) {
                            if (chunk_exists(cx + axis_dx[axis], cy + axis_dy[axis], cz + axis_dz[axis])) {
                                chunk = state.chunks[chunk_idx(cx + axis_dx[axis], cy + axis_dy[axis], cz + axis_dz[axis])];
                                if (chunk->order_idx != -1) {
                                    state.chunk_order[chunk->order_idx] = NULL;
                                    chunk->mesh_idx = -1;
                                }
                            }
                        }
                        chunks_swap[i] = load_chunk(cx, cy, cz);
                        #pragma omp atomic update
                        num_chunks_loaded++;
                    }
                }
            }
            #pragma omp barrier

            #pragma omp master
            {
                tmp = state.chunks;
                state.chunks = chunks_swap;
                chunks_swap = tmp;

                state.center.x = new_center_x;
                state.center.y = new_center_y;
                state.center.z = new_center_z;
            }
            #pragma omp barrier

            #pragma omp master
            {
                chunk_order_length = 0;
                for (i = 0; i < state.chunk_order_length; i++) {
                    if (state.chunk_order[i] != NULL) {
                        state.chunk_order[i]->order_idx = chunk_order_length;
                        state.chunk_order[chunk_order_length++] = state.chunk_order[i];
                    }
                }
            }
            #pragma omp barrier
            
            for (i = thread_num; i < state.num_chunks; i += num_threads) {
                chunk = state.chunks[i];
                if (chunk != NULL && chunk->mesh_idx == -1) {
                    generate_faces(chunk);
                    #pragma omp critical
                    {
                        chunk->order_idx = chunk_order_length;
                        state.chunk_order[chunk_order_length++] = chunk;
                    }
                }
            }
            #pragma omp barrier

            mesh_lengths[thread_num] = 0;
            #pragma omp barrier

            for (i = thread_num; i < chunk_order_length; i += num_threads)
                mesh_lengths[thread_num] += state.chunk_order[i]->num_faces;
            #pragma omp barrier

            #pragma omp master
            {
                mesh_prefix[0] = 0;
                for (i = 0; i < num_threads; i++)
                    mesh_prefix[i+1] = mesh_lengths[i] + mesh_prefix[i];
                free(mesh_buffer_swap);
                mesh_buffer_swap = malloc(mesh_prefix[num_threads] * sizeof(u32));
            }
            #pragma omp barrier

            for (i = thread_num; i < chunk_order_length; i += num_threads) {
                chunk = state.chunk_order[i];
                load_chunk_mesh(chunk, mesh_prefix[thread_num], mesh_buffer_swap);
                chunk->mesh_idx = mesh_prefix[thread_num];
                mesh_prefix[thread_num] += chunk->num_faces;
            }
            #pragma omp barrier

            #pragma omp master
            {
                sem_wait(&state.mutex);
                tmp = state.mesh_buffer;
                state.mesh_buffer = mesh_buffer_swap;
                mesh_buffer_swap = tmp;
                
                state.chunk_order_length = chunk_order_length;
                state.mesh_length = mesh_prefix[num_threads];

                i32 indirect_idx, world_pos_idx, prefix;
                indirect_idx = world_pos_idx = 0;
                for (i = 0; i < state.num_chunks; i++) {
                    Chunk* chunk = state.chunks[state.sorted_chunk_idx[i]];
                    if (chunk == NULL)
                        continue;
                    prefix = chunk->mesh_idx;
                    if (prefix == -1)
                        continue;
                    for (axis = 0; axis < 6; axis++) {
                        if (chunk->face_counts[axis] != 0 && chunk_axis_faces_center(chunk, axis)) 
                        {
                            state.indirect_buffer[indirect_idx++] = 4;
                            state.indirect_buffer[indirect_idx++] = chunk->face_counts[axis];
                            state.indirect_buffer[indirect_idx++] = 0;
                            state.indirect_buffer[indirect_idx++] = prefix;
                            state.world_pos_buffer[world_pos_idx++] = 32 * chunk->x;
                            state.world_pos_buffer[world_pos_idx++] = 32 * chunk->y;
                            state.world_pos_buffer[world_pos_idx++] = 32 * chunk->z;
                            state.world_pos_buffer[world_pos_idx++] = axis;
                        }
                        prefix += chunk->face_counts[axis];
                    }
                }
                state.indirect_length = indirect_idx;
                state.world_pos_length = world_pos_idx;
                sem_post(&state.mutex);
            }
            #pragma omp barrier
        }

        #pragma omp master
        {
            free(mesh_lengths);
            free(mesh_prefix);
            free(chunks_swap);
            free(mesh_buffer_swap);
        }
    }
}

static i32 compare_chunk_idx(const void* ptr1, const void* ptr2)
{
    i32 idx1, idx2;
    i32 dx1, dy1, dz1, dx2, dy2, dz2;
    idx1 = *(int*)ptr1;
    idx2 = *(int*)ptr2;
    dx1 = abs(state.render_distance - idx1 % state.side);
    dy1 = abs(state.render_distance - (idx1 / state.side) % state.side);
    dz1 = abs(state.render_distance - idx1 / state.side / state.side);
    dx2 = abs(state.render_distance - idx2 % state.side);
    dy2 = abs(state.render_distance - (idx2 / state.side) % state.side);
    dz2 = abs(state.render_distance - idx2 / state.side / state.side);
    return dx1 - dx2 + dy1 - dy2 + dz1 - dz2;
}

void chunk_init(void)
{
    sem_init(&state.mutex, 0, 1);
    pthread_create(&state.thread_id, NULL, chunk_worker_threads, NULL);   
    state.mesh_length = 0;
    state.indirect_length = 0;
    state.world_pos_length = 0;
    state.mesh_buffer = NULL;
    state.render_distance = RENDER_DISTANCE;
    state.kill_thread = FALSE;
    state.center.x = state.center.y = state.center.z = 0;
    state.side = state.render_distance * 2 + 1;
    state.num_chunks = state.side * state.side * state.side;

    state.indirect_buffer = malloc(6 * 4 * state.num_chunks * sizeof(u32));
    state.world_pos_buffer = malloc(6 * 4 * state.num_chunks * sizeof(i32));
    state.chunks = calloc(state.num_chunks, sizeof(Chunk*));
    state.chunk_order = calloc(state.num_chunks, sizeof(Chunk*));

    state.sorted_chunk_idx = malloc(state.num_chunks * sizeof(i32));
    for (i32 i = 0; i < state.num_chunks; i++)
        state.sorted_chunk_idx[i] = i;
    qsort(state.sorted_chunk_idx, state.num_chunks, sizeof(i32), compare_chunk_idx);

    dibo_bind(DIBO_GAME);
    dibo_malloc(DIBO_GAME, 6 * 4 * state.num_chunks * sizeof(u32), GL_STATIC_DRAW);
    ssbo_bind(SSBO_GAME);
    ssbo_malloc(SSBO_GAME, 6 * 4 * state.num_chunks * sizeof(i32), GL_STATIC_DRAW);
}

void chunk_prepare_render(void)
{
    
}

void chunk_render(void)
{
    if (state.mesh_buffer == NULL)
        return;

    sem_wait(&state.mutex);
    vbo_bind(VBO_GAME_INSTANCE);
    vbo_malloc(VBO_GAME_INSTANCE, state.mesh_length * sizeof(u32), GL_STATIC_DRAW);
    vbo_update(VBO_GAME_INSTANCE, 0, state.mesh_length * sizeof(u32), state.mesh_buffer);
    dibo_bind(DIBO_GAME);
    dibo_update(DIBO_GAME, 0, state.indirect_length * sizeof(u32), state.indirect_buffer);
    ssbo_bind(SSBO_GAME);
    ssbo_update(SSBO_GAME, 0, state.world_pos_length * sizeof(u32), state.world_pos_buffer);
    sem_post(&state.mutex);

    shader_use(SHADER_GAME);
    vao_bind(VAO_GAME);
    ebo_bind(EBO_GAME);
    glMultiDrawArraysIndirect(GL_TRIANGLE_STRIP, 0, state.indirect_length / 4, 0);
}

void chunk_destroy(void)
{
    state.kill_thread = TRUE;
    pthread_join(state.thread_id, NULL);
    for (i32 i = 0; i < state.num_chunks; i++)
        free(state.chunks[i]);

    free(state.chunks);
    free(state.chunk_order);
    free(state.mesh_buffer);
    free(state.indirect_buffer);
    free(state.world_pos_buffer);
    free(state.sorted_chunk_idx);
    sem_destroy(&state.mutex);
}