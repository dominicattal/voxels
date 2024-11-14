#include "gui.h"
#include <stdio.h>
#include <stdlib.h>

GUI gui;

static void update_components(Component* comp);
static void update_data(Component* comp);

void gui_init(void)
{
    gui.vbo_max_length = gui.ebo_max_length = 0;
    gui.vbo_length = gui.ebo_length = 0;
    gui.vbo_buffer = malloc(0);
    gui.ebo_buffer = malloc(0);

    gui.root = comp_create(0, 0, 100, 100, COMP_DEFAULT);

    comp_attach(gui.root, comp_create(10, 10, 50, 50, COMP_DEFAULT));
    comp_attach(gui.root, comp_create(60, 60, 20, 20, COMP_DEFAULT));

    printf("%lld\n", sizeof(Component));
}

void gui_update(void)
{
    update_components(gui.root);
    update_data(gui.root);
    if (gui.max_length_changed) {
        renderer_malloc(VAO_GUI, gui.vbo_max_length, gui.ebo_max_length);
        gui.max_length_changed = FALSE;
    }
    renderer_update(VAO_GUI, 0, gui.vbo_length, gui.vbo_buffer, 0, gui.ebo_length, gui.ebo_buffer);
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

static void resize_gui_buffers(u32 num_components)
{
    if (gui.vbo_length + 4 * 4 * num_components >= gui.vbo_max_length) {
        gui.vbo_max_length += 4 * 4 * num_components;
        gui.ebo_max_length += 6 * num_components;
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

void update_data(Component* comp)
{
    u32 idx;
    f32 x1, y1, x2, y2;

    resize_gui_buffers(1);
    x1 = 0.2, y1 = 0.2, x2 = 0.3, y2 = 0.3;
    A = x1, A = y1, A = 0.0, A = 0.0;
    A = x2, A = y1, A = 0.0, A = 1.0;
    A = x2, A = y2, A = 1.0, A = 1.0;
    A = x1, A = y2, A = 1.0, A = 0.0;

    idx = gui.vbo_length / 4;
    B = idx, B = idx + 1, B = idx + 2, B = idx, B = idx + 2, B = idx + 3;
    
    for (int i = 0; i < comp->num_children; i++)
        update_data(comp->children[i]);
}