#ifndef GUI_H
#define GUI_H

#include "../type.h"

void gui_init(void);
void gui_update(f64 dt);
void gui_destroy(void);

void gui_mouse_button_callback(i32 button, i32 action);
void gui_key_callback(i32 key, i32 scancode, i32 action, i32 mods);
void gui_cursor_callback(void);

typedef struct {
    u32 vbo_length, vbo_max_length;
    f32* vbo_buffer;
    u32 ebo_length, ebo_max_length;
    u32* ebo_buffer;
} GUIData;

GUIData gui_get_data(void);

#endif