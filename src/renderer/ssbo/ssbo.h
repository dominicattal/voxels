#ifndef SSBO_H
#define SSBO_H

#include "../../util.h"

typedef struct {
    u32 id;
} SSBO;

#define NUM_SSBOS 1

typedef enum {
    SSBO_TEXTURES = 0
} SSBOID;

SSBO ssbo_create(size_t size);
void ssbo_bind(SSBO ssbo);
void ssbo_bind_buffer_base(SSBO ssbo, SSBOID index);
void ssbo_update(SSBO ssbo, size_t offset, size_t size, void *data);
void ssbo_destroy(SSBO ssbo);

#endif