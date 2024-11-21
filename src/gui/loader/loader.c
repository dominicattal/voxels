#include "loader.h"

extern GUI gui;

void gui_load_main(void)
{
    Component* click_me = comp_create(50, 50, 100, 100, COMP_TEXTBOX);
    comp_set_color(click_me, 0, 255, 0, 150);
    comp_set_align(click_me, ALIGN_CENTER, ALIGN_CENTER);
    comp_set_clickable(click_me, TRUE);

    comp_set_text(click_me, "Click Me!");
    comp_set_hoverable(click_me, TRUE);
    comp_attach(gui.root, click_me);

    Component* random_color = comp_create(150, 150, 250, 250, COMP_TEXTBOX);
    comp_set_color(random_color, 255, 0, 255, 255);
    comp_set_text(random_color, "The quick brown fox jumped over the lazy dog. The quick brown fox jumped over the lazy dog.");
    comp_attach(gui.root, random_color);

    comp_textbox_set_reference(click_me, random_color);
}