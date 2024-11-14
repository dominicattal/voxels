#include "font.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <glad.h>

Font font;

void font_init(void)
{
    long size;
    unsigned char* font_buffer;

    FILE* font_file = fopen("assets/cmunrm.ttf", "rb");
    fseek(font_file, 0, SEEK_END);
    size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);
    font_buffer = malloc(size);
    fread(font_buffer, size, 1, font_file);
    fclose(font_file);

    GLuint ftex;
    
    stbtt_InitFont(&font.info, font_buffer, 0);

    stbtt_PackBegin(&font.spc, font.bitmap, 512, 512, 0, 1, NULL);

    font.fontRange.font_size = 32.0f;         
    font.fontRange.first_unicode_codepoint_in_range = 32; 
    font.fontRange.array_of_unicode_codepoints = NULL;
    font.fontRange.num_chars = 96;       
    font.fontRange.chardata_for_range = font.packedChars;

    stbtt_PackFontRanges(&font.spc, font_buffer, 0, &font.fontRange, 1);
    stbtt_PackEnd(&font.spc);
    free(font_buffer);

    glGenTextures(1, &ftex);
    glBindTexture(GL_TEXTURE_2D, ftex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512,512, 0, GL_RED, GL_UNSIGNED_BYTE, font.bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void font_destroy(void)
{

}
