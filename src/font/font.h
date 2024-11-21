#ifndef FONT_H
#define FONT_H

#include <stb_rect_pack.h>
#include <stb_truetype.h>

#define BITMAP_WIDTH    512
#define BITMAP_HEIGHT   512
#define CHAR_OFFSET     32
#define NUM_CHARS       96

typedef struct {
    stbtt_fontinfo info;
    stbtt_pack_context spc;
    stbtt_pack_range fontRange;
    stbtt_packedchar packedChars[NUM_CHARS];
    unsigned char* bitmap;
    unsigned char* font_buffer;
} Font;

extern Font font;

void font_init(void);
void font_destroy(void);

#endif