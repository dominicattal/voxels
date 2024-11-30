#include "game.h"
#include "../util.h"
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct {
    f64 dt, last;
} Game;

static Game game;

static pthread_t thread_id;
static bool kill_thread;
static sem_t mutex; 

static void* game_update(void* vargp)
{
    while (!kill_thread) {
        f64 time = get_time();
        game.dt = time - game.last;
        game.last = time;
    }
}

void game_init(void)
{
    game.dt = 0;
    kill_thread = FALSE;
    sem_init(&mutex, 0, 1);
    pthread_create(&thread_id, NULL, game_update, NULL);
}

void game_destroy(void)
{
    kill_thread = TRUE;
    pthread_join(thread_id, NULL);
    sem_destroy(&mutex);
}