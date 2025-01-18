#include "chunk.h"
#include "block.h"
#include "../camera/camera.h"
#include "../renderer/renderer.h"
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>

#define RENDER_DISTANCE 5
#define CHUNKS_PER_UPDATE 50

typedef struct {
    i32 x, y, z;
    Block blocks[32768];
    u32* mesh;
    i32 mesh_length;
    i32 mesh_idx;
    i32 num_faces;
    i32 face_counts[6];
    bool update;
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
    i32 render_distance;
    i32 side;
    i32 num_chunks;
    i32* sorted_chunk_idx;
    u64 seed;
    struct {
        i32 x, y, z;
    } center;
    sem_t mutex;
    pthread_t thread_id;
    bool kill_thread;
} Chunkctx;

static Chunkctx ctx;

static i32 axis_dx[6] = {-1, 1, 0, 0, 0, 0};
static i32 axis_dy[6] = {0, 0, -1, 1, 0, 0};
static i32 axis_dz[6] = {0, 0, 0, 0, -1, 1};

#define block_idx(_x, _y, _z) \
    ((_x) + ((_y) << 5) + ((_z) << 10))

#define block_in_bounds(_x, _y, _z) \
    (_x >= 0 && _x < 32 && _y >= 0 && _y < 32 && _z >= 0 && _z < 32)

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

static bool opaque_block(Chunk* chunk, Chunk** chunks, i32 idx, i32 axis, i32 cx, i32 cy, i32 cz)
{
    i32 bx, by, bz, x, y, z;
    bx = axis_dx[axis] + (idx & 31);
    by = axis_dy[axis] + ((idx >> 5) & 31);
    bz = axis_dz[axis] + ((idx >> 10) & 31);

    if (block_in_bounds(bx, by, bz))
        return chunk->blocks[block_idx(bx, by, bz)] != 0;

    bx &= 31;
    by &= 31;
    bz &= 31;
    x = axis_dx[axis] + chunk->x;
    y = axis_dy[axis] + chunk->y;
    z = axis_dz[axis] + chunk->z;
    
    if (chunk_exists(x, y, z, cx, cy, cz, chunks))
        return chunks[chunk_idx(x, y, z, cx, cy, cz)]->blocks[block_idx(bx, by, bz)] != 0;
    
    return TRUE;
}

static f32 interpolate(f32 a0, f32 a1, f32 w)
{
    return (1.0 - w) * a0 + w * a1;
}

static vec2 grid_gradient(i32 grid_x, i32 grid_z)
{
    u8 tag = ((grid_x & 3) << 2) + (grid_z & 3);
    u64 seed = ((u64)grid_x<<32) + grid_z;
    return vec2_direction(randf(seed) * 2 * PI);
}

static f32 fill_chunk_y(Chunk* chunk, i32 x, i32 y, i32 z)
{
    f32 dot1, dot2, ix0, ix1, res;
    vec2 point, offset, weight;
    i32 grid_x, grid_z;

    grid_x = x >> 5;
    grid_z = z >> 5;

    point = vec2_create(x / 32.0, z / 32.0);
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

    for (i32 i = 0; i < 32; i++) {
        if (y + i > res)
            break;
        chunk->blocks[block_idx(x & 31, i, z & 31)] = STONE;
    }

    return res;
}

static Chunk* load_chunk(i32 cx, i32 cy, i32 cz)
{
    Chunk* chunk = calloc(1, sizeof(Chunk));
    chunk->x = cx;
    chunk->y = cy;
    chunk->z = cz;
    chunk->mesh = NULL;
    chunk->update = TRUE;
    
    if (cy < -2)
        return chunk;

    f32 map[32][32];
    i32 x, y, z;
    for (x = 0; x < 32; x++)
        for (z = 0; z < 32; z++)
            map[x][z] = fill_chunk_y(chunk, (cx << 5) + x, cy << 5, (cz << 5) + z);
    
    return chunk;
}

static void build_chunk_mesh(Chunk* chunk, Chunk** chunks, i32 cx, i32 cy, i32 cz)
{
    chunk->num_faces = 0;
    chunk->mesh_length = 0;

    for (i32 axis = 0; axis < 6; axis++)
        chunk->face_counts[axis] = 0;

    for (i32 i = 0; i < 32768; i++) {
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
    for (i32 i = 0; i < 32768; i++) {
        if (chunk->blocks[i] != 0) {
            info1 = 0;
            info1 |=  i & 31;
            info1 |= ((i>>5) & 31) << 5;
            info1 |= ((i>>10) & 31) << 10;
            for (i32 axis = 0; axis < 6; axis++) {
                if (!opaque_block(chunk, chunks, i, axis, cx, cy, cz)) {
                    info2 = info1 | (block_face(chunk->blocks[i], axis) << 15);
                    chunk->mesh[(idxs[axis]++)+prefix[axis]] = info2;
                }
            }
        }
    }
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

static void* chunk_update(void* vargp)
{
    vec3 position;
    i32 cx, cy, cz;
    i32 side;
    i32 num_chunks_loaded;
    i32 new_mesh_length;
    i32 new_indirect_length;
    i32 group_size;
    i32* lengths;
    i32* prefix;
    u32* mesh_buffer_swap;
    Chunk** chunks_swap;
    u32* indirect_buffer_swap;
    i32* world_pos_buffer_swap;
    void* tmp;
    #pragma omp parallel
    {
        i32 num_threads, thread_num;
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
        }

        while (!ctx.kill_thread)
        {
            #pragma omp master
            {
                position = camera_position();
                cx = position.x / 32 - (position.x < 0);
                cy = position.y / 32 - (position.y < 0);
                cz = position.z / 32 - (position.z < 0);
                side = ctx.side;
                num_chunks_loaded = 0;
            }
            #pragma omp barrier

            for (i = thread_num; i < ctx.num_chunks; i += num_threads) {
                x = ctx.center.x - ctx.render_distance + (i % side);
                y = ctx.center.y - ctx.render_distance + ((i / side) % side);
                z = ctx.center.z - ctx.render_distance + (i / side / side);
                chunk = ctx.chunks[i];
                if (!chunk_in_bounds(x, y, z, cx, cy, cz) && chunk != NULL) {
                    unload_chunk(chunk);
                    ctx.chunks[i] = NULL;
                }
            }
            #pragma omp barrier

            for (j = thread_num; j < ctx.num_chunks; j += num_threads) {
                i = ctx.sorted_chunk_idx[j];
                x = cx - ctx.render_distance + (i % side);
                y = cy - ctx.render_distance + ((i / side) % side);
                z = cz - ctx.render_distance + (i / side / side);
                if (chunk_exists(x, y, z, ctx.center.x, ctx.center.y, ctx.center.z, ctx.chunks)) {
                    chunks_swap[i] = ctx.chunks[chunk_idx(x, y, z, ctx.center.x, ctx.center.y, ctx.center.z)];
                } else {
                    if (num_chunks_loaded > CHUNKS_PER_UPDATE) {
                        chunks_swap[i] = NULL;
                    } else {
                        #pragma omp atomic update
                        num_chunks_loaded++;
                        for (axis = 0; axis < 6; axis++) {
                            if (chunk_exists(x + axis_dx[axis], y + axis_dy[axis], z + axis_dz[axis], ctx.center.x, ctx.center.y, ctx.center.z, ctx.chunks)) {
                                chunk = ctx.chunks[chunk_idx(x + axis_dx[axis], y + axis_dy[axis], z + axis_dz[axis], ctx.center.x, ctx.center.y, ctx.center.z)];
                                chunk->update = TRUE;
                            }
                        }
                        chunks_swap[i] = load_chunk(x, y, z);
                    }
                }
            }
            #pragma omp barrier

            lengths[thread_num] = 0;
            for (i = thread_num; i < ctx.num_chunks; i += num_threads) {
                chunk = chunks_swap[i];
                if (chunk == NULL)
                    continue;
                if (chunk->update) {
                    destroy_chunk_mesh(chunk);
                    build_chunk_mesh(chunk, chunks_swap, cx, cy, cz);
                    chunk->update = FALSE;
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
                Chunk* chunk = chunks_swap[ctx.sorted_chunk_idx[i]];
                if (chunk == NULL) 
                    continue;
                for (axis = 0; axis < 6; axis++)
                    if (chunk->face_counts[axis] != 0 && chunk_axis_faces_center(chunk, axis, cx, cy, cz)) 
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
                Chunk* chunk = chunks_swap[ctx.sorted_chunk_idx[i]];
                if (chunk == NULL) 
                    continue;
                mesh_idx = chunk->mesh_idx;
                for (axis = 0; axis < 6; axis++) {
                    if (chunk->face_counts[axis] != 0 && chunk_axis_faces_center(chunk, axis, cx, cy, cz)) 
                    {
                        indirect_buffer_swap[prefix[thread_num]]    = 4;
                        indirect_buffer_swap[prefix[thread_num]+1]  = chunk->face_counts[axis];
                        indirect_buffer_swap[prefix[thread_num]+2]  = 0;
                        indirect_buffer_swap[prefix[thread_num]+3]  = mesh_idx;
                        world_pos_buffer_swap[prefix[thread_num]]   = 32 * chunk->x;
                        world_pos_buffer_swap[prefix[thread_num]+1] = 32 * chunk->y;
                        world_pos_buffer_swap[prefix[thread_num]+2] = 32 * chunk->z;
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
        }

        #pragma omp master
        {
            free(lengths);
            free(prefix);
            free(chunks_swap);
            free(mesh_buffer_swap);
            free(indirect_buffer_swap);
            free(world_pos_buffer_swap);
        }
    }
}

static i32 compare_chunk_idx(const void* ptr1, const void* ptr2)
{
    i32 idx1, idx2;
    i32 dx1, dy1, dz1, dx2, dy2, dz2;
    idx1 = *(int*)ptr1;
    idx2 = *(int*)ptr2;
    dx1 = abs(ctx.render_distance - idx1 % ctx.side);
    dy1 = abs(ctx.render_distance - (idx1 / ctx.side) % ctx.side);
    dz1 = abs(ctx.render_distance - idx1 / ctx.side / ctx.side);
    dx2 = abs(ctx.render_distance - idx2 % ctx.side);
    dy2 = abs(ctx.render_distance - (idx2 / ctx.side) % ctx.side);
    dz2 = abs(ctx.render_distance - idx2 / ctx.side / ctx.side);
    return dx1 - dx2 + dy1 - dy2 + dz1 - dz2;
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

    ctx.sorted_chunk_idx = malloc(ctx.num_chunks * sizeof(i32));
    for (i32 i = 0; i < ctx.num_chunks; i++)
        ctx.sorted_chunk_idx[i] = i;
    qsort(ctx.sorted_chunk_idx, ctx.num_chunks, sizeof(i32), compare_chunk_idx);

    dibo_bind(DIBO_GAME);
    dibo_malloc(DIBO_GAME, 6 * 4 * ctx.num_chunks * sizeof(u32), GL_STATIC_DRAW);
    ssbo_bind(SSBO_GAME);
    ssbo_malloc(SSBO_GAME, 6 * 4 * ctx.num_chunks * sizeof(i32), GL_STATIC_DRAW);
}

void chunk_prepare_render(void)
{
    
}

void chunk_render(void)
{
    if (ctx.mesh_buffer == NULL)
        return;
    
    i32 indirect_length;
    sem_wait(&ctx.mutex);
    indirect_length = ctx.indirect_length;
    vbo_bind(VBO_GAME_INSTANCE);
    vbo_malloc(VBO_GAME_INSTANCE, ctx.mesh_length * sizeof(u32), GL_STATIC_DRAW);
    vbo_update(VBO_GAME_INSTANCE, 0, ctx.mesh_length * sizeof(u32), ctx.mesh_buffer);
    dibo_bind(DIBO_GAME);
    dibo_update(DIBO_GAME, 0, ctx.indirect_length * sizeof(u32), ctx.indirect_buffer);
    ssbo_bind(SSBO_GAME);
    ssbo_update(SSBO_GAME, 0, ctx.world_pos_length * sizeof(i32), ctx.world_pos_buffer);
    sem_post(&ctx.mutex);

    shader_use(SHADER_GAME);
    vao_bind(VAO_GAME);
    ebo_bind(EBO_GAME);
    glMultiDrawArraysIndirect(GL_TRIANGLE_STRIP, 0, indirect_length / 4, 0);
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
    free(ctx.sorted_chunk_idx);
    sem_destroy(&ctx.mutex);
}