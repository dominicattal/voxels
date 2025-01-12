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
} Game;

static Game game;

static void* game_update(void* vargp)
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
    chunk_init();
    
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
    chunk_destroy();
    
}

f64 game_dt(void)
{
    return game.dt;
}

void game_prepare_render(void)
{
    chunk_prepare_render();
}

void game_render(void)
{    
    chunk_render();
}