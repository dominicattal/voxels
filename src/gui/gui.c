#include "gui.h"
#include "../renderer/renderer.h"
#include "../window/window.h"
#include <stdio.h>
#include <stdlib.h>

GUI gui;
extern Window window;

static void update_components(Component* comp);
static void update_data(void);

void gui_init(void)
{
    gui.vbo_max_length = gui.ebo_max_length = 0;
    gui.vbo_length = gui.ebo_length = 0;
    gui.vbo_buffer = malloc(0);
    gui.ebo_buffer = malloc(0);

    gui.root = comp_create(0, 0, 100, 100, COMP_DEFAULT);

    comp_attach(gui.root, comp_create(100, 100, 100, 100, COMP_DEFAULT));
    comp_attach(gui.root, comp_create(200, 200, 100, 100, COMP_DEFAULT));
}

void gui_update(void)
{
    update_components(gui.root);
    update_data();
}

void gui_destroy(void)
{
    comp_destroy(gui.root);
    free(gui.vbo_buffer);
    free(gui.ebo_buffer);
}

/* ------------------------------ */

#define A gui.vbo_buffer[gui.vbo_length++]
#define B gui.ebo_buffer[gui.ebo_length++]

#define FLOAT_PER_VERTEX 4
#define NUM_VERTICES     4
#define NUM_FLOATS       NUM_VERTICES * FLOAT_PER_VERTEX
#define NUM_INDEXES      6

static void resize_gui_buffers(u32 num_components)
{
    if (gui.vbo_length + NUM_FLOATS * num_components >= gui.vbo_max_length) {
        gui.vbo_max_length += NUM_FLOATS * num_components;
        gui.ebo_max_length += NUM_INDEXES * num_components;
        gui.vbo_buffer = realloc(gui.vbo_buffer, gui.vbo_max_length * sizeof(f32));
        gui.ebo_buffer = realloc(gui.ebo_buffer, gui.ebo_max_length * sizeof(u32));
        gui.max_length_changed = TRUE;
    }
}

void update_components(Component* comp)
{
    for (int i = 0; i < comp->num_children; i++)
        update_components(comp->children[i]);
}

void update_data_helper(Component* comp)
{
    f32 x1, y1, x2, y2;
    u32 idx = gui.vbo_length / 4;
    resize_gui_buffers(1);
    x1 = (f32)(comp->x - window.resolution.x / 2) / window.resolution.x;
    y1 = (f32)(comp->y - window.resolution.y / 2) / window.resolution.y;
    x2 = x1 + (f32)comp->w / window.resolution.x;
    y2 = y1 + (f32)comp->h / window.resolution.y;
    A = x1, A = y1, A = 0.0, A = 0.0;
    A = x2, A = y1, A = 0.0, A = 0.0;
    A = x2, A = y2, A = 0.0, A = 0.0;
    A = x1, A = y2, A = 0.0, A = 0.0;
    B = idx, B = idx + 1, B = idx + 2, B = idx, B = idx + 2, B = idx + 3;
    for (int i = 0; i < comp->num_children; i++)
        update_data_helper(comp->children[i]);
}

void update_data(void)
{
    gui.vbo_length = gui.ebo_length = 0;
    update_data_helper(gui.root);
    if (gui.max_length_changed) {
        renderer_malloc(VAO_GUI, gui.vbo_max_length, gui.ebo_max_length);
        gui.max_length_changed = FALSE;
    }
    renderer_update(VAO_GUI, 0, gui.vbo_length, gui.vbo_buffer, 0, gui.ebo_length, gui.ebo_buffer);
}