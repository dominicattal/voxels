#include "ssbo.h"

static u32 ssbos[NUM_SSBOS];

void ssbo_init(void)
{
    glGenBuffers(NUM_SSBOS, ssbos);
}

void ssbo_bind(SSBO ssbo)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbos[ssbo]);
}

void ssbo_bind_buffer_base(SSBO ssbo, u32 index)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, ssbos[ssbo]);
}

void ssbo_malloc(SSBO ssbo, size_t size, GLenum usage)
{
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, usage);
}

void ssbo_update(SSBO ssbo, size_t offset, size_t size, void* buffer)
{
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, buffer);
}

void ssbo_destroy(void)
{
    glDeleteBuffers(NUM_SSBOS, ssbos);
}