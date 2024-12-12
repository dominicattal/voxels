#include "shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

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

Shader shader_create(void)
{
    Shader shader;
    shader.id = glCreateProgram();
    return shader;
}

void shader_link(Shader shader)
{
    char info_log[512];
    i32 success;
    glLinkProgram(shader.id);
    glGetProgramiv(shader.id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shader.id, 512, NULL, info_log);
        printf(info_log);
        exit(1);
    }
}

void shader_bind_uniform_block(Shader shader, u32 index, const char* identifier)
{
    glUniformBlockBinding(shader.id, glGetUniformBlockIndex(shader.id, identifier), index);
}

void shader_use(Shader shader)
{
    glUseProgram(shader.id);
}

void shader_destroy(Shader shader)
{
    glDeleteProgram(shader.id);
}