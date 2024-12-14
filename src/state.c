#include "state.h"
#include "util.h"
#include <stdio.h>
#include "game/game.h"
#include "renderer/renderer.h"
#include "window/window.h"
#include "gui/gui.h"
#include "font/font.h"
#include "audio/audio.h"

void state_init(void)
{
    window_init();
    font_init();
    renderer_init();
    audio_init();
    gui_init();
    game_init();
}

void state_loop(void)
{
    audio_play_sound(AUD_DEFAULT);
    while (!window_closed()) 
    {
        gui_update(window_dt());
        window_update();
        renderer_render();
    }
}

void state_destroy(void)
{
    window_destroy();
    font_destroy();
    renderer_destroy();
    audio_destroy();
    gui_destroy();
    game_destroy();
}