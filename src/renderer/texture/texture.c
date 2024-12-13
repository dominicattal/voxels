#include "texture.h"
#include "../../font/font.h"
#include <stdio.h>
#include <stdlib.h>
#include <stb_image.h>

static struct {
    u32 id;
} textures[NUM_TEXTURES];

static u32 texture_create(const char* image_path)
{
    u32 texture;
    i32 width, height, nrChannels;
    unsigned char *data = stbi_load(image_path, &width, &height, &nrChannels, 4);
    f32 col[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    if (data == NULL) {
        printf("Could not open %s\n", image_path);
        exit(1);
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameterfv(texture, GL_TEXTURE_BORDER_COLOR, col);
    glTextureStorage2D(texture, 1, GL_RGBA8, width, height);
    glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    return texture;
}

static u32 texture_create_pixels(GLenum type, i32 width, i32 height, const unsigned char* pixels)
{
    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    return texture;
}

void texture_init(void)
{
    unsigned char pixels[4];
    pixels[0] = pixels[1] = pixels[2] = pixels[3] = 0;
    textures[TEX_NONE].id = texture_create_pixels(GL_RGBA, 1, 1, pixels);
    pixels[0] = pixels[1] = pixels[2] = pixels[3] = 255;
    textures[TEX_COLOR].id = texture_create_pixels(GL_RGB, 1, 1, pixels);

    i32 width, height;
    unsigned char* bitmap = font_bitmap(&width, &height);
    textures[TEX_BITMAP].id = texture_create_pixels(GL_RED, width, height, bitmap);
    free(bitmap);
    texture_bind(TEX_BITMAP, 0);
}

void texture_bind(Texture texture, u32 location)
{
    glActiveTexture(GL_TEXTURE0 + location);
    glBindTexture(GL_TEXTURE_2D, textures[texture].id);
}

void texture_destroy(void)
{
    for (i32 i = 0; i < NUM_TEXTURES; i++)
        glDeleteTextures(1, &textures[i].id);
}