#ifndef TEXTURE_H
#define TEXTURE_H

#include "../../type.h"

#define NUM_TEXTURES 3

typedef enum {
    TEX_NONE = 0,
    TEX_COLOR = 1,
    TEX_BITMAP = 2
} Texture;

void texture_init(void);
void texture_bind(Texture texture, u32 location);
void texture_destroy(void);

#endif