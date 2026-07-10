#pragma once

#include <SDL3/SDL.h>

#include "frame_queue.h"
#include "settings.h"

void video_renderer_init(SDL_Window* window, Settings* settings,int width, int height);
void video_renderer_destroy();

void video_renderer_draw(AVFrame* frame);