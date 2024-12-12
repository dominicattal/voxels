#ifndef TEXTURE_H
#define TEXTURE_H

#include "../../type.h"

typedef struct {
    u32 id;
} Texture;

Texture texture_create(const char* image_path);
Texture texture_create_pixels(GLenum type, i32 width, i32 height, const unsigned char* pixels);
void texture_bind(Texture texture, u32 location);
void texture_destroy(Texture texture);

#endif