#ifndef SHADER_H
#define SHADER_H

#include "../../type.h"

typedef struct {
    u32 id;
} Shader;

#define NUM_SHADERS 1

typedef enum {
    SHADER_DEFAULT = 0
} ShaderID;

Shader shader_create(char* vs_path, char* fs_path);
void shader_bind_uniform_block(Shader shader, u32 index, char* identifier);
void shader_use(Shader shader);
void shader_destroy(Shader shader);

#endif