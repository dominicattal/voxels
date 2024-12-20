#include "shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

static u32 shaders[NUM_SHADERS];

static const char* read_file(const char *path)
{
    FILE* ptr;
    char* content;
    ptr = fopen(path, "r");
    fseek(ptr, 0, SEEK_END);
    i32 len = ftell(ptr);
    if (len == 0) {
        printf("File %s is empty", path);
        exit(1);
    }
    fseek(ptr, 0, SEEK_SET);
    content = calloc(len, sizeof(char));
    fread(content, 1, len, ptr);
    fclose(ptr);
    return content;
}

static u32 compile(GLenum type, const char *path)
{
    u32 shader;
    const char* shader_code;
    char info_log[512];
    i32 success;
    DIR* dir = opendir(path);
    if (ENOENT == errno) {
        printf("File %s does not exist", path);
        exit(1);
    }
    closedir(dir);
    shader = glCreateShader(type);
    shader_code = read_file(path);
    glShaderSource(shader, 1, &shader_code, NULL);
    free((char*)shader_code);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        puts(path);
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        printf(info_log);
        exit(1);
    }
    return shader;
}

static void link(Shader id)
{
    char info_log[512];
    i32 success;
    glLinkProgram(shaders[id]);
    glGetProgramiv(shaders[id], GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaders[id], 512, NULL, info_log);
        printf(info_log);
        exit(1);
    }
}

static void attach(Shader id, u32 shader)
{
    glAttachShader(shaders[id], shader);
}

static void detach(Shader id, u32 shader)
{
    glDetachShader(shaders[id], shader);
}

static void delete(u32 shader)
{
    glDeleteShader(shader);
}

void shader_init(void)
{
    for (i32 i = 0; i < NUM_SHADERS; i++)
        shaders[i] = glCreateProgram();

    u32 vert, frag;
    vert = compile(GL_VERTEX_SHADER, "src/renderer/shaders/gui.vert");
    frag = compile(GL_FRAGMENT_SHADER, "src/renderer/shaders/gui.frag");
    attach(SHADER_GUI, vert);
    attach(SHADER_GUI, frag);
    link(SHADER_GUI);
    detach(SHADER_GUI, vert);
    detach(SHADER_GUI, frag);
    delete(vert);
    delete(frag);
    shader_use(SHADER_GUI);
    glUniform1i(glGetUniformLocation(shaders[SHADER_GUI], "TEX_ID_FONT"), 2);

    vert = compile(GL_VERTEX_SHADER, "src/renderer/shaders/test.vert");
    frag = compile(GL_FRAGMENT_SHADER, "src/renderer/shaders/test.frag");
    attach(SHADER_GAME, vert);
    attach(SHADER_GAME, frag);
    link(SHADER_GAME);
    detach(SHADER_GAME, vert);
    detach(SHADER_GAME, frag);
    delete(vert);
    delete(frag);
}

void shader_use(Shader id)
{
    glUseProgram(shaders[id]);
}

void shader_destroy(void)
{
    for (i32 i = 0; i < NUM_SHADERS; i++)
        glDeleteProgram(shaders[i]);
}

u32 shader_get_uniform_location(Shader shader, const char* identifier)
{
    return glGetUniformLocation(shaders[shader], identifier);
}

void shader_uniform_matrix_4fv(Shader shader, const char* identifier, i32 count, const f32* value)
{
    shader_use(shader);
    glUniformMatrix4fv(glGetUniformLocation(shaders[shader], identifier), count, GL_FALSE, value);
}