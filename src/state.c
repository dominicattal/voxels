#include "state.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stb_truetype.h>

void state_init(void)
{
    window_init();
    font_init();
    renderer_init();
    audio_init();
    gui_init();
}

void state_loop(void)
{
    audio_play_sound(AUD_DEFAULT);
    audio_play_sound(AUD_TEST);
    audio_play_sound(AUD_HIT);
    while (!window_closed()) 
    {
        gui_update();
        window_update();
        renderer_render();
    }
}

void state_destroy(void)
{
    window_destroy();
    renderer_destroy();
    audio_destroy();
    gui_destroy();
}