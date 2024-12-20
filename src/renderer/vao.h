#ifndef VAO_H
#define VAO_H

#include "../util/util.h"
#include "vbo.h"
#include "ebo.h"

#define NUM_VAOS 3

typedef enum {
    VAO_GUI = 0,
    VAO_FONT = 1,
    VAO_GAME = 2
} VAO;

void vao_init(void);
void vao_bind(VAO vao);
void vao_destroy(void);

#endif