#include "ubo.h"

static u32 ubos[NUM_UBOS];

void ubo_init(void)
{
    glGenBuffers(NUM_UBOS, ubos);
}

void ubo_destroy(void)
{
    glDeleteBuffers(NUM_UBOS, ubos);
}