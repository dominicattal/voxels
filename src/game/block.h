#ifndef BLOCK_H
#define BLOCK_H

#include "../util/util.h"

typedef u8 Block;

#define NUM_BLOCKS 3

typedef enum BlockEnum {
    AIR = 0,
    GRASS = 1,
    STONE = 2,
};

void block_init(void);
u8   block_face(Block block, u8 axis);

#endif