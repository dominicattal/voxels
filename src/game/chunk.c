#include "chunk.h"
#include "../camera/camera.h"
#include "../renderer/renderer.h"
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <math.h>
#include <assert.h>
#include <omp.h>

#define RENDER_DISTANCE    30

#define BITS_PER_AXIS      5
#define BITS_PER_AXIS_2    (2 * BITS_PER_AXIS)
#define BITS_PER_AXIS_3    (3 * BITS_PER_AXIS)
#define BITS_PER_AXIS_4    (4 * BITS_PER_AXIS)
#define BITS_PER_AXIS_5    (5 * BITS_PER_AXIS)
#define CHUNK_SIZE         (1 << BITS_PER_AXIS)
#define AXIS_MASK          (CHUNK_SIZE - 1)
#define CHUNK_SIZE_2       (CHUNK_SIZE * CHUNK_SIZE)
#define CHUNK_SIZE_3       (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

typedef struct {
    i32 x, y, z;
    Block blocks[CHUNK_SIZE_3];
    u32* mesh;
    i32 mesh_length;
    i32 mesh_idx;
    i32 num_faces;
    i32 face_counts[6];
    bool update_mesh;
    bool empty;
} Chunk;

typedef struct {
    i32 mesh_length;
    i32 draw_count;
    i32 indirect_length;
    i32 world_pos_length;
    i32 chunk_order_length;
    u32* mesh_buffer;
    u32* indirect_buffer;
    i32* world_pos_buffer;
    Chunk** chunks;
    i32 render_distance;
    i32 side;
    i32 num_chunks;
    u64 seed;
    struct {
        i32 x, y, z;
    } center;
    sem_t mutex;
    pthread_t thread_id;
    bool kill_thread;
} ChunkContext;

static ChunkContext ctx;

static i32 axis_dx[6] = {-1, 1, 0, 0, 0, 0};
static i32 axis_dy[6] = {0, 0, -1, 1, 0, 0};
static i32 axis_dz[6] = {0, 0, 0, 0, -1, 1};

#define block_idx(_x, _y, _z) \
    ((_x) + ((_y) << BITS_PER_AXIS) + ((_z) << BITS_PER_AXIS_2))

#define block_in_bounds(_x, _y, _z) \
      (_x >= 0 && _x < CHUNK_SIZE  \
    && _y >= 0 && _y < CHUNK_SIZE  \
    && _z >= 0 && _z < CHUNK_SIZE)

#define chunk_idx(_x, _y, _z, _cx, _cy, _cz) \
      ((_x - _cx + ctx.render_distance) \
    + ((_y - _cy + ctx.render_distance) * ctx.side) \
    + ((_z - _cz + ctx.render_distance) * ctx.side * ctx.side))

#define chunk_in_bounds(_x, _y, _z, _cx, _cy, _cz) \
      ((_x - _cx + ctx.render_distance) >= 0 \
    && (_x - _cx + ctx.render_distance) < ctx.side \
    && (_y - _cy + ctx.render_distance) >= 0 \
    && (_y - _cy + ctx.render_distance) < ctx.side \
    && (_z - _cz + ctx.render_distance) >= 0 \
    && (_z - _cz + ctx.render_distance) < ctx.side)

#define chunk_exists(_x, _y, _z, _cx, _cy, _cz, _chunks) \
      (chunk_in_bounds(_x, _y, _z, _cx, _cy, _cz) \
    && _chunks[chunk_idx(_x, _y, _z, _cx, _cy, _cz)] != NULL)

#define chunk_axis_faces_center(_chunk, _axis, _cx, _cy, _cz) \
       ((axis == NEGX && chunk->x >= _cx - 1)  \
    ||  (axis == POSX && chunk->x <= _cx + 1)  \
    ||  (axis == NEGY && chunk->y >= _cy - 1)  \
    ||  (axis == POSY && chunk->y <= _cy + 1)  \
    ||  (axis == NEGZ && chunk->z >= _cz - 1)  \
    ||  (axis == POSZ && chunk->z <= _cz + 1))

static f32 interpolate(f32 a0, f32 a1, f32 w)
{
    return (1.0 - w) * a0 + w * a1;
}

static vec2 grid_gradient(i32 grid_x, i32 grid_z)
{
    u8 tag = ((grid_x & 3) << 2) + (grid_z & 3);
    u64 seed = ((u64)grid_x << 32) + grid_z;
    return vec2_direction(randf(seed) * 2 * PI);
}

static void fill_chunk_y(Chunk* chunk, i32 x, i32 y, i32 z)
{
    f32 dot1, dot2, ix0, ix1, res;
    vec2 point, offset, weight;
    i32 grid_x, grid_z;

    grid_x = x >> 8;
    grid_z = z >> 8;

    point = vec2_create(x / 256.0, z / 256.0);
    weight = vec2_sub(point, vec2_create(grid_x, grid_z));

    offset = vec2_sub(point, vec2_create(grid_x, grid_z));
    dot1 = vec2_dot(offset, grid_gradient(grid_x, grid_z));
    offset = vec2_sub(point, vec2_create(grid_x+1, grid_z));
    dot2 = vec2_dot(offset, grid_gradient(grid_x+1, grid_z));
    ix0  = interpolate(dot1, dot2, weight.x);
    offset = vec2_sub(point, vec2_create(grid_x, grid_z+1));
    dot1 = vec2_dot(offset, grid_gradient(grid_x, grid_z+1));
    offset = vec2_sub(point, vec2_create(grid_x+1, grid_z+1));
    dot2 = vec2_dot(offset, grid_gradient(grid_x+1, grid_z+1));
    ix1  = interpolate(dot1, dot2, weight.x);
    res  = interpolate(ix0, ix1, weight.y);

    res *= 64;

    for (i32 i = 0; i < CHUNK_SIZE; i++) {
        if (y + i > res)
            break;
        chunk->blocks[block_idx(x & AXIS_MASK, i, z & AXIS_MASK)] = STONE;
        chunk->empty = FALSE;
    }
}

static Chunk* load_chunk(i32 cx, i32 cy, i32 cz)
{
    Chunk* chunk = calloc(1, sizeof(Chunk));
    chunk->x = cx;
    chunk->y = cy;
    chunk->z = cz;
    chunk->mesh = NULL;
    chunk->update_mesh = TRUE;
    chunk->empty = TRUE;
    
    if (cy < -2)
        return chunk;

    i32 x, y, z;
    for (x = 0; x < CHUNK_SIZE; x++)
        for (z = 0; z < CHUNK_SIZE; z++)
            fill_chunk_y(chunk, (cx << BITS_PER_AXIS) + x, cy << BITS_PER_AXIS, (cz << BITS_PER_AXIS) + z);
    
    return chunk;
}

static bool opaque_block(Chunk* chunk, Chunk** chunks, i32 idx, i32 axis, i32 cx, i32 cy, i32 cz)
{
    i32 bx, by, bz, x, y, z;
    bx = axis_dx[axis] + (idx & AXIS_MASK);
    by = axis_dy[axis] + ((idx >> BITS_PER_AXIS) & AXIS_MASK);
    bz = axis_dz[axis] + ((idx >> BITS_PER_AXIS_2) & AXIS_MASK);

    if (block_in_bounds(bx, by, bz))
        return chunk->blocks[block_idx(bx, by, bz)] != 0;

    bx &= AXIS_MASK;
    by &= AXIS_MASK;
    bz &= AXIS_MASK;
    x = axis_dx[axis] + chunk->x;
    y = axis_dy[axis] + chunk->y;
    z = axis_dz[axis] + chunk->z;
    
    if (chunk_exists(x, y, z, cx, cy, cz, chunks))
        return chunks[chunk_idx(x, y, z, cx, cy, cz)]->blocks[block_idx(bx, by, bz)] != 0;
    
    return TRUE;
}

static bool opaque_block_alt(Chunk* chunk, Chunk** chunks, i32 axis, i32 bx, i32 by, i32 bz, i32 cx, i32 cy, i32 cz)
{
    i32 x, y, z;
    bx = axis_dx[axis] + bx;
    by = axis_dy[axis] + by;
    bz = axis_dz[axis] + bz;
    if (block_in_bounds(bx, by, bz))
        return chunk->blocks[block_idx(bx, by, bz)] != 0;

    bx &= AXIS_MASK;
    by &= AXIS_MASK;
    bz &= AXIS_MASK;
    x = axis_dx[axis] + chunk->x;
    y = axis_dy[axis] + chunk->y;
    z = axis_dz[axis] + chunk->z;
    
    if (chunk_exists(x, y, z, cx, cy, cz, chunks))
        return chunks[chunk_idx(x, y, z, cx, cy, cz)]->blocks[block_idx(bx, by, bz)] != 0;
    
    return TRUE;
}

#define HELPER \
    switch (axis) { \
        case NEGX: case POSX: \
            x = k; y = i; z = j; break; \
        case NEGY: case POSY: \
            x = j; y = k; z = i; break; \
        case NEGZ: case POSZ: \
            x = i; y = j; z = k; break; \
    }

static Block get_block(Chunk* chunk, Chunk** chunks, i32 axis, i32 k, i32 j, i32 i, i32 cx, i32 cy, i32 cz)
{
    i32 x, y, z;
    HELPER
    if (opaque_block_alt(chunk, chunks, axis, x, y, z, cx, cy, cz))
        return AIR;
    return chunk->blocks[block_idx(x, y, z)];
}

void preinsert_block_into_mesh(Chunk* chunk, Block block, i32 axis, i32 k, i32 j, i32 i, i32 dj, i32 di)
{
    chunk->face_counts[axis]++;
    chunk->num_faces++;
}

void insert_block_into_mesh(Chunk* chunk, Block block, i32 axis, i32 k, i32 j, i32 i, i32 di, i32 dj)
{
    i32 x, y, z, w, h;
    u32 info1, info2;
    HELPER
    w = (axis & 1) ? dj : di;
    h = (axis & 1) ? di : dj;
    info1 = 0;
    info1 |=  x & AXIS_MASK;
    info1 |= (y & AXIS_MASK) << BITS_PER_AXIS;
    info1 |= (z & AXIS_MASK) << BITS_PER_AXIS_2;
    info1 |= ((w-1) & AXIS_MASK) << BITS_PER_AXIS_3;
    info1 |= ((h-1) & AXIS_MASK) << BITS_PER_AXIS_4;
    info1 |= (block_face(block, axis)) << BITS_PER_AXIS_5;
    chunk->mesh[chunk->mesh_length++] = info1;
}

#undef HELPER

static void greedy_mesh(Chunk* chunk, Chunk** chunks, i32 cx, i32 cy, i32 cz, void (*func)())
{
    i32 i, j, k, tmp_i1, tmp_i2, tmp_j, axis, mask;
    u32 flags[CHUNK_SIZE];
    for (axis = 0; axis < 6; axis++) {
        for (k = 0; k < CHUNK_SIZE; k++) {
            for (j = 0; j < CHUNK_SIZE; j++)
                flags[j] = 0;
            i = 0;
            j = 0;
            while (j < CHUNK_SIZE) {
                if (i == CHUNK_SIZE) {
                    i = 0;
                    j++;
                    continue;
                }
                Block block = get_block(chunk, chunks, axis, k, j, i, cx, cy, cz);
                tmp_i1 = i;
                mask = 1 << (tmp_i1++);
                if (block == AIR || (flags[j] & mask)) {
                    i++;
                    continue;
                }
                while (tmp_i1 < CHUNK_SIZE && !((flags[j] >> tmp_i1) & 1) 
                  && get_block(chunk, chunks, axis, k, j, tmp_i1, cx, cy, cz) == block)
                    mask |= 1 << (tmp_i1++);
                tmp_j = j;
                flags[tmp_j++] |= mask;
                while (tmp_j < CHUNK_SIZE) {
                    for (tmp_i2 = i; tmp_i2 < tmp_i1; tmp_i2++)
                        if (((flags[tmp_j] >> tmp_i2) & 1) || get_block(chunk, chunks, axis, k, tmp_j, tmp_i2, cx, cy, cz) != block)
                            goto done;
                    flags[tmp_j++] |= mask;
                }
                done:
                func(chunk, block, axis, k, j, i, tmp_i1 - i, tmp_j - j);
                i = tmp_i1;
            }
        }
    }
}

static void build_chunk_mesh_greedy(Chunk* chunk, Chunk** chunks, i32 cx, i32 cy, i32 cz)
{
    chunk->num_faces = 0;

    for (i32 axis = 0; axis < 6; axis++)
        chunk->face_counts[axis] = 0;

    greedy_mesh(chunk, chunks, cx, cy, cz, preinsert_block_into_mesh);

    if (chunk->num_faces == 0)
        return;
    
    chunk->mesh = malloc(chunk->num_faces * sizeof(u32));
    chunk->mesh_length = 0;

    greedy_mesh(chunk, chunks, cx, cy, cz, insert_block_into_mesh);
}

static void build_chunk_mesh_simple(Chunk* chunk, Chunk** chunks, i32 cx, i32 cy, i32 cz)
{
    chunk->num_faces = 0;
    chunk->mesh_length = 0;

    for (i32 axis = 0; axis < 6; axis++)
        chunk->face_counts[axis] = 0;

    for (i32 i = 0; i < CHUNK_SIZE_3; i++) {
        if (chunk->blocks[i] != 0) {
            for (i32 axis = 0; axis < 6; axis++) {
                if (!opaque_block(chunk, chunks, i, axis, cx, cy, cz)) {
                    chunk->face_counts[axis]++;
                    chunk->num_faces++;
                }
            }
        }
    }

    if (chunk->num_faces == 0)
        return;

    chunk->mesh = malloc(chunk->num_faces * sizeof(u32));

    u32 prefix[7];
    prefix[0] = 0;
    for (i32 i = 1; i < 7; i++)
        prefix[i] = prefix[i-1] + chunk->face_counts[i-1];
    chunk->mesh_length = prefix[6];

    u32 idxs[6];
    for (i32 i = 0; i < 6; i++)
        idxs[i] = 0;
    
    u32 info1, info2;
    for (i32 i = 0; i < CHUNK_SIZE_3; i++) {
        if (chunk->blocks[i] != 0) {
            info1 = 0;
            info1 |=  i & AXIS_MASK;
            info1 |= ((i>>BITS_PER_AXIS) & AXIS_MASK) << BITS_PER_AXIS;
            info1 |= ((i>>BITS_PER_AXIS_2) & AXIS_MASK) << BITS_PER_AXIS_2;
            for (i32 axis = 0; axis < 6; axis++) {
                if (!opaque_block(chunk, chunks, i, axis, cx, cy, cz)) {
                    info2 = info1 | (block_face(chunk->blocks[i], axis) << BITS_PER_AXIS_5);
                    chunk->mesh[(idxs[axis]++)+prefix[axis]] = info2;
                }
            }
        }
    }
}

static void build_chunk_mesh(Chunk* chunk, Chunk** chunks, i32 cx, i32 cy, i32 cz)
{
    if (chunk->empty)
        return;

    build_chunk_mesh_greedy(chunk, chunks, cx, cy, cz);
}

static void unload_chunk(Chunk* chunk)
{
    free(chunk->mesh);
    free(chunk);
}

static void destroy_chunk_mesh(Chunk* chunk)
{
    free(chunk->mesh);
    chunk->mesh_length = 0;
    chunk->mesh = NULL;
}

static i32 compare_chunk_idx(const void* ptr1, const void* ptr2)
{
    i32 idx1, idx2;
    i32 dx1, dy1, dz1, dx2, dy2, dz2;
    idx1 = *(i32*)ptr1;
    idx2 = *(i32*)ptr2;
    dx1 = abs(ctx.render_distance - idx1 % ctx.side);
    dy1 = abs(ctx.render_distance - (idx1 / ctx.side) % ctx.side);
    dz1 = abs(ctx.render_distance - idx1 / ctx.side / ctx.side);
    dx2 = abs(ctx.render_distance - idx2 % ctx.side);
    dy2 = abs(ctx.render_distance - (idx2 / ctx.side) % ctx.side);
    dz2 = abs(ctx.render_distance - idx2 / ctx.side / ctx.side);
    return dx1 - dx2 + dy1 - dy2 + dz1 - dz2;
}

#define CHUNKS_PER_UPDATE 5
static void* chunk_update(void* vargp)
{
    vec3 position;
    i32 cx, cy, cz;
    i32 new_mesh_length;
    i32 new_indirect_length;
    i32 group_size;
    i32* lengths;
    i32* prefix;
    u32* mesh_buffer_swap;
    Chunk** chunks_swap;
    u32* indirect_buffer_swap;
    i32* world_pos_buffer_swap;
    i32* sorted_chunk_idx;
    Stack* unload_stack;
    void* tmp;
    i32 kill_thread = FALSE;

    unload_stack = stack_create();

    #pragma omp parallel
    {
        i32 num_threads, thread_num;
        i32 num_chunks_processed;
        i32 mesh_idx, axis;
        i32 i, j, x, y, z;
        Chunk* chunk;
        
        num_threads = omp_get_num_threads();
        thread_num = omp_get_thread_num();

        #pragma omp master
        {
            group_size = ctx.num_chunks / num_threads;
            lengths = malloc(num_threads * sizeof(i32));
            prefix = malloc((num_threads + 1) * sizeof(i32));
            chunks_swap = calloc(ctx.num_chunks, sizeof(Chunk*));
            indirect_buffer_swap = malloc(6 * 4 * ctx.num_chunks * sizeof(u32));
            world_pos_buffer_swap = malloc(6 * 4 * ctx.num_chunks * sizeof(i32));
            mesh_buffer_swap = NULL;
            sorted_chunk_idx = malloc(ctx.num_chunks * sizeof(i32));
            for (i32 i = 0; i < ctx.num_chunks; i++)
                sorted_chunk_idx[i] = i;
            qsort(sorted_chunk_idx, ctx.num_chunks, sizeof(i32), compare_chunk_idx);
        }

        while (!kill_thread)
        {
            #pragma omp master
            {
                position = camera_position();
                cx = position.x / CHUNK_SIZE - (position.x < 0);
                cy = position.y / CHUNK_SIZE - (position.y < 0);
                cz = position.z / CHUNK_SIZE - (position.z < 0);
            }
            #pragma omp barrier

            for (i = thread_num; i < ctx.num_chunks; i += num_threads) {
                x = ctx.center.x - ctx.render_distance + (i % ctx.side);
                y = ctx.center.y - ctx.render_distance + ((i / ctx.side) % ctx.side);
                z = ctx.center.z - ctx.render_distance + (i / ctx.side / ctx.side);
                chunk = ctx.chunks[i];
                if (!chunk_in_bounds(x, y, z, cx, cy, cz) && chunk != NULL) {
                    #pragma omp critical
                    {
                        stack_push(unload_stack, chunk);
                    }
                    ctx.chunks[i] = NULL;
                }
            }
            #pragma omp barrier

            num_chunks_processed = 0;
            chunk = NULL;
            while (num_chunks_processed < CHUNKS_PER_UPDATE)
            {
                #pragma omp critical
                {
                    if (!stack_empty(unload_stack))
                        chunk = stack_pop(unload_stack);
                }
                if (chunk == NULL)
                    break;
                unload_chunk(chunk);
                chunk = NULL;
                num_chunks_processed++;
            }
            #pragma omp barrier

            num_chunks_processed = 0;
            lengths[thread_num] = 0;
            for (j = thread_num; j < ctx.num_chunks; j += num_threads) {
                i = sorted_chunk_idx[j];
                x = cx - ctx.render_distance + (i % ctx.side);
                y = cy - ctx.render_distance + ((i / ctx.side) % ctx.side);
                z = cz - ctx.render_distance + (i / ctx.side / ctx.side);
                if (chunk_exists(x, y, z, ctx.center.x, ctx.center.y, ctx.center.z, ctx.chunks)) {
                    chunks_swap[i] = ctx.chunks[chunk_idx(x, y, z, ctx.center.x, ctx.center.y, ctx.center.z)];
                } else {
                    if (num_chunks_processed > CHUNKS_PER_UPDATE) {
                        chunks_swap[i] = NULL;
                    } else {
                        for (axis = 0; axis < 6; axis++) {
                            if (chunk_exists(x + axis_dx[axis], y + axis_dy[axis], z + axis_dz[axis], ctx.center.x, ctx.center.y, ctx.center.z, ctx.chunks)) {
                                chunk = ctx.chunks[chunk_idx(x + axis_dx[axis], y + axis_dy[axis], z + axis_dz[axis], ctx.center.x, ctx.center.y, ctx.center.z)];
                                chunk->update_mesh = TRUE;
                            }
                        }
                        chunks_swap[i] = load_chunk(x, y, z);
                        lengths[thread_num]++;
                        num_chunks_processed++;
                    }
                }
            }
            #pragma omp barrier

            lengths[thread_num] = 0;
            for (i = thread_num; i < ctx.num_chunks; i += num_threads) {
                chunk = chunks_swap[i];
                if (chunk == NULL)
                    continue;
                if (chunk->update_mesh) {
                    destroy_chunk_mesh(chunk);
                    build_chunk_mesh(chunk, chunks_swap, cx, cy, cz);
                    chunk->update_mesh = FALSE;
                }
                lengths[thread_num] += chunk->mesh_length;
            }
            #pragma omp barrier

            #pragma omp master
            {
                prefix[0] = 0;
                for (i = 0; i < num_threads; i++)
                    prefix[i+1] = lengths[i] + prefix[i];
                free(mesh_buffer_swap);
                mesh_buffer_swap = malloc(prefix[num_threads] * sizeof(u32));
                new_mesh_length = prefix[num_threads];
            }
            #pragma omp barrier

            for (i = thread_num; i < ctx.num_chunks; i += num_threads) {
                chunk = chunks_swap[i];
                if (chunk == NULL)
                    continue;
                chunk->mesh_idx = prefix[thread_num];
                for (j = 0; j < chunk->mesh_length; j++)
                    mesh_buffer_swap[prefix[thread_num]++] = chunk->mesh[j];
            }
            #pragma omp barrier

            lengths[thread_num] = 0;
            for (i = thread_num * group_size; i < (thread_num + 1) * group_size; i++) {
                if (i >= ctx.num_chunks) 
                    break;
                Chunk* chunk = chunks_swap[sorted_chunk_idx[i]];
                if (chunk == NULL) 
                    continue;
                for (axis = 0; axis < 6; axis++)
                    if (chunk->face_counts[axis] != 0) 
                        lengths[thread_num] += 4;
            }
            #pragma omp barrier

            #pragma omp master
            {
                prefix[0] = 0;
                for (i = 0; i < num_threads; i++)
                    prefix[i+1] = lengths[i] + prefix[i];
                new_indirect_length = prefix[num_threads];
            }
            #pragma omp barrier

            for (i = thread_num * group_size; i < (thread_num + 1) * group_size; i++) {
                if (i >= ctx.num_chunks)
                    break;
                Chunk* chunk = chunks_swap[sorted_chunk_idx[i]];
                if (chunk == NULL || chunk->mesh == NULL) 
                    continue;
                mesh_idx = chunk->mesh_idx;
                for (axis = 0; axis < 6; axis++) {
                    if (chunk->face_counts[axis] != 0) 
                    {
                        indirect_buffer_swap[prefix[thread_num]]    = 4;
                        indirect_buffer_swap[prefix[thread_num]+1]  = chunk->face_counts[axis];
                        indirect_buffer_swap[prefix[thread_num]+2]  = 0;
                        indirect_buffer_swap[prefix[thread_num]+3]  = mesh_idx;
                        world_pos_buffer_swap[prefix[thread_num]]   = CHUNK_SIZE * chunk->x;
                        world_pos_buffer_swap[prefix[thread_num]+1] = CHUNK_SIZE * chunk->y;
                        world_pos_buffer_swap[prefix[thread_num]+2] = CHUNK_SIZE * chunk->z;
                        world_pos_buffer_swap[prefix[thread_num]+3] = axis;
                        prefix[thread_num] += 4;
                    }
                    mesh_idx += chunk->face_counts[axis];
                }
            }
            #pragma omp barrier

            #pragma omp master
            {
                sem_wait(&ctx.mutex);
                tmp = ctx.chunks;
                ctx.chunks = chunks_swap;
                chunks_swap = tmp;

                tmp = ctx.mesh_buffer;
                ctx.mesh_buffer = mesh_buffer_swap;
                mesh_buffer_swap = tmp;

                tmp = ctx.indirect_buffer;
                ctx.indirect_buffer = indirect_buffer_swap;
                indirect_buffer_swap = tmp;

                tmp = ctx.world_pos_buffer;
                ctx.world_pos_buffer = world_pos_buffer_swap;
                world_pos_buffer_swap = tmp;

                ctx.center.x = cx;
                ctx.center.y = cy;
                ctx.center.z = cz;

                ctx.mesh_length = new_mesh_length;
                ctx.indirect_length = new_indirect_length;
                ctx.world_pos_length = new_indirect_length;
                sem_post(&ctx.mutex);
            }
            #pragma omp barrier

            #pragma omp master
            {
                if (ctx.kill_thread)
                    kill_thread = TRUE;
            }
            #pragma omp barrier
        }

        #pragma omp master
        {
            free(lengths);
            free(prefix);
            free(chunks_swap);
            free(mesh_buffer_swap);
            free(indirect_buffer_swap);
            free(world_pos_buffer_swap);
            free(sorted_chunk_idx);
        }
    }

    stack_destroy(unload_stack);
}

static i32 min_distance_idx(f32 distances[3])
{
    i32 i = 0;
    if (distances[1] < distances[i])
        i = 1;
    if (distances[2] < distances[i])
        i = 2;
    return i;
}

static Chunk* get_chunk(i32 chunk_x, i32 chunk_y, i32 chunk_z)
{
    Chunk* chunk;
    if (!chunk_in_bounds(chunk_x, chunk_y, chunk_z, ctx.center.x, ctx.center.y, ctx.center.z))
        return NULL;
    sem_wait(&ctx.mutex);
    chunk = ctx.chunks[chunk_idx(chunk_x, chunk_y, chunk_z,
                                 ctx.center.x, ctx.center.y, ctx.center.z)];
    sem_post(&ctx.mutex);
    return chunk;
}

Block chunk_break_block(void)
{
    return AIR;
    // race conditions
    /* Chunk* chunk;
    Block block;
    vec3 pos, face;
    f32 position[3];
    f32 facing[3];
    i32 block_pos[3];
    f32 distances[3];
    pos = camera_position();
    face = camera_facing();
    position[0] = pos.x;
    position[1] = pos.y;
    position[2] = pos.z;
    facing[0] = face.x;
    facing[1] = face.y;
    facing[2] = face.z;
    assert(facing[0] != 0 || facing[1] != 0 || facing[2] != 0);
    for (i32 i = 0; i < 3; i++) {
        block_pos[i] = position[i] + (facing[i] > 0);
        distances[i] = (facing[i] == 0) ? INF : fabs((block_pos[i] - position[i]) / facing[i]);
    }
    i32 idx = min_distance_idx(distances);
    i32 block_idx;
    #define BREAK_DISTANCE 10
    while (distances[idx] < BREAK_DISTANCE) {
        chunk = get_chunk(block_pos[0] / 32, block_pos[1] / 32, block_pos[2] / 32);
        block_idx = block_idx(block_pos[0] & 31, block_pos[1] & 31, block_pos[2] & 31);
        block = chunk->blocks[block_idx];
        if (block != AIR) {
            chunk->blocks[block_idx] = AIR;
            chunk->update_mesh = TRUE;
            return block;
        }
        distances[idx] += 1 / facing[idx];
        block_pos[idx] += (facing[idx] > 0) ? 1 : -1;
        idx = min_distance_idx(distances);
    }
    return AIR; */
}

void chunk_place_block(Block block)
{

}


void chunk_init(void)
{
    sem_init(&ctx.mutex, 0, 1);
    pthread_create(&ctx.thread_id, NULL, chunk_update, NULL);   
    ctx.mesh_length = 0;
    ctx.indirect_length = 0;
    ctx.world_pos_length = 0;
    ctx.mesh_buffer = NULL;
    ctx.render_distance = RENDER_DISTANCE;
    ctx.kill_thread = FALSE;
    ctx.center.x = ctx.center.y = ctx.center.z = 0;
    ctx.side = ctx.render_distance * 2 + 1;
    ctx.num_chunks = ctx.side * ctx.side * ctx.side;

    ctx.indirect_buffer = malloc(6 * 4 * ctx.num_chunks * sizeof(u32));
    ctx.world_pos_buffer = malloc(6 * 4 * ctx.num_chunks * sizeof(i32));
    ctx.chunks = calloc(ctx.num_chunks, sizeof(Chunk*));

    dibo_bind(DIBO_GAME);
    dibo_malloc(DIBO_GAME, 6 * 4 * ctx.num_chunks * sizeof(u32), GL_STATIC_DRAW);
    ssbo_bind(SSBO_GAME);
    ssbo_malloc(SSBO_GAME, 6 * 4 * ctx.num_chunks * sizeof(i32), GL_STATIC_DRAW);
    glUniform1i(shader_get_uniform_location(SHADER_GAME, "bits_per_axis"), BITS_PER_AXIS);
}

void chunk_prepare_render(void)
{
    ctx.draw_count = -1;
    if (ctx.mesh_buffer == NULL)
        return;

    sem_wait(&ctx.mutex);
    ctx.draw_count = ctx.indirect_length / 4;
    vbo_bind(VBO_GAME_INSTANCE);
    vbo_malloc(VBO_GAME_INSTANCE, ctx.mesh_length * sizeof(u32), GL_STATIC_DRAW);
    vbo_update(VBO_GAME_INSTANCE, 0, ctx.mesh_length * sizeof(u32), ctx.mesh_buffer);
    dibo_bind(DIBO_GAME);
    dibo_update(DIBO_GAME, 0, ctx.indirect_length * sizeof(u32), ctx.indirect_buffer);
    ssbo_bind(SSBO_GAME);
    ssbo_update(SSBO_GAME, 0, ctx.world_pos_length * sizeof(i32), ctx.world_pos_buffer);
    sem_post(&ctx.mutex);
}

void chunk_render(void)
{
    if (ctx.draw_count == -1)
        return;

    shader_use(SHADER_GAME);
    vao_bind(VAO_GAME);
    ebo_bind(EBO_GAME);
    glMultiDrawArraysIndirect(GL_TRIANGLE_STRIP, 0, ctx.draw_count, 0);
}

void chunk_destroy(void)
{
    ctx.kill_thread = TRUE;
    pthread_join(ctx.thread_id, NULL);
    for (i32 i = 0; i < ctx.num_chunks; i++)
        free(ctx.chunks[i]);

    free(ctx.chunks);
    free(ctx.mesh_buffer);
    free(ctx.indirect_buffer);
    free(ctx.world_pos_buffer);
    sem_destroy(&ctx.mutex);
}