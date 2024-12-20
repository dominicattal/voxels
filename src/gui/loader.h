#ifndef GUI_LOADER_H
#define GUI_LOADER_H

#include "../util/util.h"
#include "component.h"

#define NUM_PRESETS 1

typedef enum {
    GUI_DEFAULT = 0
} GUIPreset;

void gui_loader_init(void);
void gui_load(GUIPreset preset, Component* root);

#endif