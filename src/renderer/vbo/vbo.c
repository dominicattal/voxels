#include "vbo.h"
#include <glad.h>
#include <stdlib.h>

VBO vbo_create(void)
{
    VBO vbo;
    glGenBuffers(1, &vbo.id);
    vbo.length = vbo.max_length = 0;
    return vbo;
}

void vbo_bind(VBO vbo)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo.id);
}

void vbo_destroy(VBO vbo)
{
    glDeleteBuffers(1, &vbo.id);
}

void vbo_malloc(VBO* vbo, u32 length, GLenum usage)
{
    if (length == vbo->max_length)
        return;
    glBindBuffer(GL_ARRAY_BUFFER, vbo->id);
    glBufferData(GL_ARRAY_BUFFER, length * sizeof(f32), NULL, usage);
    vbo->max_length = length;
}

void vbo_update(VBO* vbo, u32 offset, u32 length, f32* buffer)
{
    vbo->length = length;
    glBindBuffer(GL_ARRAY_BUFFER, vbo->id);
    glBufferSubData(GL_ARRAY_BUFFER, offset, (length - offset) * sizeof(f32), buffer);
}
