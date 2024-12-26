#include "vbo.h"
#include <glad.h>
#include <stdlib.h>

static struct {
    u32 id;
    size_t max_size;
} vbos[NUM_VBOS];

void vbo_init(void)
{
    for (i32 i = 0; i < NUM_VBOS; i++) {
        glGenBuffers(1, &vbos[i].id);
        vbos[i].max_size = 0;
    }
}

void vbo_bind(VBO vbo)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbos[vbo].id);
}

void vbo_destroy(void)
{
    for (i32 i = 0; i < NUM_VBOS; i++)
        glDeleteBuffers(1, &vbos[i].id);
}

void vbo_malloc(VBO vbo, size_t size, GLenum usage)
{
    if (size == vbos[vbo].max_size)
        return;
    glBindBuffer(GL_ARRAY_BUFFER, vbos[vbo].id);
    glBufferData(GL_ARRAY_BUFFER, size, NULL, usage);
    vbos[vbo].max_size = size;
}

void vbo_update(VBO vbo, size_t offset, size_t size, f32* buffer)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbos[vbo].id);
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, buffer);
}
