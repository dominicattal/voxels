#include "window.h"
#include <glfw.h>
#include <stdio.h>

#define DEFAULT_WINDOW_WIDTH  500
#define DEFAULT_WINDOW_HEIGHT 500

typedef struct {
    GLFWwindow* handle;
    struct {
        i32 x, y;
    } resolution;
    struct {
        GLFWcursor* handle;
    } cursor;
} Window;

static Window window;

static void error_callback();
extern void framebuffer_size_callback();
extern void mouse_button_callback();
extern void key_callback();
extern void cursor_pos_callback();

void window_init(void)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window.handle = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "gui", NULL, NULL);
    window.resolution.x = DEFAULT_WINDOW_WIDTH;
    window.resolution.y = DEFAULT_WINDOW_HEIGHT;
    //glfwSetWindowAspectRatio(window.handle, 16, 9);
    
    glfwMakeContextCurrent(window.handle);
    glfwSetFramebufferSizeCallback(window.handle, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window.handle, mouse_button_callback);
    glfwSetCursorPosCallback(window.handle, cursor_pos_callback);
    glfwSetKeyCallback(window.handle, key_callback);
    glfwSetInputMode(window.handle, GLFW_LOCK_KEY_MODS, GLFW_TRUE);
    glfwSetErrorCallback(error_callback);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glViewport(0, 0, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
}

void window_update(void)
{
    glfwPollEvents();
    glfwSwapBuffers(window.handle);
}

void window_close(void) { 
    glfwSetWindowShouldClose(window.handle, 1); 
}

bool window_closed(void) { 
    return glfwWindowShouldClose(window.handle);
}

void window_destroy(void) { 
    glfwTerminate(); 
}

bool window_mouse_button_pressed(GLenum button) { 
    return glfwGetMouseButton(window.handle, button) == GLFW_PRESS; 
}

bool window_key_pressed(GLenum key) { 
    return glfwGetKey(window.handle, key) == GLFW_PRESS; 
}

void error_callback(int x, const char *message) { 
    fprintf(stderr, "%d\n%s\n", x, message); 
}

void window_get_resolution(i32* xres, i32* yres) {
    *xres = window.resolution.x;
    *yres = window.resolution.y;
}

bool window_cursor_in_bbox(i32 x, i32 y, i32 w, i32 h) {
    i32 width, height;
    f64 cursor_x, cursor_y;
    glfwGetWindowSize(window.handle, &width, &height);
    glfwGetCursorPos(window.handle, &cursor_x, &cursor_y);
    return cursor_x >= x 
        && cursor_x <= x + w 
        && height - cursor_y >= y 
        && height - cursor_y <= y + h;
}

void window_pixel_to_screen_x(i32 x, f32* x1) {
    *x1 = 2.0f * x / window.resolution.x;
}
void window_pixel_to_screen_y(i32 y, f32* y1) {
    *y1 = 2.0f * y / window.resolution.y;
}
void window_pixel_to_screen_point(i32 x, i32 y, f32* x1, f32* y1) {
    *x1 = 2.0f * x / window.resolution.x;
    *y1 = 2.0f * y / window.resolution.y;
}
void window_pixel_to_screen_bbox(i32 x, i32 y, i32 w, i32 h, f32* x1, f32* y1, f32* x2, f32* y2) {
    *x1 = 2.0f * (x - window.resolution.x / 2) / window.resolution.x;
    *y1 = 2.0f * (y - window.resolution.y / 2) / window.resolution.y;
    *x2 = *x1 + 2.0f * w / window.resolution.x;
    *y2 = *y1 + 2.0f * h / window.resolution.y;
}