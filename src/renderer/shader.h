#ifndef SHADER_H
#define SHADER_H

#include "../util/util.h"

#define NUM_SHADERS 2

typedef enum {
    SHADER_GUI = 0,
    SHADER_GAME = 1
} Shader;

void shader_init(void);
void shader_use(Shader shader);
void shader_destroy(void);

u32  shader_get_uniform_location(Shader shader, const char* identifier);
void shader_bind_uniform_block(Shader shader, u32 index, const char* identifier);
void shader_uniform_matrix_4fv(Shader shader, const char* identifier, i32 count, const f32* value);

#endif