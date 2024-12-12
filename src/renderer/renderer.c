#include "renderer.h"
#include "../util.h"
#include "vao/vao.h"
#include "shader/shader.h"
#include "texture/texture.h"
#include "ssbo/ssbo.h"
#include <glad.h>
#include <stdio.h>
#include <stdlib.h>
#include "../gui/gui.h"

typedef struct {
    VAO vaos[NUM_VAOS];
    SSBO ssbos[NUM_SSBOS];
    Shader shaders[NUM_SHADERS];
    Texture textures[NUM_TEXTURES];
    f64 dt;
} Renderer;

static Renderer renderer;

static void message_callback();
static void link_shader_ssbo();
static void set_texture_ssbo();

void renderer_init(void)
{
    renderer.dt = 0;

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(message_callback, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    unsigned char pixels[4];
    pixels[0] = pixels[1] = pixels[2] = pixels[3] = 0;
    renderer.textures[TEX_NONE] = texture_create_pixels(GL_RGBA, 1, 1, pixels);
    pixels[0] = pixels[1] = pixels[2] = pixels[3] = 255;
    renderer.textures[TEX_COLOR] = texture_create_pixels(GL_RGB, 1, 1, pixels);

    renderer.shaders[SHADER_DEFAULT] = shader_create("src/renderer/shaders/default/default.vert", "src/renderer/shaders/default/default.frag");

    renderer.ssbos[SSBO_TEXTURES] = ssbo_create(NUM_TEXTURES * sizeof(u64));
    set_texture_ssbo();
    link_shader_ssbo(SHADER_DEFAULT, SSBO_TEXTURES);

    renderer.vaos[VAO_GUI] = vao_create(GL_STATIC_DRAW, GL_TRIANGLES, 9);

    vao_attr(renderer.vaos[VAO_GUI], 0, 2, 0);
    vao_attr(renderer.vaos[VAO_GUI], 1, 2, 2);
    vao_attr(renderer.vaos[VAO_GUI], 2, 4, 4);
    vao_attr(renderer.vaos[VAO_GUI], 3, 1, 8);

    renderer.vaos[VAO_FONT] = vao_create(GL_STATIC_DRAW, GL_TRIANGLES, 9);

    vao_attr(renderer.vaos[VAO_FONT], 0, 2, 0);
    vao_attr(renderer.vaos[VAO_FONT], 1, 2, 2);
    vao_attr(renderer.vaos[VAO_FONT], 2, 4, 4);
    vao_attr(renderer.vaos[VAO_FONT], 3, 1, 8);
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
    GUIData gui_data = gui_get_data();
    renderer_malloc(VAO_GUI, gui_data.comp_vbo_max_length, gui_data.comp_ebo_max_length);
    renderer_update(VAO_GUI, 0, gui_data.comp_vbo_length, gui_data.comp_vbo_buffer, 0, gui_data.comp_ebo_length, gui_data.comp_ebo_buffer);
    renderer_malloc(VAO_FONT, gui_data.font_vbo_max_length, gui_data.font_ebo_max_length);
    renderer_update(VAO_FONT, 0, gui_data.font_vbo_length, gui_data.font_vbo_buffer, 0, gui_data.font_ebo_length, gui_data.font_ebo_buffer);

    f64 start = get_time();

    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_use(renderer.shaders[SHADER_DEFAULT]);
    vao_draw(renderer.vaos[VAO_GUI]);
    vao_draw(renderer.vaos[VAO_FONT]);

    renderer.dt = get_time() - start;
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
    for (i = 0; i < NUM_TEXTURES; i++)
        texture_destroy(renderer.textures[i]);
}

f64 renderer_dt(void)
{
    return renderer.dt;
}

void renderer_create_font_bitmap(i32 width, i32 height, unsigned char* pixels)
{
    renderer.textures[TEX_BITMAP] = texture_create_pixels(GL_RED, width, height, pixels);
}

/* --------------------------------- */

void GLAPIENTRY message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    printf("\n%x, %x, %d, %p\n", source, id, length, userParam);
    printf("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ), type, severity, message);
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