#include "game.h"
#include "../util.h"
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct {
    f64 dt;
    pthread_t thread_id;
    bool kill_thread;
    sem_t mutex; 
} Game;

static Game game;

static void *game_update(void* vargp)
{
    f64 start;
    while (!game.kill_thread) {
        start = get_time();
        sleep(5);
        game.dt = get_time() - start;
    }
}

void game_init(void)
{
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
}

f64 game_dt(void)
{
    return game.dt;
}