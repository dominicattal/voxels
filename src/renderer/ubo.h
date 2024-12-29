#ifndef UBO_H
#define UBO_H

#include "../util/util.h"

#define NUM_UBOS 1

typedef enum {
    UBO_MATRICES = 0
} UBO;

void ubo_init(void);
void ubo_bind(UBO ubo);
void ubo_bind_buffer_base(UBO ubo, u32 index);
void ubo_malloc(UBO ubo, size_t size, GLenum usage);
void ubo_update(UBO ubo, size_t offset, size_t size, void* buffer);
void ubo_destroy(void);

#endif