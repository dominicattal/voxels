#include "font.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <glad.h>
#include <stb_image_write.h>
#include "../../renderer/renderer.h"

Font font;

void font_init(void)
{
    long size;

    font.bitmap = calloc(BITMAP_WIDTH * BITMAP_HEIGHT, sizeof(unsigned char));

    FILE* font_file = fopen("assets/times.ttf", "rb");
    fseek(font_file, 0, SEEK_END);
    size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);
    font.font_buffer = malloc(size);
    fread(font.font_buffer, size, 1, font_file);
    fclose(font_file);
    
    stbtt_InitFont(&font.info, font.font_buffer, 0);

    stbtt_PackBegin(&font.spc, font.bitmap, BITMAP_WIDTH, BITMAP_HEIGHT, 0, 1, NULL);

    font.fontRange.font_size = 32.0f;         
    font.fontRange.first_unicode_codepoint_in_range = CHAR_OFFSET; 
    font.fontRange.array_of_unicode_codepoints = NULL;
    font.fontRange.num_chars = NUM_CHARS;       
    font.fontRange.chardata_for_range = font.packedChars;

    stbtt_PackFontRanges(&font.spc, font.font_buffer, 0, &font.fontRange, 1);
    stbtt_PackEnd(&font.spc);

    /* GLuint ftex;
    glGenTextures(1, &ftex);
    glBindTexture(GL_TEXTURE_2D, ftex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, BITMAP_WIDTH, BITMAP_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, font.bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); */
    renderer.textures[FONT_BITMAP] = texture_create_pixels(BITMAP_WIDTH, BITMAP_HEIGHT, font.bitmap);

    stbi_write_png("data/out.png", BITMAP_WIDTH, BITMAP_HEIGHT, 1, font.bitmap, BITMAP_WIDTH);

}

void font_destroy(void)
{
    free(font.bitmap);
    free(font.font_buffer);
}
