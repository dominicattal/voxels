#include "vbo.h"
#include <glad.h>
#include <stdlib.h>

static u32 vbos[NUM_VBOS];

void vbo_init(void)
{
    glGenBuffers(NUM_VBOS, vbos);
}

void vbo_bind(VBO vbo)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbos[vbo]);
}

void vbo_destroy(void)
{
    glDeleteBuffers(NUM_VBOS, vbos);
}

void vbo_malloc(VBO vbo, size_t size, GLenum usage)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbos[vbo]);
    glBufferData(GL_ARRAY_BUFFER, size, NULL, usage);
}

void vbo_update(VBO vbo, size_t offset, size_t size, void* buffer)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbos[vbo]);
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, buffer);
}
