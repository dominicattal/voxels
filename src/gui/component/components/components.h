#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "../component.h"

void comp_default_init(Component* comp);

void comp_textbox_init(Component* comp);
void comp_textbox_hover(Component* comp, bool status);
void comp_textbox_click(Component* comp, i32 buttion, i32 action);

#endif