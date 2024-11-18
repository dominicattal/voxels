#ifndef COMPONENT_H
#define COMPONENT_H

#include "../../util.h"

#define MAX_NUM_CHILDREN  1000

#define ALIGN_LEFT    0
#define ALIGN_RIGHT   1
#define ALIGN_TOP     0
#define ALIGN_BOTTOM  1
#define ALIGN_CENTER  2
#define ALIGN_JUSTIFY 3

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
    u8 alignment;
    u8 alignment_y;
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