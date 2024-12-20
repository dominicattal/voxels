#include "loader.h"
#include "presets/presets.h"

static void (*preset_functions[NUM_PRESETS])();

void gui_loader_init(void)
{
    preset_functions[GUI_DEFAULT] = gui_preset_default;
}

void gui_load(GUIPreset preset, Component* root)
{
    preset_functions[preset](root);
}