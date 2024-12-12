#ifndef EBO_H
#define EBO_H

#include "../../type.h"

typedef struct {
    u32 id, length;
} EBO;

EBO ebo_create(void);
void ebo_bind(EBO ebo);
void ebo_destroy(EBO ebo);

void ebo_malloc(EBO* ebo, u32 length, GLenum usage);
void ebo_update(EBO* ebo, u32 offset, u32 length, u32* buffer);

#endif