#ifndef RENDERER_H
#define RENDERER_H

#include <glad.h>
#include "../util.h"
#include "vao/vao.h"
#include "shader/shader.h"
#include "texture/texture.h"

typedef struct {
    VAO vaos[NUM_VAOS];
    Shader shaders[NUM_SHADERS];
    Texture texture;
} Renderer;

extern Renderer renderer;

void renderer_init(void);
void renderer_malloc(VAOIndex vao_index, u32 vbo_length, u32 ebo_length);
void renderer_update(VAOIndex vao_index, u32 vbo_offset, u32 vbo_length, f32* vbo_buffer, u32 ebo_offset, u32 ebo_length, u32* ebo_buffer);
void renderer_render(void);
void renderer_destroy(void);

#endif