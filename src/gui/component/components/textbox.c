#include "components.h"

void comp_textbox_init(Component* comp)
{
    comp_set_halign(comp, ALIGN_LEFT);
    comp_set_valign(comp, ALIGN_TOP);
    comp_set_is_text(comp, TRUE);
}

void comp_textbox_hover(Component* comp, bool status)
{
    if (comp_is_hovered(comp) && !status) {
        comp_set_hovered(comp, FALSE);
        comp_set_color(comp, 0, 255, 0, 255);
    } else if (!comp_is_hovered(comp) && status) {
        comp_set_hovered(comp, TRUE);
        comp_set_color(comp, 0, 255, 255, 255);
    }
}