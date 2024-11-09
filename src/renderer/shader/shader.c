#include "shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

static const char* read_file(char *path)
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

static unsigned int compile(char *s_path, GLenum type)
{
    u32 shader;
    const char* shader_code;
    char info_log[512];
    i32 success;
    DIR* dir = opendir(s_path);
    if (ENOENT == errno) {
        printf("File %s does not exist", s_path);
        exit(1);
    }
    closedir(dir);
    shader = glCreateShader(type);
    shader_code = read_file(s_path);
    glShaderSource(shader, 1, &shader_code, NULL);
    free((char*)shader_code);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        puts(s_path);
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        printf(info_log);
        exit(1);
    }
    return shader;
}

Shader shader_create(char* vs_path, char* fs_path)
{
    Shader shader;
    u32 vertex, fragment;
    char info_log[512];
    i32 success;
    
    shader.id = glCreateProgram();
    vertex = compile(vs_path, GL_VERTEX_SHADER);
    glAttachShader(shader.id, vertex);
    fragment = compile(fs_path, GL_FRAGMENT_SHADER);
    glAttachShader(shader.id, fragment);

    glLinkProgram(shader.id);
    glGetProgramiv(shader.id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shader.id, 512, NULL, info_log);
        printf(info_log);
        exit(1);
    }
    glDetachShader(shader.id, vertex);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    glDetachShader(shader.id, fragment);
    return shader;
}

void shader_bind_uniform_block(Shader shader, u32 index, char* identifier)
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