#ifndef TEXTURE_H
#define TEXTURE_H

#include "../util/util.h"

#define NUM_TEXTURES 7

typedef enum {
    TEX_NONE = 0,
    TEX_COLOR = 1,
    TEX_BITMAP = 2,
    TEX_OBJECT1 = 3,
    TEX_OBJECT2 = 4,
    TEX_OBJECT3 = 5,
    TEX_OBJECT4 = 6
} Texture;

void texture_init(void);
void texture_destroy(void);

void texture_get_info(Texture texture, u32* location, f32* u, f32* v, f32* w, f32* h);
u32  texture_location(Texture texture);

#endif