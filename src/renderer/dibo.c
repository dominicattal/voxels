#include "dibo.h"

static u32 dibos[NUM_DIBOS];

void dibo_init(void)
{
    glGenBuffers(NUM_DIBOS, dibos);
}

void dibo_bind(DIBO dibo)
{
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, dibos[dibo]);
}

void dibo_malloc(DIBO dibo, size_t size, GLenum usage)
{
    glBufferData(GL_DRAW_INDIRECT_BUFFER, size, NULL, usage);
}

void dibo_update(DIBO dibo, size_t offset, size_t size, void* buffer)
{
    glBufferSubData(GL_DRAW_INDIRECT_BUFFER, offset, size, buffer);
}

void dibo_destroy(void)
{
    glDeleteBuffers(NUM_DIBOS, dibos);
}