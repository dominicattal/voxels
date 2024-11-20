#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad.h>
#include <glfw.h>
#include <stb_image.h>
#include "../../util.h"

typedef struct {
    u32 id;
    bool loaded;
    u64 handle;
} Texture;

#define NUM_TEXTURES 3

typedef enum {
    TEX_NONE = 0,
    TEX_COLOR = 1,
    TEX_BITMAP = 2
} TextureID;

Texture texture_create(const char* image_path);
Texture texture_create_pixels(GLenum type, i32 width, i32 height, const unsigned char* pixels);
void texture_bind(Texture texture);
void texture_load(Texture* texture);
void texture_unload(Texture* texture);
void texture_destroy(Texture texture);

#endif