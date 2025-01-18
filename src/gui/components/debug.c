#include "components.h"
#include "../../camera/camera.h"
#include "../../renderer/renderer.h"
#include "../../window/window.h"
#include "../../game/game.h"
#include <glfw.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    f32 timer;
} Data;

static Data* data_create(void)
{
    Data* data = malloc(sizeof(Data));
    data->timer = 0;
    return data;
}


static void toggle_visibility(Component* comp)
{
    comp_set_visible(comp, 1 - comp_is_visible(comp));
}

void comp_debug_init(Component* comp)
{
    comp_set_is_text(comp, TRUE);
    comp_set_font(comp, FONT_MONOSPACE);
    comp_set_halign(comp, ALIGN_LEFT);
    comp_set_valign(comp, ALIGN_TOP);
    comp_set_font_size(comp, 16);
    comp_set_color(comp, 255, 255, 255, 0);
    comp->data = data_create();
}

void comp_debug_key(Component* comp, i32 key, i32 scancode, i32 action, i32 mods)
{
    if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
        toggle_visibility(comp);
    }
}

void comp_debug_update(Component* comp, f64 dt)
{
    Data* data = comp->data;
    data->timer -= dt;
    if (data->timer > 0) return;
    
    char str[150];
    vec3 position = camera_position();
    f32 pitch = camera_pitch();
    f32 yaw = camera_yaw();
    sprintf(str, "Window: %5.3f\nRender: %5.3f\nGame:   %5.3f\npitch: %5.3f\nyaw: %5.3f\nx: %5.3f\ny: %5.3f\nz: %5.3f\n", 
            1000 * window_dt(), 1000 * renderer_dt(), 1000 * game_dt(),
            pitch, yaw, position.x, position.y, position.z);
    comp_set_text(comp, str);
    data->timer += 0.2;
}