#include "texture.h"
#include "../font/font.h"
#include "shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <stb_rect_pack.h>

#define NUM_TEXTURE_UNITS 3
#define NUM_IMAGES_TO_PACK 3

typedef struct {
    Texture tex;
    char* path;
} Image;

static Image images[NUM_IMAGES_TO_PACK] = {
    (Image) { TEX_OBJECT1, "assets/textures/objects/object1.png" },
    (Image) { TEX_OBJECT2, "assets/textures/objects/object2.png" },
    (Image) { TEX_OBJECT3, "assets/textures/objects/object3.png" }
};

typedef struct {
    u16 u1, v1, u2, v2;
} UV;

#define UVINIT(u, v, w, h) (UV) { u, v, u+w, v+h }

static struct {
    UV* coords;
    u32 id;
} texture_units[NUM_TEXTURE_UNITS];

typedef struct {
    u8 idx1; // location
    u8 idx2; // uv idx
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
    i32 width1, height1;
    unsigned char* bitmap1 = font_bitmap(&width1, &height1);
    texture_units[0].id = texture_create_from_pixels(GL_RED, width1, height1, bitmap1);
    free(bitmap1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_units[0].id);

    u32 tex;
    tex = texture_create_from_path("assets/textures/objects/object_atlas.png");
    texture_units[1].id = tex;
    texture_units[1].coords = malloc(3 * sizeof(u64));
    texture_units[1].coords[0] = UVINIT(0, 0, 16, 16);
    texture_units[1].coords[1] = UVINIT(17, 0, 16, 16);
    texture_units[1].coords[2] = UVINIT(34, 0, 16, 16);
    textures[TEX_OBJECT1] = IDXINIT(1, 0);
    textures[TEX_OBJECT2] = IDXINIT(1, 1);
    textures[TEX_OBJECT3] = IDXINIT(1, 2);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex);

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
        }
    }

    context_rgb  = malloc(sizeof(stbrp_context));
    context_rgba = malloc(sizeof(stbrp_context));
    nodes_rgb    = malloc(sizeof(stbrp_node) * num_nodes);
    nodes_rgba   = malloc(sizeof(stbrp_node) * num_nodes);

    stbrp_init_target(context_rgb, bitmap_width, bitmap_height, nodes_rgb, num_nodes);
    stbrp_pack_rects(context_rgb, rects_rgb, num_rects_rgb);

    num_channels = 3;
    bitmap = calloc(bitmap_height * bitmap_width * num_channels, sizeof(unsigned char));
    for (i32 i = 0; i < num_rects; i++) {
        if (!rects_rgb[i].was_packed) {
            printf("Failed to pack image %d: %s\n", images[i].tex, images[i].path);
            continue;
        }
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
    u8 idx1, idx2;
    idx1 = textures[texture].idx1;
    idx2 = textures[texture].idx2;
    
    *location = 0;
    *u1 = texture_units[idx1].coords[idx2].u1 / 1024.0;
    *v1 = texture_units[idx1].coords[idx2].v1 / 1024.0;
    *u2 = texture_units[idx1].coords[idx2].u2 / 1024.0;
    *v2 = texture_units[idx1].coords[idx2].v2 / 1024.0;
}