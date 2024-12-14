#include "presets.h"

void gui_preset_default(Component* root)
{
    i32 width, height;
    comp_get_size(root, &width, &height);
    Component* debug = comp_create(0, height-75, 150, 75, COMP_DEBUG);
    comp_attach(root, debug);

    Component* click_me = comp_create(50, 50, 100, 100, COMP_TEXTBOX);
    comp_set_color(click_me, 0, 255, 0, 255);
    comp_set_align(click_me, ALIGN_CENTER, ALIGN_TOP);
    comp_set_clickable(click_me, TRUE);

    comp_set_text(click_me, "Click Me!");
    comp_set_hoverable(click_me, TRUE);
    comp_attach(root, click_me);

    Component* random_color = comp_create(150, 150, 250, 250, COMP_TEXTBOX);
    comp_set_color(random_color, 255, 0, 255, 255);
    comp_set_align(random_color, ALIGN_LEFT, ALIGN_TOP);
    comp_attach(root, random_color);

    comp_textbox_set_reference(click_me, random_color);
}