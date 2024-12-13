#include "vao.h"

static u32 vaos[NUM_VAOS];

void vao_init(void)
{
    glGenVertexArrays(NUM_VAOS, vaos);
}

void vao_bind(VAO vao)
{
    glBindVertexArray(vaos[vao]);
}

void vao_destroy(void)
{
    glDeleteVertexArrays(NUM_VAOS, vaos);
}
