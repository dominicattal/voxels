#include "window.h"
#include "../gui/gui.h"
#include "../camera/camera.h"
#include <glfw.h>
#include <stdio.h>
#include <stb_image.h>

#define DEFAULT_WINDOW_WIDTH  500
#define DEFAULT_WINDOW_HEIGHT 500

typedef struct {
    GLFWwindow* handle;
    i32 width, height;
    struct {
        i32 x, y;
    } resolution;
    struct {
        GLFWcursor* handle;
        f64 x, y;
    } cursor;
    f64 dt;
} Window;

static Window window;

static void error_callback();
static void framebuffer_size_callback();
static void mouse_button_callback();
static void key_callback();
static void cursor_pos_callback();
static void load_images(void);

void window_init(void)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window.handle = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "program", NULL, NULL);
    window.resolution.x = DEFAULT_WINDOW_WIDTH;
    window.resolution.y = DEFAULT_WINDOW_HEIGHT;
    glfwGetWindowSize(window.handle, &window.width, &window.height);
    glfwGetCursorPos(window.handle, &window.cursor.x, &window.cursor.y);
    glfwSetWindowAspectRatio(window.handle, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    
    glfwMakeContextCurrent(window.handle);
    glfwSetFramebufferSizeCallback(window.handle, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window.handle, mouse_button_callback);
    glfwSetCursorPosCallback(window.handle, cursor_pos_callback);
    glfwSetKeyCallback(window.handle, key_callback);
    glfwSetInputMode(window.handle, GLFW_LOCK_KEY_MODS, GLFW_TRUE);
    glfwSetErrorCallback(error_callback);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glViewport(0, 0, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);

    load_images();

    window.dt = 0;
}

void window_update(void)
{
    f64 start = get_time();
    glfwPollEvents();
    glfwSwapBuffers(window.handle);
    window.dt = get_time() - start;
}

void window_close(void) 
{ 
    glfwSetWindowShouldClose(window.handle, 1); 
}

bool window_closed(void) 
{ 
    return glfwWindowShouldClose(window.handle);
}

void window_destroy(void) 
{ 
    glfwTerminate(); 
}

bool window_mouse_button_pressed(GLenum button) 
{ 
    return glfwGetMouseButton(window.handle, button) == GLFW_PRESS; 
}

bool window_key_pressed(GLenum key) 
{ 
    return glfwGetKey(window.handle, key) == GLFW_PRESS; 
}

void error_callback(int x, const char *message) 
{ 
    fprintf(stderr, "%d\n%s\n", x, message); 
}

void window_get_resolution(i32* xres, i32* yres) 
{
    *xres = window.resolution.x;
    *yres = window.resolution.y;
}

bool window_cursor_in_bbox(i32 x, i32 y, i32 w, i32 h) 
{
    f32 cursor_x = window.cursor.x * (f32)window.resolution.x  / window.width;
    f32 cursor_y = (window.height - window.cursor.y) * (f32)window.resolution.y / window.height;
    return cursor_x >= x 
        && cursor_x <= x + w 
        && cursor_y >= y 
        && cursor_y <= y + h;
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

static void framebuffer_size_callback(GLFWwindow* handle, i32 width, i32 height)
{
    window.width = width;
    window.height = height;
    glViewport(0, 0, window.width, window.height);
}

static void mouse_button_callback(GLFWwindow* handle, i32 button, i32 action)
{
    gui_mouse_button_callback(button, action);
}

static void key_callback(GLFWwindow* handle, i32 key, i32 scancode, i32 action, i32 mods)
{
    gui_key_callback(key, scancode, action, mods);
}

static void cursor_pos_callback(GLFWwindow* handle, f64 xpos, f64 ypos)
{
    window.cursor.x = xpos;
    window.cursor.y = ypos;
    gui_cursor_callback();
}

#define _KEY_CASE_SHIFT(_c, _sc) \
    case _c : c = (shift) ? _sc : _c; break;

#define _KEY_CASE_GLFW(_c, _gc) \
    case _gc : c = _c; break;

#define _KEY_CASE(_c) \
    case _c : c = _c; break;

char window_get_char(i32 key, i32 mods)
{
    static char num_to_sym[] = { ')', '!', '@', '#', '$', '%', '^', '&', '*', '(' };
    char c = '\0';
    bool shift, caps;
    shift = mods & GLFW_MOD_SHIFT;
    caps = (mods & GLFW_MOD_CAPS_LOCK) >> 4;
    if (key >= 'A' && key <= 'Z')
        c = key + ((shift ^ caps) ? 0 : 'a' - 'A');
    else if (key >= '0' && key <= '9')
        c = (shift) ? num_to_sym[key - '0'] : key;
    switch (key) {
        _KEY_CASE(' ')
        _KEY_CASE_GLFW('\t', GLFW_KEY_TAB)
        _KEY_CASE_GLFW('\n', GLFW_KEY_ENTER)
        _KEY_CASE_SHIFT('`', '~')
        _KEY_CASE_SHIFT('-', '_')
        _KEY_CASE_SHIFT('=', '+')
        _KEY_CASE_SHIFT('[', '{')
        _KEY_CASE_SHIFT(']', '}')
        _KEY_CASE_SHIFT('\\', '|')
        _KEY_CASE_SHIFT(';', ':')
        _KEY_CASE_SHIFT('\'', '\"')
        _KEY_CASE_SHIFT(',', '<')
        _KEY_CASE_SHIFT('.', '>')
        _KEY_CASE_SHIFT('/', '?')
    }
    return c;
}

f64 window_dt(void)
{
    return window.dt;
}

f32  window_aspect_ratio(void)
{
    return (f32)window.resolution.y / window.resolution.x;
}

static void load_images(void)
{
    GLFWimage images[2], cursor_image;

    images[0].pixels = stbi_load("assets/textures/my_icon.png", &images[0].width, &images[0].height, 0, 4);
    images[1].pixels = stbi_load("assets/textures/my_icon_small.png", &images[1].width, &images[1].height, 0, 4);
    glfwSetWindowIcon(window.handle, 2, images);

    cursor_image.pixels = stbi_load("assets/textures/cursor.png", &cursor_image.width, &cursor_image.height, 0, 4);
    window.cursor.handle = glfwCreateCursor(&cursor_image, 8, 8);
    glfwSetCursor(window.handle, window.cursor.handle);

    stbi_image_free(images[0].pixels);
    stbi_image_free(images[1].pixels);
    stbi_image_free(cursor_image.pixels);
}