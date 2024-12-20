#include "ebo.h"
#include <stdlib.h>

static struct {
    u32 id, length, max_length;
} ebos[NUM_EBOS];

void ebo_init(void)
{
    for (i32 i = 0; i < NUM_EBOS; i++)
        glGenBuffers(1, &ebos[i].id);
}

void ebo_bind(EBO ebo)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebos[ebo].id);
}

void ebo_destroy(void)
{
    for (i32 i = 0; i < NUM_EBOS; i++)
        glDeleteBuffers(1, &ebos[i].id);
}

void ebo_malloc(EBO ebo, u32 length, GLenum usage)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebos[ebo].id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, length * sizeof(u32), NULL, usage);
}

void ebo_update(EBO ebo, u32 offset, u32 length, u32* buffer)
{
    ebos[ebo].length = length;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebos[ebo].id);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, (length - offset) * sizeof(u32), buffer);
}

u32 ebo_length(EBO ebo)
{
    return ebos[ebo].length;
}