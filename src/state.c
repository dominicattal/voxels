#include "state.h"
#include <stdio.h>

void state_init(void)
{
    window_init();
    renderer_init();

    static float points[] = {
        0.0, 0.0,
        0.0, 1.0,
        1.0, 1.0,
        1.0, 0.0
    };

    static int idxs[] = { 0, 1, 2, 0, 2, 3 };

    renderer_malloc(VAO_DEFAULT, sizeof(points) / sizeof(float), sizeof(idxs) / sizeof(int));
    renderer_update(VAO_DEFAULT, 0, sizeof(points) / sizeof(float), points, 0, sizeof(idxs) / sizeof(int), idxs);
}

void state_loop(void)
{
    while (!window_closed()) 
    {
        window_update();
        renderer_render();
    }
}

void state_destroy(void)
{
    window_destroy();
    renderer_destroy();
}