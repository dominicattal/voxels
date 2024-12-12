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

Shader shader_create(void);
u32  shader_compile(GLenum type, const char* path);
void shader_link(Shader shader);
void shader_bind_uniform_block(Shader shader, u32 index, const char* identifier);
void shader_use(Shader shader);
void shader_destroy(Shader shader);

#endif