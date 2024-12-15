#ifndef EBO_H
#define EBO_H

#include "../../util/util.h"

#define NUM_EBOS 3

typedef enum {
    EBO_GUI = 0,
    EBO_FONT = 1,
    EBO_GAME = 2
} EBO;

void ebo_init(void);
void ebo_bind(EBO ebo);
void ebo_destroy(void);

void ebo_malloc(EBO ebo, u32 length, GLenum usage);
void ebo_update(EBO ebo, u32 offset, u32 length, u32* buffer);
u32  ebo_length(EBO ebo);

#endif