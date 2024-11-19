#include "components.h"

void comp_default_init(Component* comp)
{
    comp_set_is_text(comp, FALSE);
    comp_set_hoverable(comp, TRUE);
}