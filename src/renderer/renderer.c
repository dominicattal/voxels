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

#define NUM_VAOS 2

typedef enum {
    VAO_GUI = 0,
    VAO_FONT = 1
} VAOID;

#define NUM_VBOS 2

typedef enum {
    VBO_GUI = 0,
    VBO_FONT = 1
} VBOID;

#define NUM_EBOS 2

typedef enum {
    EBO_GUI = 0,
    EBO_FONT = 1
} EBOID;


typedef struct {
    VAO vaos[NUM_VAOS];
    VBO vbos[NUM_VBOS];
    EBO ebos[NUM_EBOS];
    SSBO ssbos[NUM_SSBOS];
    Shader shaders[NUM_SHADERS];
    Texture textures[NUM_TEXTURES];
    f64 dt;
} Renderer;

static Renderer renderer;

static void message_callback();
static void link_shader_ssbo();

static void initialize_textures()
{
    unsigned char pixels[4];
    pixels[0] = pixels[1] = pixels[2] = pixels[3] = 0;
    renderer.textures[TEX_NONE] = texture_create_pixels(GL_RGBA, 1, 1, pixels);
    pixels[0] = pixels[1] = pixels[2] = pixels[3] = 255;
    renderer.textures[TEX_COLOR] = texture_create_pixels(GL_RGB, 1, 1, pixels);
}

static void initialize_shaders(void)
{
    u32 vert, frag;
    renderer.shaders[SHADER_DEFAULT] = shader_create();
    vert = shader_compile(GL_VERTEX_SHADER, "src/renderer/shaders/default/default.vert");
    frag = shader_compile(GL_FRAGMENT_SHADER, "src/renderer/shaders/default/default.frag");
    glAttachShader(renderer.shaders[SHADER_DEFAULT].id, vert);
    glAttachShader(renderer.shaders[SHADER_DEFAULT].id, frag);
    shader_link(renderer.shaders[SHADER_DEFAULT]);
    glDetachShader(renderer.shaders[SHADER_DEFAULT].id, vert);
    glDetachShader(renderer.shaders[SHADER_DEFAULT].id, frag);
    glDeleteShader(vert);
    glDeleteShader(frag);
}

static void initialize_buffers(void)
{
    renderer.vbos[VBO_GUI] = vbo_create();
    renderer.vbos[VBO_FONT] = vbo_create();
    renderer.ebos[EBO_GUI] = ebo_create();
    renderer.ebos[EBO_FONT] = ebo_create();

    renderer.ssbos[SSBO_TEXTURES] = ssbo_create(NUM_TEXTURES * sizeof(u64));

    u64 handles[NUM_TEXTURES];
    for (i32 i = 0; i < NUM_TEXTURES; i++)
        handles[i] = renderer.textures[i].handle;
    ssbo_update(renderer.ssbos[SSBO_TEXTURES], 0, NUM_TEXTURES * sizeof(u64), handles);

    link_shader_ssbo(SHADER_DEFAULT, SSBO_TEXTURES);
}

static void initialize_vaos(void)
{
    renderer.vaos[VAO_GUI] = vao_create();

    vao_bind(renderer.vaos[VAO_GUI]);
    vbo_bind(renderer.vbos[VBO_GUI]);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(0 * sizeof(f32)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(2 * sizeof(f32)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(4 * sizeof(f32)));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(8 * sizeof(f32)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    renderer.vaos[VAO_FONT] = vao_create();

    vao_bind(renderer.vaos[VAO_FONT]);
    vbo_bind(renderer.vbos[VBO_FONT]);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(0 * sizeof(f32)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(2 * sizeof(f32)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(4 * sizeof(f32)));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(8 * sizeof(f32)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
}

void renderer_init(void)
{
    renderer.dt = 0;

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(message_callback, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initialize_textures();
    initialize_shaders();
    initialize_buffers();
    initialize_vaos();
}

void renderer_render(void)
{
    GUIData gui_data = gui_get_data();
    vbo_malloc(&renderer.vbos[VBO_GUI],  gui_data.comp_vbo_max_length, GL_STATIC_DRAW);
    ebo_malloc(&renderer.ebos[EBO_GUI],  gui_data.comp_ebo_max_length, GL_STATIC_DRAW);
    vbo_update(&renderer.vbos[VBO_GUI],  0, gui_data.comp_vbo_length, gui_data.comp_vbo_buffer);
    ebo_update(&renderer.ebos[EBO_GUI],  0, gui_data.comp_ebo_length, gui_data.comp_ebo_buffer);
    vbo_malloc(&renderer.vbos[VBO_FONT], gui_data.font_vbo_max_length, GL_STATIC_DRAW);
    ebo_malloc(&renderer.ebos[EBO_FONT], gui_data.font_ebo_max_length, GL_STATIC_DRAW);
    vbo_update(&renderer.vbos[VBO_FONT], 0, gui_data.font_vbo_length, gui_data.font_vbo_buffer);
    ebo_update(&renderer.ebos[EBO_FONT], 0, gui_data.font_ebo_length, gui_data.font_ebo_buffer);

    f64 start = get_time();

    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_use(renderer.shaders[SHADER_DEFAULT]);

    vao_bind(renderer.vaos[VAO_GUI]);
    vbo_bind(renderer.vbos[VBO_GUI]);
    ebo_bind(renderer.ebos[EBO_GUI]);
    glDrawElements(GL_TRIANGLES, renderer.ebos[EBO_GUI].length, GL_UNSIGNED_INT, 0);

    vao_bind(renderer.vaos[VAO_FONT]);
    vbo_bind(renderer.vbos[VBO_FONT]);
    ebo_bind(renderer.ebos[EBO_FONT]);
    glDrawElements(GL_TRIANGLES, renderer.ebos[EBO_FONT].length, GL_UNSIGNED_INT, 0);

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
    for (i = 0; i < NUM_VBOS; i++)
        vbo_destroy(renderer.vbos[i]);
    for (i = 0; i < NUM_EBOS; i++)
        ebo_destroy(renderer.ebos[i]);
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

