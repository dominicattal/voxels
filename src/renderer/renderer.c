#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>

Renderer renderer;

static void message_callback();
static void link_shader_ssbo();
static void set_texture_ssbo();

void renderer_init(void)
{
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(message_callback, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    renderer.shaders[SHADER_DEFAULT] = shader_create("src/renderer/shaders/default/default.vert", "src/renderer/shaders/default/default.frag");

    renderer.ssbos[SSBO_TEXTURES] = ssbo_create(NUM_TEXTURES * sizeof(u64));
    link_shader_ssbo(SHADER_DEFAULT, SSBO_TEXTURES);
    set_texture_ssbo();

    renderer.vaos[VAO_GUI] = vao_create(GL_STATIC_DRAW, GL_TRIANGLES, 9);

    vao_attr(renderer.vaos[VAO_GUI], 0, 2, 0);
    vao_attr(renderer.vaos[VAO_GUI], 1, 2, 2);
    vao_attr(renderer.vaos[VAO_GUI], 2, 4, 4);
    vao_attr(renderer.vaos[VAO_GUI], 3, 1, 8);
}

void renderer_malloc(VAOID vao_index, u32 vbo_length, u32 ebo_length)
{
    vao_malloc(renderer.vaos[vao_index], vbo_length, ebo_length);
}

void renderer_update(VAOID vao_index, u32 vbo_offset, u32 vbo_length, f32* vbo_buffer, u32 ebo_offset, u32 ebo_length, u32* ebo_buffer)
{
    vao_update(renderer.vaos[vao_index], vbo_offset, vbo_length, vbo_buffer, ebo_offset, ebo_length, ebo_buffer);
}

void renderer_render(void)
{
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_use(renderer.shaders[SHADER_DEFAULT]);
    vao_draw(renderer.vaos[VAO_GUI]);
}

void renderer_destroy(void)
{
    i32 i;
    for (i = 0; i < NUM_SHADERS; i++)
        shader_destroy(renderer.shaders[i]);
    for (i = 0; i < NUM_SSBOS; i++)
        ssbo_destroy(renderer.ssbos[i]);
    for (i = 0; i < NUM_VAOS; i++)
        vao_destroy(renderer.vaos[i]);
}

/* --------------------------------- */

void GLAPIENTRY message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    printf("\nGL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ), type, severity, message);
    exit(1);
}

void link_shader_ssbo(ShaderID shader_id, SSBOID ssbo_id)
{
    shader_use(renderer.shaders[shader_id]);
    ssbo_bind_buffer_base(renderer.ssbos[ssbo_id], ssbo_id);
}

static void set_texture_ssbo()
{
    u64 handles[NUM_TEXTURES];
    for (i32 i = 0; i < NUM_TEXTURES; i++)
        handles[i] = renderer.textures[i].handle;
    ssbo_update(renderer.ssbos[SSBO_TEXTURES], 0, NUM_TEXTURES * sizeof(u64), handles);
}