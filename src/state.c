#include "state.h"
#include "util/util.h"
#include <stdio.h>
#include "game/game.h"
#include "renderer/renderer.h"
#include "window/window.h"
#include "gui/gui.h"
#include "font/font.h"
#include "audio/audio.h"
#include "camera/camera.h"
#include <glfw.h>

void state_init(void)
{
    font_init();
    window_init();
    renderer_init();
    camera_init();
    audio_init();
    gui_init();
    game_init();
}

static void process_input(f32 dt)
{
    i32 rotation_magnitude = 0;
    i32 tilt_magnitude = 0;
    i32 zoom_magnitude = 0;
    vec3 direction = vec3_create(0, 0, 0);

    if (window_key_pressed(GLFW_KEY_Q))
        rotation_magnitude++;
    if (window_key_pressed(GLFW_KEY_E))
        rotation_magnitude--;
    if (window_key_pressed(GLFW_KEY_W))
        direction.z++;
    if (window_key_pressed(GLFW_KEY_S))
        direction.z--;
    if (window_key_pressed(GLFW_KEY_A))
        direction.x--;
    if (window_key_pressed(GLFW_KEY_D))
        direction.x++;
    if (window_key_pressed(GLFW_KEY_Z))
        direction.y--;
    if (window_key_pressed(GLFW_KEY_X))
        direction.y++;
    if (window_key_pressed(GLFW_KEY_T))
        tilt_magnitude++;
    if (window_key_pressed(GLFW_KEY_Y))
        tilt_magnitude--;
    if (window_key_pressed(GLFW_KEY_O))
        zoom_magnitude++;
    if (window_key_pressed(GLFW_KEY_P))
        zoom_magnitude--;

    if (vec3_mag(direction) != 0)
        camera_move(direction, dt);
    if (rotation_magnitude != 0)
        camera_rotate(rotation_magnitude, dt);
    if (tilt_magnitude != 0)
        camera_tilt(tilt_magnitude, dt);
    if (zoom_magnitude != 0)
        camera_zoom(zoom_magnitude, dt);
}

void state_loop(void)
{
    f64 dt = 0, time;
    while (!window_closed()) 
    {
        time = get_time();
        process_input(dt);
        gui_update(dt);
        window_update();
        renderer_render();
        dt = get_time() - time;
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