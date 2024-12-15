#ifndef AUDIO_H
#define AUDIO_H

#include "../util/util.h"

#define NUM_AUDIO 3

typedef enum {
    AUD_DEFAULT = 0,
    AUD_HIT = 1,
    AUD_TEST = 2
} AudioID;

void audio_init(void);
void audio_play_sound(AudioID id);
void audio_destroy(void);

#endif