#include "audio_renderer.h"

#include <SDL3/SDL_audio.h>
#include <stdio.h>
#include <libswresample/swresample.h>

typedef struct AudioRenderer {
    SDL_AudioStream* audio_stream;
    int channels;
    int frequency;
    double audio_end_pts;
}AudioRenderer;

AudioRenderer g_aud_renderer;

void audio_renderer_init(int channels, int frequency)
{
    g_aud_renderer = (AudioRenderer){0};

    g_aud_renderer.channels = channels;
    g_aud_renderer.frequency = frequency;

    SDL_AudioSpec spec = {
        .channels = channels,
        .freq = frequency,
        .format = SDL_AUDIO_S16
    };

    g_aud_renderer.audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    if (g_aud_renderer.audio_stream == NULL) {
        fprintf(stderr, "ERROR::SDL::%s\n", SDL_GetError());
        return;
    }

    SDL_ResumeAudioStreamDevice(g_aud_renderer.audio_stream);
}

void audio_renderer_update(AVFrame* f, double pts) {
    int size = av_samples_get_buffer_size(
        NULL,
        f->ch_layout.nb_channels,
        f->nb_samples,
        AV_SAMPLE_FMT_S16,
        1
    );
    SDL_PutAudioStreamData(g_aud_renderer.audio_stream, f->data[0], size);

    g_aud_renderer.audio_end_pts = pts + (double)f->nb_samples / f->sample_rate;
}

double audio_renderer_get_clock() {
    int queued = SDL_GetAudioStreamQueued(g_aud_renderer.audio_stream);

    double queued_seconds = (double)queued / (2.0 * g_aud_renderer.channels * g_aud_renderer.frequency);
    return g_aud_renderer.audio_end_pts - queued_seconds;
}

void audio_renderer_destroy() {
    if (g_aud_renderer.audio_stream != NULL) {
        SDL_DestroyAudioStream(g_aud_renderer.audio_stream);
        g_aud_renderer.audio_stream = NULL;
    }
}