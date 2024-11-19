#ifndef COMPONENT_H
#define COMPONENT_H

#include "../../util.h"

#define MAX_NUM_CHILDREN  1000

#define ALIGN_LEFT      0
#define ALIGN_RIGHT     1
#define ALIGN_CENTER    2
#define ALIGN_JUSTIFY   3
#define ALIGN_TOP       0
#define ALIGN_BOTTOM    1

#define HOVER_ON        0
#define HOVER_OFF       1

typedef struct Component Component;

typedef struct Component {
    u64 info1;
    u64 info2;
    union {
        char* text;
        Component** children;
    };
} Component;

#define NUM_COMPONENTS 3

typedef enum CompID {
    COMP_DEFAULT = 0,
    COMP_TEXTBOX = 1,
} CompID;

void comp_init(void);
Component* comp_create(i16 x, i16 y, i16 w, i16 h, CompID id);
void comp_attach(Component* parent, Component* child);
void comp_detach(Component* parent, Component* child);
void comp_destroy(Component* comp);
void comp_detach_and_destroy(Component* parent, Component* child);
void comp_set_text(Component* comp, const char* text);
void comp_hover(Component* comp, bool status);

/* Setters for packed info */
void comp_set_id(Component* comp, CompID id);
void comp_set_is_text(Component* comp, bool it);
void comp_set_r(Component* comp, u8 r);
void comp_set_g(Component* comp, u8 g);
void comp_set_b(Component* comp, u8 b);
void comp_set_a(Component* comp, u8 a);
void comp_set_x(Component* comp, i32 x);
void comp_set_y(Component* comp, i32 y);
void comp_set_w(Component* comp, i32 w);
void comp_set_h(Component* comp, i32 h);
void comp_set_num_children(Component* comp, i32 num_children);
void comp_set_halign(Component* comp, u8 ha);
void comp_set_valign(Component* comp, u8 va);
void comp_set_hoverable(Component* comp, bool hv);
void comp_set_hovered(Component* comp, bool hd);
void comp_set_color(Component* comp, u8 r, u8 g, u8 b, u8 a);
void comp_set_bbox(Component* comp, i32 x, i32 y, i32 w, i32 h);
void comp_set_position(Component* comp, i32 x, i32 y);
void comp_set_size(Component* comp, i32 w, i32 h);
void comp_set_align(Component* comp, u8 ha, u8 va);

/* Getters for packed info */
void comp_get_id(Component* comp, CompID* id);
void comp_get_is_text(Component* comp, bool* it);
void comp_get_r(Component* comp, u8* r);
void comp_get_g(Component* comp, u8* g);
void comp_get_b(Component* comp, u8* b);
void comp_get_a(Component* comp, u8* a);
void comp_get_x(Component* comp, i32* x);
void comp_get_y(Component* comp, i32* y);
void comp_get_w(Component* comp, i32* w);
void comp_get_h(Component* comp, i32* h);
void comp_get_num_children(Component* comp, i32* num_children);
void comp_get_halign(Component* comp, u8* ha);
void comp_get_valign(Component* comp, u8* va);
void comp_get_hoverable(Component* comp, bool* hv);
void comp_get_hovered(Component* comp, bool* hd);
void comp_get_color(Component* comp, u8* r, u8* g, u8* b, u8* a);
void comp_get_bbox(Component* comp, i32* x, i32* y, i32* w, i32* h);
void comp_get_position(Component* comp, i32* x, i32* y);
void comp_get_size(Component* comp, i32* w, i32* h);
void comp_get_align(Component* comp, u8* ha, u8* va);

/* Second set of getters */
CompID comp_id(Component* comp);
i32  comp_num_children(Component* comp);
bool comp_is_text(Component* comp);
bool comp_is_hoverable(Component* comp);
bool comp_is_hovered(Component* comp);

void print_bits(u64 x);

#endif