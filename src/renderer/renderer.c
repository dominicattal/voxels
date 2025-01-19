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
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    shader_init();
    ssbo_init();
    ubo_init();
    texture_init();
    vbo_init();
    ebo_init();
    dibo_init();
    vao_init();

    ssbo_bind(SSBO_GAME);
    ssbo_bind_buffer_base(SSBO_GAME, 2);

    ubo_bind(UBO_MATRICES);
    ubo_malloc(UBO_MATRICES, 32 * sizeof(f32), GL_STATIC_DRAW);
    shader_bind_uniform_block(SHADER_GAME, 1, "Matrices");
    ubo_bind_buffer_base(UBO_MATRICES, 1);
}

void renderer_render(void)
{
    f64 start = get_time();

    game_prepare_render();
    gui_prepare_render();

    glClearColor(0.2f, 0.8f, 1.0f, 1.0f);
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
    ssbo_destroy();
    ubo_destroy();
    dibo_destroy();
    ebo_destroy();
    texture_destroy();
}

f64 renderer_dt(void)
{
    return renderer.dt;
}

void renderer_uniform_update_view(f32* mat)
{
    ubo_update(UBO_MATRICES, 0, 16 * sizeof(f32), mat);
}

void renderer_uniform_update_proj(f32* mat)
{
    ubo_update(UBO_MATRICES, 16 * sizeof(f32), 16 * sizeof(f32), mat);
}

void renderer_toggle_line_mode(void)
{
    static int mode;
    mode = 1 - mode;
    glPolygonMode(GL_FRONT_AND_BACK, mode ? GL_LINE : GL_FILL);
}

void GLAPIENTRY message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION && !ENV_SHOW_GL_NOTIFICATIONS)
        return;

    puts("======== gl message callback ========");
    char* source_str;
    char* type_str;
    char* severity_str;
    switch (source) {
        case GL_DEBUG_SOURCE_API:
            source_str = "GL_DEBUG_SOURCE_API"; break;
        default:
            source_str = ""; break;
    }
    switch (type) {
        case GL_DEBUG_TYPE_OTHER:
            type_str = "GL_DEBUG_TYPE_OTHER"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            type_str = "GL_DEBUG_TYPE_PERFORMANCE"; break;
        default:
            type_str = ""; break;
    }
    switch (severity) {
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            severity_str = "GL_DEBUG_SEVERITY_NOTIFICATION"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            severity_str = "GL_DEBUG_SEVERITY_MEDIUM"; break;
        default:
            severity_str = ""; break;
    }
    printf("%-8s = 0x%04x %s\n", "source", source, source_str);
    printf("%-8s = 0x%04x %s\n", "type", type, type_str);
    printf("%-8s = 0x%04x %s\n", "severity", severity, severity_str);
    printf("%-8s = %u\n\n", "id", id);
    printf("%s\n\n", message);
    
    if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
        exit(1);
}
