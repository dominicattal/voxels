#ifndef TEXTURE_H
#define TEXTURE_H

#include "../../util.h"

typedef struct {
    u32 id;
    bool loaded;
    u64 handle;
} Texture;

Texture texture_create(const char* image_path);
Texture texture_create_pixels(GLenum type, i32 width, i32 height, const unsigned char* pixels);
void texture_bind(Texture texture);
void texture_load(Texture* texture);
void texture_unload(Texture* texture);
void texture_destroy(Texture texture);

#endif