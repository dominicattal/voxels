#include "block.h"
#include "../renderer/renderer.h"

void block_init(void)
{
    u8 tx[] = {0, 1, 1, 0};
    u8 ty[] = {0, 0, 1, 1};
    u8 windingCCW[] = {0, 1, 2, 0, 2, 3};

    u8 vbo_buffer[4];
    u32 ebo_buffer[6];

    for (i32 i = 0; i < 4; i++) {
        vbo_buffer[i] = 0;
        vbo_buffer[i] |= tx[i];
        vbo_buffer[i] |= ty[i] << 1;
    }

    for (i32 i = 0; i < 6; i++)
        ebo_buffer[i] = windingCCW[i];

    vbo_bind(VBO_GAME);
    vbo_malloc(VBO_GAME, sizeof(vbo_buffer), GL_STATIC_DRAW);
    vbo_update(VBO_GAME, 0, sizeof(vbo_buffer), vbo_buffer);
    ebo_bind(EBO_GAME);
    ebo_malloc(EBO_GAME, sizeof(ebo_buffer), GL_STATIC_DRAW);
    ebo_update(EBO_GAME, 0, sizeof(ebo_buffer), ebo_buffer);
}