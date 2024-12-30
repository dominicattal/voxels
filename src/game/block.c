#include "block.h"

Block block_create(u32 id, i32 x, i32 y, i32 z)
{
    Block block;
    block.id = id;
    block.position.x = x;
    block.position.y = y;
    block.position.z = z;
    return block;
}
