#ifndef DIBO_H
#define DIBO_H

#include "../util/util.h"

#define NUM_DIBOS 1

typedef enum {
    DIBO_GAME = 0
} DIBO;

void dibo_init(void);
void dibo_bind(DIBO dibo);
void dibo_malloc(DIBO dibo, size_t size, GLenum usage);
void dibo_update(DIBO dibo, size_t offset, size_t size, void* buffer);
void dibo_destroy(void);

#endif