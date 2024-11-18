#ifndef WINDOW_H
#define WINDOW_H

#include <glad.h>
#include <glfw.h>
#include "../util.h"

typedef struct {
    GLFWwindow* handle;
    GLFWcursor* cursor;
    i32 width, height;
    f32 aspect_ratio;
    struct {
        i32 x, y;
    } resolution;
} Window;

extern Window window;

void window_init(void);
void window_update(void);
void window_close(void);
bool window_closed(void);
bool window_mouse_button_pressed(GLenum button);
bool window_key_pressed(GLenum key);
void window_destroy(void);

#endif