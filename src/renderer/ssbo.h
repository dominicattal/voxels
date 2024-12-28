#ifndef SSBO_H
#define SSBO_H

#include "../util/util.h"

#define NUM_SSBOS 1

typedef enum {
    SSBO_TEXTURE = 0
} SSBO;

void ssbo_init(void);
void ssbo_bind(SSBO ssbo);
void ssbo_bind_buffer_base(SSBO ssbo, u32 index);
void ssbo_malloc(SSBO ssbo, size_t size, GLenum usage);
void ssbo_update(SSBO ssbo, size_t offset, size_t size, void* buffer);
void ssbo_destroy(void);

#endif