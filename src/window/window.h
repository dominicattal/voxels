#ifndef WINDOW_H
#define WINDOW_H

#include <glad.h>
#include "../util/util.h"

void window_init(void);
void window_update(void);
void window_close(void);
bool window_closed(void);
bool window_mouse_button_pressed(GLenum button);
bool window_key_pressed(GLenum key);
void window_destroy(void);

bool window_cursor_in_bbox(i32 x, i32 y, i32 w, i32 h);

// convert pixel coordinates to screen coordinates
// range x: [0..resolution.x] -> [-1..1]
// range y: [0..resolution.y] -> [-1..1]
void window_pixel_to_screen_x(i32 x, f32* x1);
void window_pixel_to_screen_y(i32 y, f32* y1);
void window_pixel_to_screen_point(i32 x, i32 y, f32* x1, f32* y1);
void window_pixel_to_screen_bbox(i32 x, i32 y, i32 w, i32 h, f32* x1, f32* y1, f32* x2, f32* y2);

void window_get_resolution(i32* xres, i32* yres);

char window_get_char(i32 key, i32 mods);

f64  window_dt(void);
f32  window_aspect_ratio(void);

#endif