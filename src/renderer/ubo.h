#ifndef UBO_H
#define UBO_H

#include "../util/util.h"

#define NUM_UBOS 0

typedef enum {
    NOTHING
} UBO;

void ubo_init(void);
void ubo_destroy(void);

#endif