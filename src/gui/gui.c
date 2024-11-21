#include "gui.h"
#include "loader/loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

GUI gui;
extern Window window;

static void update_components(void);
static void update_data(void);
static int cursor_in_bounds();

void gui_init(void)
{
    comp_init();

    gui.vbo_max_length = gui.ebo_max_length = 0;
    gui.vbo_length = gui.ebo_length = 0;
    gui.vbo_buffer = malloc(0);
    gui.ebo_buffer = malloc(0);

    gui.root = comp_create(0, 0, window.resolution.x, window.resolution.y, COMP_DEFAULT);
    comp_set_color(gui.root, 0, 0, 0, 0);
    comp_set_hoverable(gui.root, FALSE);

    gui_load_main();
}

void gui_update(void)
{
    update_components();
    update_data();
}

void gui_destroy(void)
{
    font_destroy();
    comp_destroy(gui.root);
    free(gui.vbo_buffer);
    free(gui.ebo_buffer);
}

void gui_mouse_button_callback_helper(Component* comp, i32 button, i32 action)
{
    i32 x, y, w, h;
    comp_get_bbox(comp, &x, &y, &w, &h);

    if (comp_is_clickable(comp) && cursor_in_bounds(x, y, w, h))
        comp_click(comp, button, action);

    for (i32 i = 0; i < comp_num_children(comp); i++)
        gui_mouse_button_callback_helper(comp->children[i], button, action);
}

void gui_mouse_button_callback(i32 button, i32 action)
{
    gui_mouse_button_callback_helper(gui.root, button, action);
}

void gui_key_callback(i32 key, i32 scancode, i32 action, i32 mods) 
{

}

void gui_cursor_callback_helper(Component* comp)
{
    i32 x, y, w, h;
    comp_get_bbox(comp, &x, &y, &w, &h);

    if (comp_is_hoverable(comp))
        comp_hover(comp, cursor_in_bounds(x, y, w, h));

    for (i32 i = 0; i < comp_num_children(comp); i++)
        gui_cursor_callback_helper(comp->children[i]);
}

void gui_cursor_callback(void) 
{
    gui_cursor_callback_helper(gui.root);
}

/* ------------------------------ */

#define A gui.vbo_buffer[gui.vbo_length++]
#define B gui.ebo_buffer[gui.ebo_length++]

#define FLOAT_PER_VERTEX 9
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

static void update_components_helper(Component* comp)
{
    i32 num_children;
    comp_get_num_children(comp, &num_children);
    for (i32 i = 0; i < num_children; i++)
        update_components_helper(comp->children[i]);
}

static void update_components(void)
{
    return;
    update_components_helper(gui.root);
}

static void update_data_text(Component* comp)
{
    CompID id;
    comp_get_id(comp, &id);

    if (id != COMP_TEXTBOX)
        return;

    if (comp->text == NULL)
        return;

    i32 cx, cy, cw, ch;     // comp position and size
    f32 x1, y1, x2, y2;     // screen coordinates
    f32 u1, v1, u2, v2;     // bitmap coordinates
    i32 a1, b1, a2, b2;     // glyph bounding box
    i32 ox, oy, test_ox;    // glyph origin
    i32 x, y, w, h;         // pixel coordinates
    i32 ascent, descent;    // highest and lowest glyph offsets
    i32 line_gap;           // gap between lines
    i32 adv, lsb, kern;     // advance, left side bearing, kerning
    i32 left, right, mid;   // pointers for word
    u8  ha, va;             // horizontal and vertical alignment
    i32 num_spaces;         // count whitespace for horizontal alignment
    f32 dy;                 // change in y for vertical alignment
    i32 ebo_idx, vbo_idx;   // ebo index of current glyph, vbo index of first glyph
    i32 length;             // index in text, length of text
    char* text;             // text, equal to comp->text
    i32 font_size = 7;
    
    comp_get_position(comp, &cx, &cy);
    comp_get_size(comp, &cw, &ch);
    comp_get_align(comp, &ha, &va);

    font_info(FONT_DEFAULT, font_size, &ascent, &descent, &line_gap);
    text = comp->text;
    length = strlen(text);

    left = right = 0;
    ox = 0;
    oy = ascent;
    resize_gui_buffers(length);
    vbo_idx = gui.vbo_length;
    while (right < length) {

        while (right < length && text[right] == ' ')
            right++;

        left = right;
        test_ox = 0;
        num_spaces = 0;
        while (right < length && text[right] != '\n' && test_ox <= cw) {
            font_char_hmetrics(FONT_DEFAULT, font_size, text[right], &adv, &lsb);
            font_char_kern(FONT_DEFAULT, font_size, text[right], text[right+1], &kern);
            test_ox += adv + kern;
            num_spaces += text[right] == ' ';
            right++;
        }

        mid = right;
        if (test_ox > cw) {
            while (mid > left && text[mid-1] != ' ') {
                font_char_hmetrics(FONT_DEFAULT, font_size, text[mid-1], &adv, &lsb);
                font_char_kern(FONT_DEFAULT, font_size, text[mid-1], text[mid], &kern);
                test_ox -= adv + kern;
                mid--;
            }
            while (mid > left && text[mid-1] == ' ') {
                font_char_hmetrics(FONT_DEFAULT, font_size, text[mid-1], &adv, &lsb);
                font_char_kern(FONT_DEFAULT, font_size, text[mid-1], text[mid], &kern);
                test_ox -= adv + kern;
                num_spaces -= text[mid-1] == ' ';
                mid--;
            }
        }

        if (mid == left) {
            test_ox = cw;
            right--;
        }
        else {
            right = mid;
        }

        if (left == right)
            right++;
        
        ox = 0;
        if (ha == ALIGN_RIGHT)
            ox = cw - test_ox;
        else if (ha == ALIGN_CENTER)
            ox = (cw - test_ox) / 2;
        
        while (left < right) {
            font_char_hmetrics(FONT_DEFAULT, font_size, text[left], &adv, &lsb);
            font_char_bbox(FONT_DEFAULT, font_size, text[left], &a1, &b1, &a2, &b2);
            font_char_bmap(FONT_DEFAULT, font_size, text[left], &u1, &v1, &u2, &v2);
            font_char_kern(FONT_DEFAULT, font_size, text[left], text[left+1], &kern);

            x = ox + lsb;
            y = ch - oy - b2;
            w = a2 - a1;
            h = b2 - b1;

            x1 = 2.0f * (cx + x - window.resolution.x / 2) / window.resolution.x;
            y1 = 2.0f * (cy + y - window.resolution.y / 2) / window.resolution.y;
            x2 = x1 + 2.0f * w / window.resolution.x;
            y2 = y1 + 2.0f * h / window.resolution.y;

            ebo_idx = gui.vbo_length / FLOAT_PER_VERTEX;

            if (text[left] != ' ') {
                A = x1, A = y1, A = u1, A = v2, A = 1, A = 1, A = 0, A = 1, A = TEX_BITMAP;
                A = x1, A = y2, A = u1, A = v1, A = 1, A = 1, A = 0, A = 1, A = TEX_BITMAP;
                A = x2, A = y2, A = u2, A = v1, A = 1, A = 1, A = 0, A = 1, A = TEX_BITMAP;
                A = x2, A = y1, A = u2, A = v2, A = 1, A = 1, A = 0, A = 1, A = TEX_BITMAP;
                B = ebo_idx, B = ebo_idx + 1, B = ebo_idx + 2, 
                B = ebo_idx, B = ebo_idx + 2, B = ebo_idx + 3;
            }   

            ox += adv + kern;
            if (ha == ALIGN_JUSTIFY && text[left] == ' ')
                ox += (cw - test_ox) / num_spaces;

            left++;
        }

        oy += ascent - descent + line_gap;
    }

    if (va == ALIGN_TOP)
        return;

    oy -= ascent - descent + line_gap;
    dy = ((va == ALIGN_BOTTOM) + 1) * (f32)(ch - oy) / window.resolution.y;

    while (vbo_idx < gui.vbo_length) {
        gui.vbo_buffer[vbo_idx + 1] -= dy;
        vbo_idx += FLOAT_PER_VERTEX;
    }
}

static void update_data_helper(Component* comp)
{
    i32 cx, cy, cw, ch;
    u8  cr, cg, cb, ca;
    f32 x1, y1, x2, y2, r, g, b, a;
    u32 idx = gui.vbo_length / FLOAT_PER_VERTEX;
    comp_get_position(comp, &cx, &cy);
    comp_get_size(comp, &cw, &ch);
    comp_get_color(comp, &cr, &cg, &cb, &ca);

    resize_gui_buffers(1);
    x1 = 2.0f * (cx - window.resolution.x / 2) / window.resolution.x;
    y1 = 2.0f * (cy - window.resolution.y / 2) / window.resolution.y;
    x2 = x1 + 2.0f * cw / window.resolution.x;
    y2 = y1 + 2.0f * ch / window.resolution.y;
    r = cr / 255.0f, g = cg / 255.0f, b = cb / 255.0f, a = ca / 255.0f;

    A = x1, A = y1, A = 0.0, A = 1.0, A = r, A = g, A = b, A = a, A = TEX_COLOR;
    A = x1, A = y2, A = 0.0, A = 0.0, A = r, A = g, A = b, A = a, A = TEX_COLOR;
    A = x2, A = y2, A = 1.0, A = 0.0, A = r, A = g, A = b, A = a, A = TEX_COLOR;
    A = x2, A = y1, A = 1.0, A = 1.0, A = r, A = g, A = b, A = a, A = TEX_COLOR;
    B = idx, B = idx + 1, B = idx + 2, B = idx, B = idx + 2, B = idx + 3;

    if (comp_id(comp) == COMP_TEXTBOX)
        update_data_text(comp);

    for (i32 i = 0; i < comp_num_children(comp); i++)
        update_data_helper(comp->children[i]);
}

static void update_data(void)
{
    gui.vbo_length = gui.ebo_length = 0;
    update_data_helper(gui.root);
    if (gui.max_length_changed) {
        renderer_malloc(VAO_GUI, gui.vbo_max_length, gui.ebo_max_length);
        gui.max_length_changed = FALSE;
    }
    renderer_update(VAO_GUI, 0, gui.vbo_length, gui.vbo_buffer, 0, gui.ebo_length, gui.ebo_buffer);
}

static int cursor_in_bounds(i32 x, i32 y, i32 w, i32 h)
{
    return window.cursor.x >= x 
        && window.cursor.x <= x + w 
        && window.height - window.cursor.y >= y 
        && window.height - window.cursor.y <= y + h;
}