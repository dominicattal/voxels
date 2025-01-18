#ifndef CHUCK_H
#define CHUCK_H

#include "block.h"

void chunk_init(void);
void chunk_prepare_render(void);
void chunk_render(void);
void chunk_destroy(void);

Block chunk_break_block(void);
void  chunk_place_block(Block block);

#endif