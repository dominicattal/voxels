#ifndef GUI_H
#define GUI_H

#include "../util/util.h"

void gui_init(void);
void gui_update(f64 dt);
void gui_destroy(void);

void gui_mouse_button_callback(i32 button, i32 action);
void gui_key_callback(i32 key, i32 scancode, i32 action, i32 mods);
void gui_cursor_callback(void);

void gui_prepare_render(void);
void gui_render(void);

#endif