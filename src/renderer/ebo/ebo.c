#include "ebo.h"
#include <stdlib.h>

EBO ebo_create(void)
{
    EBO ebo;
    glGenBuffers(1, &ebo.id);
    ebo.length = 0;
    return ebo;
}

void ebo_bind(EBO ebo)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.id);
}

void ebo_destroy(EBO ebo)
{
    glDeleteBuffers(1, &ebo.id);
}

void ebo_malloc(EBO* ebo, u32 length, GLenum usage)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo->id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, length * sizeof(u32), NULL, usage);
}

void ebo_update(EBO* ebo, u32 offset, u32 length, u32* buffer)
{
    ebo->length = length;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo->id);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, (length - offset) * sizeof(u32), buffer);
}

