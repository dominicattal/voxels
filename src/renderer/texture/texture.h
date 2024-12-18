#ifndef TEXTURE_H
#define TEXTURE_H

#include "../../util/util.h"

#define NUM_TEXTURES 6

typedef enum {
    TEX_NONE = 0,
    TEX_COLOR = 1,
    TEX_BITMAP = 2,
    TEX_OBJECT1 = 3,
    TEX_OBJECT2 = 4,
    TEX_OBJECT3 = 5
} Texture;

void texture_init(void);
void texture_destroy(void);

void texture_get_info(Texture texture, u32* location, f32* u1, f32* v1, f32* u2, f32* v2);

#endif