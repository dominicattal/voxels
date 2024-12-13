#ifndef VAO_H
#define VAO_H

#include "../../type.h"
#include "../vbo/vbo.h"
#include "../ebo/ebo.h"

#define NUM_VAOS 2

typedef enum {
    VAO_GUI = 0,
    VAO_FONT = 1
} VAO;

void vao_init(void);
void vao_bind(VAO vao);
void vao_destroy(void);

#endif