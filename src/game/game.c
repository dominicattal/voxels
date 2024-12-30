#include "game.h"
#include "block.h"
#include "chunk.h"
#include "../renderer/renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct {
    f64 dt;
    pthread_t thread_id;
    bool kill_thread;
    sem_t mutex;
    Chunk* chunk;
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
    block_init();
    game.chunk = chunk_create();

    game.dt = 0;
    game.kill_thread = FALSE;
    sem_init(&game.mutex, 0, 1);
    pthread_create(&game.thread_id, NULL, game_update, NULL);    
}

void game_destroy(void)
{
    game.kill_thread = TRUE;
    pthread_join(game.thread_id, NULL);
    sem_destroy(&game.mutex);
    free(game.chunk);
}

f64 game_dt(void)
{
    return game.dt;
}

void game_render(void)
{    
    shader_use(SHADER_GAME);
    vao_bind(VAO_GAME);
    ebo_bind(EBO_GAME);
    chunk_draw(game.chunk);
}