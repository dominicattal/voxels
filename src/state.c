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

    FILE* font_file = fopen("assets/cmunrm.ttf", "rb");
    fseek(font_file, 0, SEEK_END);
    size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);
    font_buffer = malloc(size);
    fread(font_buffer, size, 1, font_file);
    fclose(font_file);

    unsigned char temp_bitmap[512*512];
    stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
    GLuint ftex;

    stbtt_BakeFontBitmap(font_buffer, 0, 64.0, temp_bitmap, 512, 512, 32, 96, cdata);
    free(font_buffer);
    glGenTextures(1, &ftex);
    glBindTexture(GL_TEXTURE_2D, ftex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512,512, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
    // can free temp_bitmap at this point
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    float x, y;
    stbtt_aligned_quad q;
    stbtt_GetBakedQuad(cdata, 512,512, 'c'-32, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9

    float points[] = {
        -1.0, -1.0,  q.s0,q.t1,
        -1.0,  1.0,  q.s0,q.t0,
         1.0,  1.0,  q.s1,q.t0,
         1.0, -1.0,  q.s1,q.t1,
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