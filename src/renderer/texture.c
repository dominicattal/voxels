#include "texture.h"
#include "../font/font.h"
#include "shader.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <stb_rect_pack.h>

#define NUM_TEXTURE_UNITS  3
#define NUM_IMAGES_TO_PACK 3

typedef struct {
    Texture tex;
    char* path;
} Image;

typedef struct {
    u16 u1, v1, u2, v2;
} UV;

typedef struct {
    i8 location;
    i8 uv_idx;
} TEX;

static Image images[NUM_IMAGES_TO_PACK] = {
    (Image) { TEX_OBJECT1, "assets/textures/objects/object1.png" },
    (Image) { TEX_OBJECT2, "assets/textures/objects/object2.png" },
    (Image) { TEX_OBJECT3, "assets/textures/objects/object3.png" }
};

static TEX textures[NUM_TEXTURES];

static struct {
    UV* coords;
    u32 id;
} texture_units[NUM_TEXTURE_UNITS];

#define UVINIT(u, v, w, h) (UV) { u, v, u+w, v+h }
#define TEXINIT(location, uv_idx) (IDX) { location, uv_idx }

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
    i32 width1, height1;
    unsigned char* bitmap1 = font_bitmap(&width1, &height1);
    texture_units[0].id = texture_create_from_pixels(GL_RED, width1, height1, bitmap1);
    free(bitmap1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_units[0].id);

    i32 texs[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    shader_use(SHADER_GUI);
    glUniform1iv(shader_get_uniform_location(SHADER_GUI, "textures"), 2, texs);
    shader_use(SHADER_GAME);
    glUniform1iv(shader_get_uniform_location(SHADER_GAME, "textures"), 2, texs);

    i32 padding = 1;
    i32 bitmap_width = 1024;
    i32 bitmap_height = 1024;
    i32 width, height, num_channels;
    i32 num_rects_rgb, num_rects_rgba;
    unsigned char** image_data;
    unsigned char* bitmap;
    i32 num_nodes = bitmap_width;
    i32 num_rects = NUM_IMAGES_TO_PACK;
    i32 num_rects_packed;
    i32 location;
    i32 uv_idx;
    u32 tex;
    stbrp_context* context_rgb;
    stbrp_context* context_rgba;
    stbrp_node* nodes_rgb;
    stbrp_node* nodes_rgba;
    stbrp_rect* rects_rgb;
    stbrp_rect* rects_rgba;

    rects_rgb  = malloc(sizeof(stbrp_rect) * NUM_IMAGES_TO_PACK);
    rects_rgba = malloc(sizeof(stbrp_rect) * NUM_IMAGES_TO_PACK);
    image_data = malloc(sizeof(unsigned char*) * NUM_IMAGES_TO_PACK);
    num_rects_rgb = num_rects_rgba = 0;

    for (i32 i = 0; i < NUM_IMAGES_TO_PACK; i++) {
        image_data[i] = stbi_load(images[i].path, &width, &height, &num_channels, 0);
        if (num_channels == 3) {
            rects_rgb[num_rects_rgb].id = images[i].tex;
            rects_rgb[num_rects_rgb].w  = padding + width;
            rects_rgb[num_rects_rgb].h  = padding + height;
            ++num_rects_rgb;
        } else if (num_channels == 4) {
            rects_rgba[num_rects_rgba].id = images[i].tex;
            rects_rgba[num_rects_rgba].w  = padding + width;
            rects_rgba[num_rects_rgba].h  = padding + height;
            ++num_rects_rgba;
        } else {
            printf("Unsupported number of channels (%d) for image %d: %s\n", num_channels, images[i].tex, images[i].path);
            stbi_image_free(image_data[i]);
        }
    }

    context_rgb  = malloc(sizeof(stbrp_context));
    context_rgba = malloc(sizeof(stbrp_context));
    nodes_rgb    = malloc(sizeof(stbrp_node) * num_nodes);
    nodes_rgba   = malloc(sizeof(stbrp_node) * num_nodes);

    stbrp_init_target(context_rgb, bitmap_width, bitmap_height, nodes_rgb, num_nodes);
    stbrp_pack_rects(context_rgb, rects_rgb, num_rects_rgb);

    num_channels = 3;
    num_rects_packed = 0;
    location = 1;
    bitmap = calloc(bitmap_height * bitmap_width * num_channels, sizeof(unsigned char));
    for (i32 i = 0; i < num_rects; ++i) {
        if (!rects_rgb[i].was_packed) {
            printf("Failed to pack image %d: %s\n", images[i].tex, images[i].path);
            continue;
        }
        ++num_rects_packed;
        // height, width, channels, index in data, index in bitmap
        i32 y, x, c, data_idx, bitmap_idx;
        for (y = 0; y < rects_rgb[i].h - padding; ++y) {
            for (x = 0; x < rects_rgb[i].w - padding; ++x) {
                for (c = 0; c < num_channels; ++c) {

                    data_idx =   y * num_channels * (rects_rgb[i].w - padding)
                               + x * num_channels 
                               + c;

                    bitmap_idx =   (y + rects_rgb[i].y) * num_channels * bitmap_width
                                 + (x + rects_rgb[i].x) * num_channels
                                 +  c;

                    bitmap[bitmap_idx] = image_data[i][data_idx];
                }
            }
        }
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &tex);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(tex, 1, GL_RGB8, bitmap_width, bitmap_height);
    glTextureSubImage2D(tex, 0, 0, 0, bitmap_width, bitmap_height, GL_RGB, GL_UNSIGNED_BYTE, bitmap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex);

    texture_units[location].id = tex;
    texture_units[location].coords = malloc(sizeof(UV) * num_rects_packed);
    uv_idx = 0;
    for (i32 i = 0; i < num_rects; ++i) {
        stbrp_rect rect = rects_rgb[i];
        if (!rect.was_packed) {
            textures[rect.id].location = -1;
            textures[rect.id].uv_idx = -1;
            continue;
        }
        texture_units[location].coords[uv_idx] = UVINIT(rect.x, rect.y, rect.w-padding, rect.h-padding);
        textures[rect.id].location = location;
        textures[rect.id].uv_idx = uv_idx;
        ++uv_idx;
    }

    stbi_write_png("data/packed.png", bitmap_width, bitmap_height, num_channels, bitmap, 0);

    stbrp_init_target(context_rgba, bitmap_width, bitmap_height, nodes_rgba, num_nodes);
    stbrp_pack_rects(context_rgba, rects_rgba, num_rects_rgba);

    for (i32 i = 0; i < NUM_IMAGES_TO_PACK; i++)
        stbi_image_free(image_data[i]);

    free(image_data);
    free(bitmap);
    free(context_rgb);
    free(context_rgba);
    free(nodes_rgb);
    free(nodes_rgba);
    free(rects_rgb);
    free(rects_rgba);
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
    assert(textures[texture].location != -1);
    *location = textures[texture].location;
    *u1 = texture_units[textures[texture].location].coords[textures[texture].uv_idx].u1 / 1024.0;
    *v1 = texture_units[textures[texture].location].coords[textures[texture].uv_idx].v1 / 1024.0;
    *u2 = texture_units[textures[texture].location].coords[textures[texture].uv_idx].u2 / 1024.0;
    *v2 = texture_units[textures[texture].location].coords[textures[texture].uv_idx].v2 / 1024.0;
}