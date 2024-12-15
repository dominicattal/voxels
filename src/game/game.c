#include "game.h"
#include "object/object.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_OBJECTS 100
#define FLOAT_PER_VERTEX 3
#define NUM_VERTICES 8
#define INDEXES_PER_FACE 6
#define NUM_FACES 6

typedef struct {
    GameData data;
    f64 dt;
    pthread_t thread_id;
    bool kill_thread;
    sem_t mutex;
    Object* objects[NUM_OBJECTS];
} Game;

static Game game;

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
    for (i32 i = 0; i < NUM_OBJECTS; i++)
        game.objects[i] = object_create(i/10*2, i%10*2, i/10*2);

    game.data.vbo_max_length = NUM_OBJECTS * FLOAT_PER_VERTEX * NUM_VERTICES;
    game.data.ebo_max_length = NUM_OBJECTS * INDEXES_PER_FACE * NUM_FACES;
    game.data.vbo_buffer = malloc(game.data.vbo_max_length * sizeof(f32));
    game.data.ebo_buffer = malloc(game.data.ebo_max_length * sizeof(u32));

    game.dt = 0;
    game.kill_thread = FALSE;
    sem_init(&game.mutex, 0, 1);
    pthread_create(&game.thread_id, NULL, game_update, NULL);
}

void game_destroy(void)
{
    for (i32 i = 0; i < NUM_OBJECTS; i++)
        object_destroy(game.objects[i]);
    free(game.data.vbo_buffer);
    free(game.data.ebo_buffer);
    game.kill_thread = TRUE;
    pthread_join(game.thread_id, NULL);
    sem_destroy(&game.mutex);
}

f64 game_dt(void)
{
    return game.dt;
}

#define A game.data.vbo_buffer[game.data.vbo_length++]
#define B game.data.ebo_buffer[game.data.ebo_length++]

void game_update_data(void)
{
    game.data.vbo_length = game.data.ebo_length = 0;
    
    static u8 dx[] = {0, 0, 0, 0, 1, 1, 1, 1};
    static u8 dy[] = {0, 0, 1, 1, 0, 0, 1, 1};
    static u8 dz[] = {0, 1, 0, 1, 0, 1, 0, 1};
    static u8 tx[] = {0, 1, 1, 0};
    static u8 ty[] = {0, 0, 1, 1};
    static u8 winding[] = {0, 1, 2, 0, 2, 3};
    static u8 sides[][4] = {
        {4, 5, 7, 6}, // +x
        {1, 0, 2, 3}, // -x
        {2, 6, 7, 3}, // +y
        {0, 1, 5, 4}, // -y
        {5, 1, 3, 7}, // +z
        {0, 4, 6, 2}  // -z
    };

    for (i32 i = 0; i < NUM_OBJECTS; i++) {
        u32 idx = game.data.vbo_length / FLOAT_PER_VERTEX;
        for (i32 j = 0; j < NUM_VERTICES; j++) {
            A = dx[j] + game.objects[i]->position.x;
            A = dy[j] + game.objects[i]->position.y;
            A = dz[j] + game.objects[i]->position.z;
        }
        for (i32 j = 0; j < NUM_FACES; j++)
            for (i32 k = 0; k < INDEXES_PER_FACE; k++)
                B = idx + sides[j][winding[k]];
    }
}

#undef A
#undef B

GameData game_get_data(void)
{
    sem_wait(&game.mutex);
    game_update_data();
    sem_post(&game.mutex);
    return game.data;
}