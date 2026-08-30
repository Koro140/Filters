#pragma once

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

#include "player.h"

typedef struct Exporter {
	AVFormatContext *format_context;

    AVCodecContext* video_codec_ctx;
    AVStream* video_stream;
    AVFrame* video_frame;      // YUV420P frame fed to encoder
    AVFrame* video_rgba_frame; // Raw RGBA buffer
    struct SwsContext* sws_ctx;
    int width, height;
    int64_t video_frame_index;

    AVCodecContext* audio_codec_ctx;
    AVStream* audio_stream;
    AVFrame* audio_frame;
    SwrContext* swr_ctx;
    int audio_frame_size;

    int64_t video_pts;
    int64_t audio_pts;

    bool initialized;
}Exporter;

bool exporter_init(Exporter* exporter, Player* player, char* out_name);

static void exporter_flush_video_packets(Exporter* exporter);

static void exporter_flush_audio_packets(Exporter* exporter);

void exporter_write_frame(Exporter* exporter, unsigned char* rgb_buffer);

void exporter_write_audio(Exporter* exporter, const uint8_t** src_planes, int nb_samples);

void exporter_close(Exporter* exporter);