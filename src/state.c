#include "state.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stb_truetype.h>

void state_init(void)
{
    window_init();
    renderer_init();

    long size;
    unsigned char* font_buffer;
    unsigned char* font_buffer2;

    FILE* font_file = fopen("assets/cmunrm.ttf", "rb");
    fseek(font_file, 0, SEEK_END);
    size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);
    font_buffer = malloc(size);
    fread(font_buffer, size, 1, font_file);
    fclose(font_file);

    FILE* font_file2 = fopen("assets/cmunrm.ttf", "rb");
    fseek(font_file2, 0, SEEK_END);
    size = ftell(font_file2);
    fseek(font_file2, 0, SEEK_SET);
    font_buffer2 = malloc(size);
    fread(font_buffer2, size, 1, font_file);
    fclose(font_file2);

    unsigned char temp_bitmap[512*512];
    stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
    GLuint ftex;

    stbtt_pack_context spc;
    stbtt_PackBegin(&spc, temp_bitmap, 512, 512, 0, 1, NULL);
    //stbtt_PackSetOversampling();

    stbtt_packedchar packedChars[96];
    stbtt_packedchar packedChars2[96];

    stbtt_pack_range fontRange[2];
    fontRange[0].font_size = 32.0f;         
    fontRange[0].first_unicode_codepoint_in_range = 32; 
    fontRange[0].array_of_unicode_codepoints = NULL;
    fontRange[0].num_chars = 96;       
    fontRange[0].chardata_for_range = packedChars;
    fontRange[1].font_size = 16.0f;         
    fontRange[1].first_unicode_codepoint_in_range = 32; 
    fontRange[1].array_of_unicode_codepoints = NULL;
    fontRange[1].num_chars = 96;       
    fontRange[1].chardata_for_range = packedChars2;

    stbtt_PackFontRanges(&spc, font_buffer, 0, fontRange, 2);
    stbtt_PackEnd(&spc);
    //stbtt_GetPackedQuad();

    //stbtt_BakeFontBitmap(font_buffer, 0, 64.0, temp_bitmap, 512, 512, 32, 96, cdata);
    free(font_buffer);
    free(font_buffer2);
    glGenTextures(1, &ftex);
    glBindTexture(GL_TEXTURE_2D, ftex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512,512, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
    // can free temp_bitmap at this point
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    float x, y;
    stbtt_aligned_quad q;
    stbtt_GetPackedQuad(packedChars, 512,512, 'e'-32, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9

    /* float points[] = {
        -1.0, -1.0,  q.s0,q.t1,
        -1.0,  1.0,  q.s0,q.t0,
         1.0,  1.0,  q.s1,q.t0,
         1.0, -1.0,  q.s1,q.t1,
    }; */
    float points[] = {
        -1.0, -1.0,  0, 1,
        -1.0,  1.0,  0, 0,
         1.0,  1.0,  1, 0,
         1.0, -1.0,  1, 1
    };

    int idxs[] = { 0, 1, 2, 0, 2, 3 };

    renderer_malloc(VAO_DEFAULT, sizeof(points) / sizeof(float), sizeof(idxs) / sizeof(int));
    renderer_update(VAO_DEFAULT, 0, sizeof(points) / sizeof(float), points, 0, sizeof(idxs) / sizeof(int), idxs);
}

void state_loop(void)
{
    while (!window_closed()) 
    {
        window_update();
        renderer_render();
    }
}

void state_destroy(void)
{
    window_destroy();
    renderer_destroy();
}