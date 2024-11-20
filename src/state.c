#include "state.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stb_truetype.h>

void state_init(void)
{
    window_init();
    gui_init();
    renderer_init();
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
    glViewport(0, 0, window.width, window.height);
    gui_update();
}

void mouse_button_callback(GLFWwindow* handle, i32 button, i32 action)
{
    gui_mouse_button_callback(button, action);
}

void key_callback(GLFWwindow* handle, i32 key, i32 scancode, i32 action, i32 mods)
{
    gui_key_callback(key, scancode, action, mods);
}

void cursor_pos_callback(GLFWwindow* handle, double xpos, double ypos)
{
    glfwGetCursorPos(window.handle, &window.cursor.x, &window.cursor.y);
    gui_cursor_callback();
}