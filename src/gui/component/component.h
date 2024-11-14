#ifndef COMPONENT_H
#define COMPONENT_H

#include "../../util.h"

#define MAX_NUM_CHILDREN  1000

typedef struct Component Component;

typedef enum CompID {
    COMP_DEFAULT = 0,
    COMP_TEXTBOX = 1
} CompID;

typedef struct Component {
    u8 id;
    u8 r, g, b, a;
    i16 x, y, w, h;
    u16 num_children;
    union {
        char* text;
        Component** children;
    };
} Component;

Component* comp_create(i16 x, i16 y, i16 w, i16 h, CompID id);
void comp_attach(Component* parent, Component* child);
void comp_detach(Component* parent, Component* child);
void comp_destroy(Component* comp);
void comp_detach_and_destroy(Component* parent, Component* child);

#endif