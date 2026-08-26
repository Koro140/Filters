#pragma once

#include <SDL3/SDL_audio.h>
#include <libavcodec/codec.h>

void audio_renderer_init(int channels, int frequency);
void audio_renderer_update(AVFrame* f,double pts);
double audio_renderer_get_clock();
double audio_renderer_get_queued_seconds();
void audio_renderer_destroy();