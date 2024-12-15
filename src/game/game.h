#ifndef GAME_H
#define GAME_H

#include "../util/util.h"

void game_init(void);
void game_destroy(void);
f64  game_dt(void);

typedef struct {
    u32 vbo_length, vbo_max_length;
    f32* vbo_buffer;
    u32 ebo_length, ebo_max_length;
    u32* ebo_buffer;
} GameData;

GameData game_get_data(void);

#endif