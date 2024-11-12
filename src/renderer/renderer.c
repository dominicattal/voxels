#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>

Renderer renderer;

static void message_callback();

void renderer_init(void)
{
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(message_callback, 0);

    renderer.shaders[SHADER_DEFAULT] = shader_create("src/renderer/shaders/default/default.vert", "src/renderer/shaders/default/default.frag");

    renderer.vaos[VAO_DEFAULT] = vao_create(GL_STATIC_DRAW, GL_TRIANGLES, 4);

    vao_attr(renderer.vaos[VAO_DEFAULT], 0, 2, 0);
    vao_attr(renderer.vaos[VAO_DEFAULT], 1, 2, 2);

    renderer.texture = texture_create("assets/test.png");
}

void renderer_malloc(VAOIndex vao_index, u32 vbo_length, u32 ebo_length)
{
    vao_malloc(renderer.vaos[vao_index], vbo_length, ebo_length);
}

void renderer_update(VAOIndex vao_index, u32 vbo_offset, u32 vbo_length, f32* vbo_buffer, u32 ebo_offset, u32 ebo_length, u32* ebo_buffer)
{
    vao_update(renderer.vaos[vao_index], vbo_offset, vbo_length, vbo_buffer, ebo_offset, ebo_length, ebo_buffer);
}

void renderer_render(void)
{
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    texture_bind(renderer.texture);
    shader_use(renderer.shaders[SHADER_DEFAULT]);
    vao_draw(renderer.vaos[VAO_DEFAULT]);
}

void renderer_destroy(void)
{
    for (int i = 0; i < NUM_SHADERS; i++)
        shader_destroy(renderer.shaders[i]);
    for (int i = 0; i < NUM_VAOS; i++)
        vao_destroy(renderer.vaos[i]);

    texture_destroy(renderer.texture);
}


void GLAPIENTRY message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    printf("\nGL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ), type, severity, message);
    exit(1);
}
