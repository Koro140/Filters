#pragma once

#include "frame_queue.h"
#include "settings.h"

void video_renderer_init(Settings* settings,int width, int height);
void video_renderer_destroy();

void video_renderer_draw(AVFrame* frame);