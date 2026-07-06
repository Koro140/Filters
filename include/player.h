#pragma once

#include <pthread.h>
#include <stdbool.h>
#include <stdatomic.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

#include "frame_queue.h"

typedef struct Player {
  AVFormatContext *format_context;
  AVPacket *packet;

  const AVCodec *video_codec;
  AVCodecContext *video_decoder;
  AVFrame *frame;
  AVStream *video_stream;
  enum AVPixelFormat last_fmt;

  const AVCodec *audio_codec;
  AVCodecContext *audio_decoder;
  AVFrame *audio_frame;
  AVFrame *resampled_frame;
  struct SwrContext *swr;
  
  int video_stream_idx;
  int audio_stream_idx;

  double video_timebase;
  double audio_timebase;

  Frame_Queue *video_queue_referenece;
  Frame_Queue *audio_queue_referenece;
  
  atomic_bool quit;
  pthread_t   thread;
}Player;

void player_init(Player *player, Frame_Queue* video_queue, Frame_Queue* audio_queue, const char *url);
void player_destroy(Player *player);

void player_thread_run(Player* p);
void player_thread_stop(Player* p);