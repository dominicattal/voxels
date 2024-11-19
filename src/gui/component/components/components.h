#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "../component.h"

void comp_default_init(Component* comp);

void comp_textbox_init(Component* comp);
void comp_textbox_hover(Component* comp, bool status);

#endif