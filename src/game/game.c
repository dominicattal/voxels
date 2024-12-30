#include "game.h"
#include "block.h"
#include "../renderer/renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_BLOCKS          100000
#define FLOATS_PER_VERTEX   5
#define VERTICES_PER_FACE   4
#define INDICES_PER_FACE    6
#define FACES_PER_BLOCK     6

typedef struct {
    u32 vbo_length, vbo_max_length;
    f32* vbo_buffer;
} GameData;

typedef struct {
    GameData data;
    f64 dt;
    pthread_t thread_id;
    bool kill_thread;
    sem_t mutex;
    Block blocks[NUM_BLOCKS];
} Game;

static Game game;

static void initialize_instance_mesh(void);

static void *game_update(void* vargp)
{
    f64 start;
    while (!game.kill_thread) {
        sem_wait(&game.mutex);
        start = get_time();
        game.dt = get_time() - start;
        sem_post(&game.mutex);
    }
}

void game_init(void)
{
    for (i32 i = 0; i < NUM_BLOCKS; i++)
        game.blocks[i] = block_create(3 + (rand() % 3), rand()%1000,rand()%32, rand()%1000);

    game.data.vbo_max_length = NUM_BLOCKS * 8;
    game.data.vbo_buffer = malloc(game.data.vbo_max_length * sizeof(f32));
    initialize_instance_mesh();

    game.dt = 0;
    game.kill_thread = FALSE;
    sem_init(&game.mutex, 0, 1);
    pthread_create(&game.thread_id, NULL, game_update, NULL);

    game.data.vbo_length = 0;

    for (i32 i = 0; i < NUM_BLOCKS; i++) {
        Block block = game.blocks[i];
        game.data.vbo_buffer[game.data.vbo_length++] = block.position.x;
        game.data.vbo_buffer[game.data.vbo_length++] = block.position.y;
        game.data.vbo_buffer[game.data.vbo_length++] = block.position.z;
        game.data.vbo_buffer[game.data.vbo_length++] = block.id;
    }

    vbo_bind(VBO_GAME_INSTANCE);
    vbo_malloc(VBO_GAME_INSTANCE, game.data.vbo_max_length * sizeof(f32), GL_STATIC_DRAW);
    vbo_update(VBO_GAME_INSTANCE, 0, game.data.vbo_length * sizeof(f32), game.data.vbo_buffer);
}

void game_destroy(void)
{
    free(game.data.vbo_buffer);
    game.kill_thread = TRUE;
    pthread_join(game.thread_id, NULL);
    sem_destroy(&game.mutex);
}

f64 game_dt(void)
{
    return game.dt;
}

void initialize_instance_mesh(void)
{
    u8 dx[] = {0, 0, 0, 0, 1, 1, 1, 1};
    u8 dy[] = {0, 0, 1, 1, 0, 0, 1, 1};
    u8 dz[] = {0, 1, 0, 1, 0, 1, 0, 1};
    u8 tx[] = {0, 1, 1, 0};
    u8 ty[] = {0, 0, 1, 1};
    u8 winding[] = {0, 1, 2, 0, 2, 3};
    u8 faces[][4] = {
        {4, 5, 7, 6}, // +x
        {1, 0, 2, 3}, // -x
        {2, 6, 7, 3}, // +y
        {0, 1, 5, 4}, // -y
        {5, 1, 3, 7}, // +z
        {0, 4, 6, 2}  // -z
    };
    f32 vbo_buffer[FACES_PER_BLOCK * VERTICES_PER_FACE * 5];
    u32 ebo_buffer[FACES_PER_BLOCK * INDICES_PER_FACE];
    i32 vbo_idx = 0, ebo_idx = 0;
    for (i32 face_num = 0; face_num < FACES_PER_BLOCK; face_num++) {
        for (i32 i = 0; i < VERTICES_PER_FACE; i++) {
            vbo_buffer[vbo_idx++] = dx[faces[face_num][i]];
            vbo_buffer[vbo_idx++] = dy[faces[face_num][i]];
            vbo_buffer[vbo_idx++] = dz[faces[face_num][i]];
            vbo_buffer[vbo_idx++] = tx[i];
            vbo_buffer[vbo_idx++] = ty[i];
        }
        for (i32 i = 0; i < INDICES_PER_FACE; i++)
            ebo_buffer[ebo_idx++] = face_num * 4 + winding[i];
    }
    vbo_bind(VBO_GAME);
    vbo_malloc(VBO_GAME, FACES_PER_BLOCK * VERTICES_PER_FACE * FLOATS_PER_VERTEX * sizeof(f32), GL_STATIC_DRAW);
    vbo_update(VBO_GAME, 0, FACES_PER_BLOCK * VERTICES_PER_FACE * FLOATS_PER_VERTEX * sizeof(f32), vbo_buffer);
    ebo_bind(EBO_GAME);
    ebo_malloc(EBO_GAME, FACES_PER_BLOCK * INDICES_PER_FACE * sizeof(u32), GL_STATIC_DRAW);
    ebo_update(EBO_GAME, 0, FACES_PER_BLOCK * INDICES_PER_FACE * sizeof(u32), ebo_buffer);
}

void game_render(void)
{    
    shader_use(SHADER_GAME);

    vao_bind(VAO_GAME);
    ebo_bind(EBO_GAME);
    glDrawElementsInstanced(GL_TRIANGLES, FACES_PER_BLOCK * INDICES_PER_FACE, GL_UNSIGNED_INT, 0, NUM_BLOCKS);
}