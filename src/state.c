#include "state.h"
#include "renderer/renderer.h"
#include "window/window.h"
#include <stdio.h>

void state_init(void)
{
    window_init();
}

void state_loop(void)
{
    while (!window_closed()) 
    {
        window_update();
    }
}

void state_destroy(void)
{
    window_destroy();
}