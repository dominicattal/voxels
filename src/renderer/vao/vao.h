#ifndef VAO_H
#define VAO_H

#include "../../type.h"
#include "../vbo/vbo.h"
#include "../ebo/ebo.h"

typedef struct {
    u32 id;
} VAO;

VAO vao_create(void);
void vao_bind(VAO vao);
void vao_destroy(VAO vao);

#endif