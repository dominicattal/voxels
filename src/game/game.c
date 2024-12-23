#include "game.h"
#include "object/object.h"
#include "../renderer/renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_OBJECTS         100
#define FLOATS_PER_VERTEX   6
#define VERTICES_PER_FACE   4
#define INDICES_PER_FACE    6
#define FACES_PER_OBJECT    6

typedef struct {
    u32 vbo_length, vbo_max_length;
    f32* vbo_buffer;
    u32 ebo_length, ebo_max_length;
    u32* ebo_buffer;
} GameData;

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
        game.objects[i] = object_create(3 + (i % 4), i/10*2, i%10*2, i/10*2);

    game.data.vbo_max_length = NUM_OBJECTS * FACES_PER_OBJECT * VERTICES_PER_FACE * FLOATS_PER_VERTEX;
    game.data.ebo_max_length = NUM_OBJECTS * FACES_PER_OBJECT * INDICES_PER_FACE;
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
    static u8 faces[][4] = {
        {4, 5, 7, 6}, // +x
        {1, 0, 2, 3}, // -x
        {2, 6, 7, 3}, // +y
        {0, 1, 5, 4}, // -y
        {5, 1, 3, 7}, // +z
        {0, 4, 6, 2}  // -z
    };

    for (i32 obj_num = 0; obj_num < NUM_OBJECTS; obj_num++) {
        Object* obj = game.objects[obj_num];
        f32 u1, v1, u2, v2;
        u32 location;
        texture_get_info(obj->id, &location, &u1, &v1, &u2, &v2);
        f32 u[2] = {u1, u2};
        f32 v[2] = {v1, v2};
        for (i32 face_num = 0; face_num < FACES_PER_OBJECT; face_num++) {
            u32 idx = game.data.vbo_length / FLOATS_PER_VERTEX;
            for (i32 i = 0; i < VERTICES_PER_FACE; i++) {
                A = obj->position.x + dx[faces[face_num][i]];
                A = obj->position.y + dy[faces[face_num][i]];
                A = obj->position.z + dz[faces[face_num][i]];
                A = u[tx[i]];
                A = v[ty[i]];
                A = location;
            }
            for (i32 i = 0; i < INDICES_PER_FACE; i++)
                B = idx + winding[i];
        }
    }
}

#undef A
#undef B

void game_render(void)
{
    sem_wait(&game.mutex);
    game_update_data();
    sem_post(&game.mutex);

    vbo_malloc(VBO_GAME, game.data.vbo_max_length, GL_STATIC_DRAW);
    ebo_malloc(EBO_GAME, game.data.ebo_max_length, GL_STATIC_DRAW);
    vbo_update(VBO_GAME, 0, game.data.vbo_length, game.data.vbo_buffer);
    ebo_update(EBO_GAME, 0, game.data.ebo_length, game.data.ebo_buffer);
    glEnable(GL_DEPTH_TEST);
    shader_use(SHADER_GAME);

    vao_bind(VAO_GAME);
    vbo_bind(VBO_GAME);
    ebo_bind(EBO_GAME);
    glDrawElements(GL_TRIANGLES, ebo_length(EBO_GAME), GL_UNSIGNED_INT, 0);
}