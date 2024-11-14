#ifndef FONT_H
#define FONT_H

#include <stb_rect_pack.h>
#include <stb_truetype.h>

typedef struct {
    stbtt_pack_context spc;
    stbtt_pack_range fontRange;
    stbtt_packedchar packedChars[96];
    unsigned char bitmap[512*512];
} Font;

extern Font font;

void font_init(void);
void font_destroy(void);

#endif