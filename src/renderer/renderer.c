#include "renderer.h"
#include "../util.h"
#include "vao/vao.h"
#include "shader/shader.h"
#include "texture/texture.h"
#include <glad.h>
#include <stdio.h>
#include <stdlib.h>
#include "../gui/gui.h"


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

    texture_init();
    shader_init();
    vbo_init();
    ebo_init();
    vao_init();
}

void renderer_render(void)
{
    GUIData gui_data = gui_get_data();
    vbo_malloc(VBO_GUI,  gui_data.comp_vbo_max_length, GL_STATIC_DRAW);
    ebo_malloc(EBO_GUI,  gui_data.comp_ebo_max_length, GL_STATIC_DRAW);
    vbo_update(VBO_GUI,  0, gui_data.comp_vbo_length, gui_data.comp_vbo_buffer);
    ebo_update(EBO_GUI,  0, gui_data.comp_ebo_length, gui_data.comp_ebo_buffer);
    vbo_malloc(VBO_FONT, gui_data.font_vbo_max_length, GL_STATIC_DRAW);
    ebo_malloc(EBO_FONT, gui_data.font_ebo_max_length, GL_STATIC_DRAW);
    vbo_update(VBO_FONT, 0, gui_data.font_vbo_length, gui_data.font_vbo_buffer);
    ebo_update(EBO_FONT, 0, gui_data.font_ebo_length, gui_data.font_ebo_buffer);

    f64 start = get_time();

    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_use(SHADER_DEFAULT);

    vao_bind(VAO_GUI);
    vbo_bind(VBO_GUI);
    ebo_bind(EBO_GUI);
    glDrawElements(GL_TRIANGLES, ebo_length(EBO_GUI), GL_UNSIGNED_INT, 0);

    vao_bind(VAO_FONT);
    vbo_bind(VBO_FONT);
    ebo_bind(EBO_FONT);
    glDrawElements(GL_TRIANGLES, ebo_length(EBO_FONT), GL_UNSIGNED_INT, 0);

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

/* --------------------------------- */

void GLAPIENTRY message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    printf("\n%x, %x, %d, %p\n", source, id, length, userParam);
    printf("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ), type, severity, message);
    exit(1);
}
