#include "texture.h"
#include <stdio.h>

Texture texture_create(const char* image_path)
{
    Texture texture;
    i32 width, height, nrChannels;
    unsigned char *data = stbi_load(image_path, &width, &height, &nrChannels, 4);
    f32 col[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    if (data == NULL) {
        printf("Could not open %s\n", image_path);
        exit(1);
    }
    glCreateTextures(GL_TEXTURE_2D, 1, &texture.id);
    glTextureParameteri(texture.id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(texture.id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(texture.id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(texture.id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameterfv(texture.id, GL_TEXTURE_BORDER_COLOR, col);
    glTextureStorage2D(texture.id, 1, GL_RGBA8, width, height);
    glTextureSubImage2D(texture.id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    texture.loaded = FALSE;
    texture_load(&texture);
    stbi_image_free(data);
    return texture;
}

Texture texture_create_pixels(i32 width, i32 height, const unsigned char* pixels)
{
    Texture texture;
    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    texture.loaded = FALSE;
    texture_load(&texture);
    return texture;
}

void texture_bind(Texture texture)
{
    glBindTexture(GL_TEXTURE_2D, texture.id);
}

void texture_load(Texture* texture)
{
    if (!texture->loaded) {
        texture->handle = glGetTextureHandleARB(texture->id);
        glMakeTextureHandleResidentARB(texture->handle);
        texture->loaded = TRUE;
    }
}

void texture_unload(Texture* texture)
{
    if (texture->loaded) {
        glMakeTextureHandleNonResidentARB(texture->handle);
        texture->loaded = FALSE;
    }
}

void texture_destroy(Texture texture)
{
    glDeleteTextures(1, &texture.id);
}