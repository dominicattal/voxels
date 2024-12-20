#include "components.h"
#include "../../../window/window.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <glfw.h>

typedef struct {
    Component* ref;
    f32 timer;
} Data;

static Data* data_create(void)
{
    Data* data = malloc(sizeof(Data));
    data->ref = NULL;
    data->timer = 0;
    return data;
}

void comp_textbox_init(Component* comp)
{
    comp_set_halign(comp, ALIGN_LEFT);
    comp_set_valign(comp, ALIGN_TOP);
    comp_set_is_text(comp, TRUE);
    comp_set_font(comp, FONT_DEFAULT);
    comp_set_font_size(comp, 24);
    comp->data = data_create();
}

void comp_textbox_hover(Component* comp, bool status)
{
    if (comp_is_hovered(comp) && !status) {
        comp_set_hovered(comp, FALSE);
        comp_set_color(comp, 0, 255, 0, 255);
    } else if (!comp_is_hovered(comp) && status) {
        comp_set_hovered(comp, TRUE);
        comp_set_color(comp, 0, 255, 255, 255);
    }
}

void comp_textbox_click(Component* comp, i32 button, i32 action)
{
    Data* data = comp->data;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        comp_set_color(data->ref, rand() % 256, rand() % 256, rand() % 256, 255);
    }
}

void comp_textbox_set_reference(Component* comp, Component* ref)
{
    assert(comp_id(comp) == COMP_TEXTBOX);
    Data* data = comp->data;
    data->ref = ref;
}

void comp_textbox_key(Component* comp, i32 key, i32 scancode, i32 action, i32 mods)
{
    (void)(scancode);
    Data* data = comp->data;
    if (data->ref != NULL) return;
    if ((key == GLFW_KEY_ENTER || key == GLFW_KEY_TAB || (key >= 32 && key < 128)) && action != GLFW_RELEASE) {
        comp_insert_char(comp, window_get_char(key, mods), -1);
    } else if (key == GLFW_KEY_BACKSPACE && action != GLFW_RELEASE) {
        comp_delete_char(comp, -1);
    }
}

void comp_textbox_update(Component* comp, f64 dt)
{
    return;
    Data* data = comp->data;
    data->timer -= dt;
    if (data->timer < 0) {
        data->timer += 1;
        comp_set_color(comp, rand() % 256, rand() % 256, rand() % 256, 255);
    }
}