#include "component.h"
#include <assert.h>
#include <stdlib.h>

Component* comp_create(i16 x, i16 y, i16 w, i16 h, CompEnum id)
{
    Component* comp = malloc(sizeof(Component));
    comp->x = x, comp->y = y, comp->w = w, comp->h = h;
    comp->r = comp->g = comp->b = comp->a = 0;
    comp->type = COMP_TYPE_ELEMENT;
    comp->id = id;
    comp->num_children = 0;
    comp->children = malloc(0);
    return comp;
}

void comp_attach(Component* parent, Component* child)
{
    assert(parent->type == COMP_TYPE_ELEMENT);
    assert(parent->num_children < MAX_NUM_CHILDREN);
    parent->children = realloc(parent->children, (parent->num_children + 1) * sizeof(Component*));
    parent->children[parent->num_children++] = child;
}

void comp_detach(Component* parent, Component* child)
{
    assert(parent->type == COMP_TYPE_ELEMENT);
    for (i32 i = 0; i < parent->num_children; i++) {
        if (parent->children[i] == child) {
            parent->children[i] = parent->children[--parent->num_children];
            parent->children = realloc(parent->children, parent->num_children * sizeof(Component*));
        }
    }
}

void comp_destroy(Component* comp)
{
    for (int i = 0; i < comp->num_children; i++)
        comp_destroy(comp->children[i]);
    free(comp->children);
    free(comp);
}

void comp_detach_and_destroy(Component* parent, Component* child)
{
    comp_detach(parent, child);
    comp_destroy(child);
}

