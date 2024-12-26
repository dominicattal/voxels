#ifndef OBJECT_H
#define OBJECT_H

#include "../../util/util.h"

typedef struct {
    struct {
        i32 x, y, z;
    } position;
    u8 id;
} Object;

Object object_create(u32 id, i32 x, i32 y, i32 z);

#endif