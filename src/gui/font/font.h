#ifndef FONT_H
#define FONT_H

#include <stb_rect_pack.h>
#include <stb_truetype.h>

typedef struct {
    stbtt_fontinfo info;
    stbtt_pack_context spc;
    stbtt_pack_range fontRange;
    stbtt_packedchar packedChars[96];
    unsigned char* bitmap;
    unsigned char* font_buffer;
} Font;

extern Font font;

void font_init(void);
void font_destroy(void);

#endif