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

#define NUM_TEXTURES 1

typedef enum {
    FONT_BITMAP = 0
} TextureID;

Texture texture_create(const char* image_path);
Texture texture_create_pixels(i32 width, i32 height, const unsigned char* pixels);
void texture_bind(Texture texture);
void texture_load(Texture* texture);
void texture_unload(Texture* texture);
void texture_destroy(Texture texture);

#endif