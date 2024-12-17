#include "texture.h"
#include "../../font/font.h"
#include "../shader/shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <stb_image.h>

#define NUM_TEXTURE_UNITS 3

typedef struct {
    u16 u1, v1, u2, v2;
} UV;

#define UVINIT(u, v, w, h) (UV) { u, v, u+w, v+h }

static struct {
    UV* coords;
    u32 id, location;
} texture_units[NUM_TEXTURE_UNITS];

typedef struct {
    u8 idx1, idx2;
} IDX;

#define IDXINIT(idx1, idx2) (IDX) { idx1, idx2 }

static IDX textures[NUM_TEXTURES];

static u32 texture_create_from_path(const char* image_path)
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

static u32 texture_create_from_pixels(GLenum type, i32 width, i32 height, const unsigned char* pixels)
{
    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    return texture;
}

void texture_init(void)
{
    i32 width, height;
    unsigned char* bitmap = font_bitmap(&width, &height);
    texture_units[0].id = texture_create_from_pixels(GL_RED, width, height, bitmap);
    free(bitmap);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_units[0].id);

    u32 tex;
    tex = texture_create_from_path("assets/textures/objects/object_atlas.png");
    texture_units[1].id = tex;
    texture_units[1].location = 1;
    texture_units[1].coords = malloc(3 * sizeof(u64));
    texture_units[1].coords[0] = UVINIT(0, 0, 16, 16);
    texture_units[1].coords[1] = UVINIT(17, 0, 16, 16);
    texture_units[1].coords[2] = UVINIT(34, 0, 16, 16);
    textures[TEX_OBJECT1] = IDXINIT(1, 0);
    textures[TEX_OBJECT2] = IDXINIT(1, 1);
    textures[TEX_OBJECT3] = IDXINIT(1, 2);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex);

    shader_use(SHADER_GUI);
    glUniform1i(shader_get_uniform_location(SHADER_GUI, "Texture"), 0);
    shader_use(SHADER_GAME);
    glUniform1i(shader_get_uniform_location(SHADER_GAME, "texture"), 1);
}

void texture_destroy(void)
{
    for (i32 i = 0; i < NUM_TEXTURE_UNITS; i++) {
        glDeleteTextures(1, &texture_units[i].id);
        free(texture_units[i].coords);
    }
}

void texture_get_info(Texture texture, u32* location, f32* u1, f32* v1, f32* u2, f32* v2)
{
    u8 idx1, idx2;
    idx1 = textures[texture].idx1;
    idx2 = textures[texture].idx2;
    
    *location = texture_units[idx1].location;
    *u1 = texture_units[idx1].coords[idx2].u1 / 1024.0;
    *v1 = texture_units[idx1].coords[idx2].v1 / 1024.0;
    *u2 = texture_units[idx1].coords[idx2].u2 / 1024.0;
    *v2 = texture_units[idx1].coords[idx2].v2 / 1024.0;
}

u32 texture_location(Texture texture)
{
    return texture_units[textures[texture].idx1].location;
}