#ifndef GUI_H
#define GUI_H

#include "../util.h"
#include "component/component.h"
#include "font/font.h"

typedef struct {
    u32 vbo_length, vbo_max_length;
    f32* vbo_buffer;
    u32 ebo_length, ebo_max_length;
    u32* ebo_buffer;
    bool max_length_changed;
    Component* root;
} GUI;

extern GUI gui;

void gui_init(void);
void gui_update(void);
void gui_destroy(void);

#endif