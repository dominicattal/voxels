#ifndef SHADER_H
#define SHADER_H

#include "../../type.h"

#define NUM_SHADERS 1

typedef enum {
    SHADER_DEFAULT = 0
} ShaderID;

void shader_init(void);
u32  shader_compile(GLenum type, const char* path);
void shader_attach(ShaderID id, u32 shader);
void shader_detach(ShaderID id, u32 shader);
void shader_link(ShaderID id);
void shader_use(ShaderID id);
void shader_destroy(void);

#endif