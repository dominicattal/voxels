#include "object.h"

Object object_create(u32 id, i32 x, i32 y, i32 z)
{
    Object obj;
    obj.id = id;
    obj.position.x = x;
    obj.position.y = y;
    obj.position.z = z;
    return obj;
}
