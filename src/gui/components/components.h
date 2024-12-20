#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "../component.h"

void comp_default_init(Component* comp);
void comp_textbox_init(Component* comp);
void comp_textbox_hover(Component* comp, bool status);
void comp_textbox_click(Component* comp, i32 buttion, i32 action);
void comp_textbox_key(Component* comp, i32 key, i32 scancode, i32 action, i32 mods);
void comp_textbox_update(Component* comp, f64 dt);
void comp_debug_init(Component* comp);
void comp_debug_key(Component* comp, i32 key, i32 scancode, i32 action, i32 mods);
void comp_debug_update(Component* comp, f64 dt);

#endif