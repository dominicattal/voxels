#include "audio.h"
#include <al.h>
#include <alc.h>
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_NUM_SOURCES 100
#define NUM_BUFFERS NUM_AUDIO

typedef struct {
    ALCdevice* device;
    ALCcontext* context;
    ALuint sources[MAX_NUM_SOURCES];
    ALuint buffers[NUM_BUFFERS];
    ALuint num_sources;
    pthread_t thread_id;
    bool kill_thread;
    sem_t mutex;
} Audio;

static Audio audio;

static void checkError(const char* msg) {
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        fprintf(stderr, "OpenAL Error: %s\n", msg);
        fprintf(stderr, "Error code: %X\n", error);
        exit(1);
    }
}

static void *audio_update(void *vargp)
{
    while (!audio.kill_thread) {
        sem_wait(&audio.mutex);
        for (ALuint i = 0; i < audio.num_sources; i++) {
            ALint source_state, source, tmp;
            source = audio.sources[i];
            alGetSourcei(source, AL_SOURCE_STATE, &source_state);
            if (source_state == AL_STOPPED) {
                tmp = audio.sources[i];
                audio.sources[i] = audio.sources[audio.num_sources-1];
                audio.sources[audio.num_sources-1] = tmp;
                audio.num_sources--;
                i--;
            }
        }
        sem_post(&audio.mutex);
    }
}

static void load_sounds(void);

void audio_init() {
    audio.device = alcOpenDevice(NULL);
    if (!audio.device) {
        printf("Failed to open OpenAL device.\n");
        exit(1);
    }
    audio.context = alcCreateContext(audio.device, NULL);
    if (!audio.context) {
        printf("Failed to create OpenAL context.\n");
        alcCloseDevice(audio.device);
        exit(1);
    }
    alcMakeContextCurrent(audio.context);

    alGenBuffers(NUM_BUFFERS, audio.buffers);
    checkError("Failed to generate buffers.");

    alGenSources(MAX_NUM_SOURCES, audio.sources);
    checkError("Failed to generate sources.");
    audio.num_sources = 0;

    load_sounds();

    audio.kill_thread = FALSE;
    sem_init(&audio.mutex, 0, 1);
    pthread_create(&audio.thread_id, NULL, audio_update, NULL);
}

void audio_play_sound(AudioID id)
{
    if (audio.num_sources >= MAX_NUM_SOURCES)
        return;
    ALuint source = audio.sources[audio.num_sources];
    sem_wait(&audio.mutex);
    alSourcei(source, AL_BUFFER, audio.buffers[id]);
    alSourcePlay(source);
    checkError("Failed to play source.");
    audio.num_sources++;
    sem_post(&audio.mutex);
}

void audio_destroy()
{
    audio.kill_thread = TRUE;
    pthread_join(audio.thread_id, NULL);
    sem_destroy(&audio.mutex);

    for (ALint i = 0; i < MAX_NUM_SOURCES; i++) {
        alSourceStop(audio.sources[i]);
        checkError("Failed to stop source.");
    }

    alDeleteSources(MAX_NUM_SOURCES, audio.sources);
    checkError("Failed to delete source.");

    alDeleteBuffers(1, audio.buffers);
    checkError("Failed to delete buffers.");

    alcMakeContextCurrent(NULL);
    alcDestroyContext(audio.context);
    alcCloseDevice(audio.device);
}

static int load_sound(ALuint id, char *path)
{
    SNDFILE* file;
    SF_INFO sfinfo;
    ALenum format;
    ALshort* samples;
    ALsizei numSamples;
    ALsizei size;
    ALsizei freq;
    
    file = sf_open(path, SFM_READ, &sfinfo);
    if (!file) {
        fprintf(stderr, "Failed to open sound file: %s.\n", path);
        exit(1);
    }

    if (sfinfo.channels == 1) {
        format = AL_FORMAT_MONO16;
    } else if (sfinfo.channels == 2) {
        format = AL_FORMAT_STEREO16;
    } else {
        fprintf(stderr, "Unsupported number of channels: %s.\n", path);
        sf_close(file);
        exit(1);
    }

    size = sfinfo.frames * sfinfo.channels * sizeof(ALshort);
    samples = malloc(size);
    if (!samples) {
        fprintf(stderr, "Failed to allocate memory for audio samples: %s.\n", path);
        sf_close(file);
        exit(1);
    }

    numSamples = sf_read_short(file, samples, sfinfo.frames * sfinfo.channels);
    if (numSamples != sfinfo.frames * sfinfo.channels) {
        fprintf(stderr, "Failed to read all samples from file: %s.\n", path);
        free(samples);
        sf_close(file);
        exit(1);
    }
    freq = sfinfo.samplerate;
    sf_close(file);
    
    alBufferData(audio.buffers[id], format, samples, size, freq);
    checkError("Failed to fill buffer with data.");

    free(samples);
}

static void load_sounds(void)
{
    load_sound(AUD_DEFAULT, "assets/audio/gui_click.wav");
    load_sound(AUD_HIT, "assets/audio/hit.wav");
    load_sound(AUD_TEST, "assets/audio/test.wav");
}