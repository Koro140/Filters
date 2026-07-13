#include "audio_renderer.h"

#include <SDL3/SDL_audio.h>
#include <stdio.h>
#include <libswresample/swresample.h>

SDL_AudioStream* audio_stream = NULL;

void audio_renderer_init(int channels, int frequency)
{
    SDL_AudioSpec spec = {
        .channels = channels,
        .freq = frequency,
        .format = SDL_AUDIO_S16
    };

    audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    if (audio_stream == NULL) {
        fprintf(stderr, "ERROR::SDL::%s\n", SDL_GetError());
        return;
    }

    SDL_ResumeAudioStreamDevice(audio_stream);
}

void audio_renderer_update(AVFrame* f) {
    int size = av_samples_get_buffer_size(
        NULL,
        f->ch_layout.nb_channels,
        f->nb_samples,
        AV_SAMPLE_FMT_S16,
        1
    );
    SDL_PutAudioStreamData(audio_stream, f->data[0], size);
}

void audio_renderer_destroy() {
    if (audio_stream != NULL) {
        SDL_DestroyAudioStream(audio_stream);
    }
}