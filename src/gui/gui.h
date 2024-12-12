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
    u32 comp_vbo_length, comp_vbo_max_length;
    f32* comp_vbo_buffer;
    u32 comp_ebo_length, comp_ebo_max_length;
    u32* comp_ebo_buffer;
    u32 font_vbo_length, font_vbo_max_length;
    f32* font_vbo_buffer;
    u32 font_ebo_length, font_ebo_max_length;
    u32* font_ebo_buffer;
} GUIData;

GUIData gui_get_data(void);

#endif