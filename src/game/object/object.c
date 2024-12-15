#include "object.h"
#include <stdlib.h>

Object* object_create(i32 x, i32 y, i32 z)
{
    Object* obj = malloc(sizeof(Object));
    obj->position.x = x;
    obj->position.y = y;
    obj->position.z = z;
    return obj;
}

void object_destroy(Object* object)
{
    free(object);
}