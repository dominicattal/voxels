#include "block.h"
#include "../renderer/renderer.h"

#define VERTICES_PER_FACE   4
#define INDICES_PER_FACE    6
#define FACES_PER_BLOCK     6

void block_init(void)
{
    u8 dx[] = {0, 0, 0, 0, 1, 1, 1, 1};
    u8 dy[] = {0, 0, 1, 1, 0, 0, 1, 1};
    u8 dz[] = {0, 1, 0, 1, 0, 1, 0, 1};
    u8 tx[] = {0, 1, 1, 0};
    u8 ty[] = {0, 0, 1, 1};
    u8 winding[] = {0, 1, 2, 0, 2, 3};
    u8 faces[][4] = {
        {4, 5, 7, 6}, // +x
        {1, 0, 2, 3}, // -x
        {2, 6, 7, 3}, // +y
        {0, 1, 5, 4}, // -y
        {5, 1, 3, 7}, // +z
        {0, 4, 6, 2}  // -z
    };
    u8 vbo_buffer[FACES_PER_BLOCK * VERTICES_PER_FACE];
    u32 ebo_buffer[FACES_PER_BLOCK * INDICES_PER_FACE];
    i32 vbo_idx = 0, ebo_idx = 0;
    for (i32 face_num = 0; face_num < FACES_PER_BLOCK; face_num++) {
        for (i32 i = 0; i < VERTICES_PER_FACE; i++) {
            vbo_buffer[vbo_idx] = 0;
            vbo_buffer[vbo_idx] |= dx[faces[face_num][i]];
            vbo_buffer[vbo_idx] |= dy[faces[face_num][i]] << 1;
            vbo_buffer[vbo_idx] |= dz[faces[face_num][i]] << 2;
            vbo_buffer[vbo_idx] |= tx[i] << 3;
            vbo_buffer[vbo_idx] |= ty[i] << 4;
            vbo_idx++;
        }
        for (i32 i = 0; i < INDICES_PER_FACE; i++)
            ebo_buffer[ebo_idx++] = face_num * 4 + winding[i];
    }

    vbo_bind(VBO_GAME);
    vbo_malloc(VBO_GAME, sizeof(vbo_buffer), GL_STATIC_DRAW);
    vbo_update(VBO_GAME, 0, sizeof(vbo_buffer), vbo_buffer);
    ebo_bind(EBO_GAME);
    ebo_malloc(EBO_GAME, sizeof(ebo_buffer), GL_STATIC_DRAW);
    ebo_update(EBO_GAME, 0, sizeof(ebo_buffer), ebo_buffer);
}