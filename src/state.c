#include "state.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stb_truetype.h>

void state_init(void)
{
    window_init();
    renderer_init();
    gui_init();
}

void state_loop(void)
{
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
    gui_destroy();
}

/* --------------------------------- */

void framebuffer_size_callback(GLFWwindow* handle, i32 width, i32 height)
{
    glfwGetWindowSize(window.handle, &window.width, &window.height);
    window.aspect_ratio = (f32)window.width / window.height;
    glViewport(0, 0, window.width, window.height);
    gui_update();
}