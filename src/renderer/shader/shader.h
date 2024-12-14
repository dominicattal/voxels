#ifndef SHADER_H
#define SHADER_H

#include "../../type.h"

#define NUM_SHADERS 1

typedef enum {
    SHADER_GUI = 0
} Shader;

void shader_init(void);
void shader_use(Shader shader);
void shader_destroy(void);

#endif