#ifndef COMPONENT_H
#define COMPONENT_H

#include "../../util.h"

#define COMP_TYPE_ELEMENT 0
#define COMP_TYPE_TEXT    1
#define MAX_NUM_CHILDREN  1000

typedef struct Component Component;

typedef struct Component {
    u8 r, g, b, a;
    i16 x, y, w, h;
    u8 type, id;
    u16 num_children;
    union {
        char* text;
        Component** children;
    };
} Component;

typedef enum CompEnum {
    COMP_DEFAULT = 0
} CompEnum;

Component* comp_create(i16 x, i16 y, i16 w, i16 h, CompEnum id);
void comp_attach(Component* parent, Component* child);
void comp_detach(Component* parent, Component* child);
void comp_destroy(Component* comp);
void comp_detach_and_destroy(Component* parent, Component* child);

#endif