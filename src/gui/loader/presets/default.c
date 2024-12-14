#include "presets.h"

void gui_preset_default(Component* root)
{
    i32 width, height;
    comp_get_size(root, &width, &height);
    Component* debug = comp_create(0, height-75, 150, 75, COMP_DEBUG);
    comp_attach(root, debug);
}