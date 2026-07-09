#pragma once

#include "frame_queue.h"

void video_renderer_init(int width, int height);
void video_renderer_destroy();

void video_renderer_draw(AVFrame* frame);