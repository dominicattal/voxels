#include "renderer.h"
#include <glad.h>
#include <stdio.h>
#include <stdlib.h>
#include "../gui/gui.h"
#include "../game/game.h"

typedef struct {
    f64 dt;
} Renderer;

static Renderer renderer;

static void message_callback();

void renderer_init(void)
{
    renderer.dt = 0;

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(message_callback, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    shader_init();
    texture_init();
    vbo_init();
    ebo_init();
    vao_init();
}

void renderer_render(void)
{
    f64 start = get_time();

    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_DEPTH_TEST);
    game_render();
    glDisable(GL_DEPTH_TEST);
    gui_render();

    renderer.dt = get_time() - start;
}

void renderer_destroy(void)
{
    shader_destroy();
    vao_destroy();
    vbo_destroy();
    ebo_destroy();
    texture_destroy();
}

f64 renderer_dt(void)
{
    return renderer.dt;
}

void renderer_uniform_update_view(f32* mat)
{
    shader_uniform_matrix_4fv(SHADER_GAME, "view", 1, mat);
}

void renderer_uniform_update_proj(f32* mat)
{
    shader_uniform_matrix_4fv(SHADER_GAME, "proj", 1, mat);
}

void GLAPIENTRY message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    printf("\n%x, %x, %d, %p\n", source, id, length, userParam);
    printf("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ), type, severity, message);
    exit(1);
}
