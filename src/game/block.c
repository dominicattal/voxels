#include "block.h"
#include "../renderer/renderer.h"

void block_init(void)
{
    u8 tx[] = {0, 1, 0, 1};
    u8 ty[] = {0, 0, 1, 1};
    u8 windingCCW[] = {0, 1, 3, 2};

    u8 vbo_buffer[4];
    u32 ebo_buffer[4];

    for (i32 i = 0; i < 4; i++) {
        vbo_buffer[i] = 0;
        vbo_buffer[i] |= tx[i];
        vbo_buffer[i] |= ty[i] << 1;
    }

    for (i32 i = 0; i < 4; i++)
        ebo_buffer[i] = windingCCW[i];

    vbo_bind(VBO_GAME);
    vbo_malloc(VBO_GAME, sizeof(vbo_buffer), GL_STATIC_DRAW);
    vbo_update(VBO_GAME, 0, sizeof(vbo_buffer), vbo_buffer);
    ebo_bind(EBO_GAME);
    ebo_malloc(EBO_GAME, sizeof(ebo_buffer), GL_STATIC_DRAW);
    ebo_update(EBO_GAME, 0, sizeof(ebo_buffer), ebo_buffer);
}

static u8 faces[6 * NUM_BLOCKS] = {
    0, 0, 0, 0, 0, 0,
    TEX_GRASS_SIDE, TEX_GRASS_SIDE, TEX_GRASS_BOTTOM, TEX_GRASS_TOP, TEX_GRASS_SIDE, TEX_GRASS_SIDE, 
    TEX_STONE, TEX_STONE, TEX_STONE, TEX_STONE, TEX_STONE, TEX_STONE
};

u8 block_face(Block block, u8 axis)
{
    return faces[6*block+axis];
}