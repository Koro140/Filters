#pragma once

#include "frame_queue.h"

unsigned int get_y_tex();
unsigned int get_u_tex();
unsigned int get_v_tex();

void video_renderer_init(int width, int height);
void video_renderer_destroy();

void video_renderer_draw(AVFrame* frame);