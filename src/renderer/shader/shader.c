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

u32 shader_compile(GLenum type, const char *path)
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

void shader_init(void)
{
    for (i32 i = 0; i < NUM_SHADERS; i++)
        shaders[i] = glCreateProgram();

    u32 vert, frag;
    vert = shader_compile(GL_VERTEX_SHADER, "src/renderer/shaders/default/default.vert");
    frag = shader_compile(GL_FRAGMENT_SHADER, "src/renderer/shaders/default/default.frag");
    shader_attach(SHADER_DEFAULT, vert);
    shader_attach(SHADER_DEFAULT, frag);
    shader_link(SHADER_DEFAULT);
    shader_detach(SHADER_DEFAULT, vert);
    shader_detach(SHADER_DEFAULT, frag);
    glDeleteShader(vert);
    glDeleteShader(frag);
}

void shader_link(ShaderID id)
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

void shader_attach(ShaderID id, u32 shader)
{
    glAttachShader(shaders[id], shader);
}

void shader_detach(ShaderID id, u32 shader)
{
    glDetachShader(shaders[id], shader);
}

void shader_use(ShaderID id)
{
    glUseProgram(shaders[id]);
}

void shader_destroy(void)
{
    for (i32 i = 0; i < NUM_SHADERS; i++)
        glDeleteProgram(shaders[i]);
}