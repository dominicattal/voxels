#include "component.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define NUM_COMPONENT_FUNCS 1

static void (*component_functions[NUM_COMPONENTS][NUM_COMPONENT_FUNCS])();

static void initialize_functions(void);
static void initialize_comp(Component* comp);

void comp_init(void)
{
    initialize_functions();
}

Component* comp_create(i16 x, i16 y, i16 w, i16 h, CompID id)
{
    Component* comp = malloc(sizeof(Component));
    comp->x = x, comp->y = y, comp->w = w, comp->h = h;
    comp->r = comp->g = comp->b = 0;
    comp->a = 255;
    comp->id = id;
    comp->num_children = 0;
    comp->children = malloc(0);
    comp->alignment = ALIGN_LEFT;
    comp->alignment_y = ALIGN_TOP;
    initialize_comp(comp);
    return comp;
}

void comp_attach(Component* parent, Component* child)
{
    assert(parent->num_children < MAX_NUM_CHILDREN);
    parent->children = realloc(parent->children, (parent->num_children + 1) * sizeof(Component*));
    parent->children[parent->num_children++] = child;
}

void comp_detach(Component* parent, Component* child)
{
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

void comp_set_text(Component* comp, const char* text)
{
    assert(comp->id == COMP_TEXTBOX);   
    u32 length;
    char* copied_text;
    free(comp->text);
    length = strlen(text);
    if (length == 0) {
        comp->text = NULL;
        return;
    }
    copied_text = malloc((length + 1) * sizeof(char));
    strncpy(copied_text, text, length + 1);
    comp->text = copied_text;
}

/* --------------------------------- */

#include "components/components.h"

#define COMP_FUNC_INIT  0
#define COMP_FUNC_HOVER 1
#define COMP_FUNC_CLICK 2

static void initialize_functions(void)
{
    for (i32 i = 0; i < NUM_COMPONENTS; i++) 
        for (i32 j = 0; j < NUM_COMPONENT_FUNCS; j++)
            component_functions[i][j] = NULL;
}

static void initialize_comp(Component* comp)
{
    if (component_functions[comp->id][COMP_FUNC_INIT] != NULL)
        component_functions[comp->id][COMP_FUNC_INIT](comp);
}