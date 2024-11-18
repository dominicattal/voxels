#include "window.h"
#include <stdio.h>

#define DEFAULT_WINDOW_WIDTH  500
#define DEFAULT_WINDOW_HEIGHT 500

Window window;

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
    glfwGetWindowSize(window.handle, &window.width, &window.height);
    //glfwSetWindowAspectRatio(window.handle, 16, 9);
    window.aspect_ratio = (f32)window.width / window.height;
    
    glfwMakeContextCurrent(window.handle);
    glfwSetFramebufferSizeCallback(window.handle, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window.handle, mouse_button_callback);
    glfwSetCursorPosCallback(window.handle, cursor_pos_callback);
    glfwSetKeyCallback(window.handle, key_callback);
    glfwSetInputMode(window.handle, GLFW_LOCK_KEY_MODS, GLFW_TRUE);
    glfwSetErrorCallback(error_callback);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glViewport(0, 0, window.width, window.height);
}

void window_update(void)
{
    glfwPollEvents();
    glfwSwapBuffers(window.handle);
}

void window_close(void) { glfwSetWindowShouldClose(window.handle, 1); }
bool window_closed(void) { return glfwWindowShouldClose(window.handle); }
void window_destroy(void) { glfwTerminate(); };
bool window_mouse_button_pressed(GLenum button) { return glfwGetMouseButton(window.handle, button) == GLFW_PRESS; }
bool window_key_pressed(GLenum key) { return glfwGetKey(window.handle, key) == GLFW_PRESS; }

void error_callback(int x, const char *message) { fprintf(stderr, "%d\n%s\n", x, message); }
