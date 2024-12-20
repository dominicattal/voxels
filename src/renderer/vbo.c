#include "vbo.h"
#include <glad.h>
#include <stdlib.h>

static struct {
    u32 id, length, max_length;
} vbos[NUM_VBOS];

void vbo_init(void)
{
    for (i32 i = 0; i < NUM_VBOS; i++) {
        glGenBuffers(1, &vbos[i].id);
        vbos[i].length = vbos[i].max_length = 0;
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

void vbo_malloc(VBO vbo, u32 length, GLenum usage)
{
    if (length == vbos[vbo].max_length)
        return;
    glBindBuffer(GL_ARRAY_BUFFER, vbos[vbo].id);
    glBufferData(GL_ARRAY_BUFFER, length * sizeof(f32), NULL, usage);
    vbos[vbo].max_length = length;
}

void vbo_update(VBO vbo, u32 offset, u32 length, f32* buffer)
{
    vbos[vbo].length = length;
    glBindBuffer(GL_ARRAY_BUFFER, vbos[vbo].id);
    glBufferSubData(GL_ARRAY_BUFFER, offset, (length - offset) * sizeof(f32), buffer);
}
