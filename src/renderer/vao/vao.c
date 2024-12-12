#include "vao.h"

VAO vao_create(void)
{
    VAO vao;
    glGenVertexArrays(1, &vao.id);
    return vao;
}


void vao_bind(VAO vao)
{
    glBindVertexArray(vao.id);
}

void vao_destroy(VAO vao)
{
    glDeleteVertexArrays(1, &vao.id);
}
