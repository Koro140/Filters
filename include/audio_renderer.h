#pragma once

#include <SDL3/SDL_audio.h>
#include <libavcodec/codec.h>

void audio_renderer_init(int channels, int frequency);
void audio_renderer_update(AVFrame* f);
void audio_renderer_destroy();