#include "ssbo.h"

SSBO ssbo_create(size_t size)
{
    SSBO ssbo;
    glGenBuffers(1, &ssbo.id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo.id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, GL_STATIC_DRAW);
    return ssbo;
}

void ssbo_bind(SSBO ssbo)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo.id);
}

void ssbo_bind_buffer_base(SSBO ssbo, SSBOID index)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, ssbo.id);
}

void ssbo_update(SSBO ssbo, size_t offset, size_t size, void *data)
{
    ssbo_bind(ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
}

void ssbo_destroy(SSBO ssbo)
{
    glDeleteBuffers(1, &ssbo.id);
}