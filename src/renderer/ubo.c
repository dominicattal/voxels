#include "ubo.h"

static u32 ubos[NUM_UBOS];

void ubo_init(void)
{
    glGenBuffers(NUM_UBOS, ubos);
}

void ubo_bind(UBO ubo)
{
    glBindBuffer(GL_UNIFORM_BUFFER, ubos[ubo]);
}

void ubo_bind_buffer_base(UBO ubo, u32 index)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, index, ubos[ubo]);
}

void ubo_malloc(UBO ubo, size_t size, GLenum usage)
{
    glBufferData(GL_UNIFORM_BUFFER, size, NULL, usage);
}

void ubo_update(UBO ubo, size_t offset, size_t size, void* buffer)
{
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, buffer);
}

void ubo_destroy(void)
{
    glDeleteBuffers(NUM_UBOS, ubos);
}