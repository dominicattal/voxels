#include "gui.h"
#include "../renderer/renderer.h"
#include "../window/window.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

GUI gui;
extern Window window;
extern Font font;

static void update_components(void);
static void update_data(void);

void gui_init(void)
{
    font_init();

    gui.vbo_max_length = gui.ebo_max_length = 0;
    gui.vbo_length = gui.ebo_length = 0;
    gui.vbo_buffer = malloc(0);
    gui.ebo_buffer = malloc(0);

    gui.root = comp_create(0, 0, window.resolution.x, window.resolution.y, COMP_DEFAULT);
    gui.root->r = 0xFF;
    gui.root->b = 0xF0;
    gui.root->a = 50;

    Component* text_box = comp_create(50, 50, 300, 300, COMP_TEXTBOX);
    char* text = "The quick brown fox jumped over the lazy dog. THE QUICK BROWN FOX JUMPED OVER THE LAZY DOG";
    text_box->text = malloc((strlen(text) + 1) * sizeof(char));
    strncpy(text_box->text, text, strlen(text) + 1);
    text_box->g = 255;
    text_box->a = 30;
    text_box->alignment = ALIGN_LEFT;
    text_box->alignment_y = ALIGN_TOP;
    comp_attach(gui.root, text_box);
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
    for (i32 i = 0; i < comp->num_children; i++)
        update_components_helper(comp->children[i]);
}

static void update_components(void)
{
    update_components_helper(gui.root);
}

static void update_data_text(Component* comp)
{
    if (comp->id != COMP_TEXTBOX)
        return;

    f32 x1, y1, x2, y2;     // screen coordinates
    f32 u1, v1, u2, v2;     // bitmap coordinates
    i32 a1, b1, a2, b2;     // glyph bounding box
    i32 ox, oy, test_ox;    // glyph origin
    i32 x, y, w, h;         // pixel coordinates
    f32 scale;              // pixel scaling for font size
    i32 ascent, descent;    // highest and lowest glyph offsets
    i32 line_gap;           // gap between lines
    i32 adv, lsb, kern;     // advance, left side bearing, kerning
    i32 left, right, mid;   // pointers for word
    i32 num_spaces;         // count whitespace for horizontal alignment
    f32 dy;                 // change in y for vertical alignment
    i32 ebo_idx, vbo_idx;   // ebo index of current glyph, vbo index of first glyph
    i32 length;             // index in text, length of text
    char* text;             // text, equal to comp->text
    stbtt_packedchar b;     // information for each char
    
    scale = stbtt_ScaleForPixelHeight(&font.info, 32);
    stbtt_GetFontVMetrics(&font.info, &ascent, &descent, &line_gap);
    ascent = roundf(ascent * scale);
    descent = roundf(descent * scale);
    line_gap = roundf(line_gap * scale);
    text = comp->text;
    length = strlen(text);

    left = right = 0;
    ox = 0;
    oy = ascent;
    resize_gui_buffers(length);
    vbo_idx = gui.vbo_length;
    printf("%d, %d, %d\n", ascent, descent, line_gap);

    while (right < length) {

        while (right < length && text[right] == ' ')
            right++;

        left = right;
        test_ox = 0;
        num_spaces = 0;
        while (right < length && text[right] != '\n' && test_ox <= comp->w) {
            stbtt_GetCodepointHMetrics(&font.info, text[right], &adv, &lsb);
            kern = stbtt_GetCodepointKernAdvance(&font.info, text[right], text[right+1]);
            test_ox += roundf((adv + kern) * scale);
            num_spaces += text[right] == ' ';
            right++;
        }

        mid = right;
        if (test_ox > comp->w) {
            while (mid > left && text[mid-1] != ' ') {
                stbtt_GetCodepointHMetrics(&font.info, text[mid-1], &adv, &lsb);
                kern = stbtt_GetCodepointKernAdvance(&font.info, text[mid-1], text[mid]);
                test_ox -= roundf((adv + kern) * scale);
                mid--;
            }
            while (mid > left && text[mid-1] == ' ') {
                stbtt_GetCodepointHMetrics(&font.info, text[mid-1], &adv, &lsb);
                kern = stbtt_GetCodepointKernAdvance(&font.info, text[mid-1], text[mid]);
                test_ox -= roundf((adv + kern) * scale);
                num_spaces -= text[mid-1] == ' ';
                mid--;
            }
        }

        if (mid == left) {
            test_ox = comp->w;
            right--;
        }
        else {
            right = mid;
        }

        if (left == right)
            right++;
        
        ox = 0;
        if (comp->alignment == ALIGN_RIGHT)
            ox = comp->w - test_ox;
        else if (comp->alignment == ALIGN_CENTER)
            ox = (comp->w - test_ox) / 2;
        
        while (left < right) {
            stbtt_GetCodepointHMetrics(&font.info, text[left], &adv, &lsb);
            stbtt_GetCodepointBitmapBox(&font.info, text[left], scale, scale, &a1, &b1, &a2, &b2);
            kern = stbtt_GetCodepointKernAdvance(&font.info, text[left], text[left+1]);

            x = ox + a1 + lsb * scale;
            y = comp->h - oy - b2;
            w = a2 - a1;
            h = b2 - b1;

            x1 = 2.0f * (comp->x + x - window.resolution.x / 2) / window.resolution.x;
            y1 = 2.0f * (comp->y + y - window.resolution.y / 2) / window.resolution.y;
            x2 = x1 + 2.0f * w / window.resolution.x;
            y2 = y1 + 2.0f * h / window.resolution.y;

            b = font.packedChars[text[left]-CHAR_OFFSET];
            u1 = (f32)b.x0 / BITMAP_WIDTH;
            v1 = (f32)b.y0 / BITMAP_HEIGHT;
            u2 = (f32)b.x1 / BITMAP_WIDTH;
            v2 = (f32)b.y1 / BITMAP_HEIGHT;

            ebo_idx = gui.vbo_length / FLOAT_PER_VERTEX;

            if (text[left] != ' ') {
                A = x1, A = y1, A = u1, A = v2, A = 0, A = 1, A = 0, A = 1, A = 1;
                A = x1, A = y2, A = u1, A = v1, A = 0, A = 1, A = 0, A = 1, A = 1;
                A = x2, A = y2, A = u2, A = v1, A = 0, A = 1, A = 0, A = 1, A = 1;
                A = x2, A = y1, A = u2, A = v2, A = 0, A = 1, A = 0, A = 1, A = 1;
                B = ebo_idx, B = ebo_idx + 1, B = ebo_idx + 2, 
                B = ebo_idx, B = ebo_idx + 2, B = ebo_idx + 3;
            }   

            ox += roundf((adv + kern) * scale);
            if (comp->alignment == ALIGN_JUSTIFY && text[left] == ' ')
                ox += (comp->w - test_ox) / num_spaces;

            left++;
        }

        oy += ascent - descent + line_gap;
    }

    if (comp->alignment_y == ALIGN_TOP)
        return;

    oy -= ascent - descent + line_gap;
    dy = ((comp->alignment_y == ALIGN_BOTTOM) + 1) * (f32)(comp->h - oy) / window.resolution.y;

    while (vbo_idx < gui.vbo_length) {
        gui.vbo_buffer[vbo_idx + 1] -= dy;
        vbo_idx += FLOAT_PER_VERTEX;
    }
}

static void update_data_helper(Component* comp)
{
    f32 x1, y1, x2, y2, r, g, b, a;
    u32 idx = gui.vbo_length / FLOAT_PER_VERTEX;

    resize_gui_buffers(1);
    x1 = 2.0f * (f32)(comp->x - window.resolution.x / 2) / window.resolution.x;
    y1 = 2.0f * (f32)(comp->y - window.resolution.y / 2) / window.resolution.y;
    x2 = x1 + 2.0f * (f32)comp->w / window.resolution.x;
    y2 = y1 + 2.0f * (f32)comp->h / window.resolution.y;
    r = comp->r / 255.0f, g = comp->g / 255.0f, b = comp->b / 255.0f, a = comp->a / 255.0f;

    A = x1, A = y1, A = 0.0, A = 1.0, A = r, A = g, A = b, A = a, A = 0;
    A = x1, A = y2, A = 0.0, A = 0.0, A = r, A = g, A = b, A = a, A = 0;
    A = x2, A = y2, A = 1.0, A = 0.0, A = r, A = g, A = b, A = a, A = 0;
    A = x2, A = y1, A = 1.0, A = 1.0, A = r, A = g, A = b, A = a, A = 0;
    B = idx, B = idx + 1, B = idx + 2, B = idx, B = idx + 2, B = idx + 3;

    update_data_text(comp);
    for (i32 i = 0; i < comp->num_children; i++)
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