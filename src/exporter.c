#include "exporter.h"
#include <string.h>

static void print_av_error(const char* context, int err) {
    char errbuf[AV_ERROR_MAX_STRING_SIZE] = { 0 };
    av_strerror(err, errbuf, sizeof(errbuf));
    fprintf(stderr, "%s: %s\n", context, errbuf);
}


bool exporter_init(Exporter* exporter, Player* player, char* out_name) {
    *exporter = (Exporter){ 0 };

    int ret = avformat_alloc_output_context2(&exporter->format_context, NULL, NULL, out_name);
    if (exporter->format_context == NULL) {
        fprintf(stderr, "ERROR::EXPORTER::Coudln't create output format context\n");
        return false;
    }

    const AVCodec* video_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (video_codec == NULL) {
        fprintf(stderr, "ERROR::EXPORTER::H264 encoder not found\n");
        return false;
    }

    const AVCodec* audio_codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (audio_codec == NULL) {
        fprintf(stderr, "ERROR::EXPORTER::AAC encoder not found\n");
        return false;
    }

    exporter->video_stream = avformat_new_stream(exporter->format_context, video_codec);
    exporter->audio_stream = avformat_new_stream(exporter->format_context, audio_codec);
    if (exporter->video_stream == NULL || exporter->audio_stream == NULL) {
        fprintf(stderr, "ERROR::EXPORTER::Couldn't create streams\n");
        return false;
    }

    exporter->video_codec_ctx = avcodec_alloc_context3(video_codec);
    exporter->audio_codec_ctx = avcodec_alloc_context3(audio_codec);

    // --- Video codec setup ---
    exporter->video_codec_ctx->width = player->video_decoder->width;
    exporter->video_codec_ctx->height = player->video_decoder->height;
    exporter->video_codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    AVRational fr = player->format_context->streams[player->video_stream_idx]->avg_frame_rate;
    int fps = fr.den ? (int)av_q2d(fr) : 30; // fallback if avg_frame_rate is unset

    exporter->video_codec_ctx->time_base = (AVRational){ 1, fps };
    exporter->video_codec_ctx->framerate = (AVRational){ fps, 1 };
    exporter->video_codec_ctx->gop_size = fps;
    exporter->video_codec_ctx->bit_rate = 8000000;

    if (exporter->format_context->oformat->flags & AVFMT_GLOBALHEADER) {
        exporter->video_codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    ret = avcodec_open2(exporter->video_codec_ctx, video_codec, NULL);
    if (ret < 0) {
        print_av_error("avcodec_open2(video export)", ret);
        return false;
    }
    avcodec_parameters_from_context(exporter->video_stream->codecpar, exporter->video_codec_ctx);
    exporter->video_stream->time_base = exporter->video_codec_ctx->time_base;

    // --- Audio codec setup (mirrors player's decoded format, not hardcoded) ---
    exporter->audio_codec_ctx->sample_rate = player->audio_decoder->sample_rate;
    av_channel_layout_default(&exporter->audio_codec_ctx->ch_layout,
        player->audio_decoder->ch_layout.nb_channels);
    exporter->audio_codec_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
    exporter->audio_codec_ctx->bit_rate = 192000;

    if (exporter->format_context->oformat->flags & AVFMT_GLOBALHEADER) {
        exporter->audio_codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    ret = avcodec_open2(exporter->audio_codec_ctx, audio_codec, NULL);
    if (ret < 0) {
        print_av_error("avcodec_open2(audio export)", ret);
        return false;
    }
    avcodec_parameters_from_context(exporter->audio_stream->codecpar, exporter->audio_codec_ctx);
    exporter->audio_stream->time_base = (AVRational){ 1, exporter->audio_codec_ctx->sample_rate };

    // --- Open output file & write header ---
    if (!(exporter->format_context->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&exporter->format_context->pb, out_name, AVIO_FLAG_WRITE);
        if (ret < 0) {
            print_av_error("avio_open", ret);
            return false;
        }
    }

    ret = avformat_write_header(exporter->format_context, NULL);
    if (ret < 0) {
        print_av_error("avformat_write_header", ret);
        return false;
    }

    // --- Video frame + sws context (RGB24 from renderer -> YUV420P for encoder) ---
    exporter->video_frame = av_frame_alloc();
    exporter->video_frame->format = AV_PIX_FMT_YUV420P;
    exporter->video_frame->width = exporter->video_codec_ctx->width;
    exporter->video_frame->height = exporter->video_codec_ctx->height;
    av_frame_get_buffer(exporter->video_frame, 32);

    exporter->sws_ctx = sws_getContext(
        exporter->video_codec_ctx->width, exporter->video_codec_ctx->height, AV_PIX_FMT_RGB24,
        exporter->video_codec_ctx->width, exporter->video_codec_ctx->height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, NULL, NULL, NULL
    );

    // --- Audio frame + swr context (player's S16 output -> FLTP for AAC) ---
    exporter->audio_frame_size = exporter->audio_codec_ctx->frame_size;
    exporter->audio_frame = av_frame_alloc();
    exporter->audio_frame->format = exporter->audio_codec_ctx->sample_fmt;
    exporter->audio_frame->ch_layout = exporter->audio_codec_ctx->ch_layout;
    exporter->audio_frame->sample_rate = exporter->audio_codec_ctx->sample_rate;
    exporter->audio_frame->nb_samples = exporter->audio_frame_size;
    av_frame_get_buffer(exporter->audio_frame, 0);

    AVChannelLayout src_layout;
    av_channel_layout_default(&src_layout, player->audio_decoder->ch_layout.nb_channels);

    swr_alloc_set_opts2(&exporter->swr_ctx,
        &exporter->audio_codec_ctx->ch_layout, AV_SAMPLE_FMT_FLTP, exporter->audio_codec_ctx->sample_rate,
        &src_layout, player->audio_decoder->sample_fmt, player->audio_decoder->sample_rate,
        0, NULL);

    swr_init(exporter->swr_ctx);

    exporter->video_pts = 0;
    exporter->audio_pts = 0;
    exporter->initialized = true;
    return true;
}



static void exporter_flush_video_packets(Exporter* exporter) {
    AVPacket* pkt = av_packet_alloc();
    while (avcodec_receive_packet(exporter->video_codec_ctx, pkt) == 0) {
        av_packet_rescale_ts(pkt, exporter->video_codec_ctx->time_base, exporter->video_stream->time_base);
        pkt->stream_index = exporter->video_stream->index;
        av_interleaved_write_frame(exporter->format_context, pkt);
        av_packet_unref(pkt);
    }
    av_packet_free(&pkt);
}

static void exporter_flush_audio_packets(Exporter* exporter) {
    AVPacket* pkt = av_packet_alloc();
    while (avcodec_receive_packet(exporter->audio_codec_ctx, pkt) == 0) {
        av_packet_rescale_ts(pkt, exporter->audio_codec_ctx->time_base, exporter->audio_stream->time_base);
        pkt->stream_index = exporter->audio_stream->index;
        av_interleaved_write_frame(exporter->format_context, pkt);
        av_packet_unref(pkt);
    }
    av_packet_free(&pkt);
}

void exporter_write_frame(Exporter* exporter, unsigned char* rgb_buffer) {
    int stride = exporter->video_codec_ctx->width * 3; // RGB24 — matches video_renderer_get_frame

    // Flip vertically: glReadPixels gives bottom-up rows, sws_scale wants top-down.
    // Feed it the last row first with a negative stride to flip during conversion.
    const uint8_t* src_data[1] = { rgb_buffer + (exporter->video_codec_ctx->height - 1) * stride };
    int      src_linesize[1] = { -stride };

    sws_scale(exporter->sws_ctx, src_data, src_linesize, 0, exporter->video_codec_ctx->height,
        exporter->video_frame->data, exporter->video_frame->linesize);

    exporter->video_frame->pts = exporter->video_pts++;

    int ret = avcodec_send_frame(exporter->video_codec_ctx, exporter->video_frame);
    if (ret < 0) {
        print_av_error("avcodec_send_frame(video)", ret);
        return;
    }

    exporter_flush_video_packets(exporter);
}

void exporter_write_audio(Exporter* exporter, const uint8_t** src_planes, int nb_samples) {
    const uint8_t** in_ptr = src_planes;
    int in_count = nb_samples;

    // Loop: each call may produce 0, 1, or multiple full AAC frames, since swr
    // buffers leftover input samples internally between calls.
    for (;;) {
        int out_samples = swr_convert(exporter->swr_ctx,
            exporter->audio_frame->data, exporter->audio_frame_size,
            in_ptr, in_count);

        if (out_samples < 0) {
            print_av_error("swr_convert", out_samples);
            return;
        }
        if (out_samples == 0) {
            break; // not enough buffered samples for a full frame yet
        }

        exporter->audio_frame->pts = exporter->audio_pts;
        exporter->audio_pts += out_samples;

        int ret = avcodec_send_frame(exporter->audio_codec_ctx, exporter->audio_frame);
        if (ret < 0) {
            print_av_error("avcodec_send_frame(audio)", ret);
        }
        else {
            exporter_flush_audio_packets(exporter);
        }

        // Subsequent iterations drain already-buffered samples — no new input
        in_ptr = NULL;
        in_count = 0;

        if (out_samples < exporter->audio_frame_size) {
            break; // buffer fully drained for now
        }
    }
}

void exporter_close(Exporter* exporter) {
    if (!exporter->initialized) {
        // Nothing safe to flush/write — just release whatever partial state exists.
        avcodec_free_context(&exporter->video_codec_ctx);
        avcodec_free_context(&exporter->audio_codec_ctx);
        av_frame_free(&exporter->video_frame);
        av_frame_free(&exporter->audio_frame);
        sws_freeContext(exporter->sws_ctx);
        swr_free(&exporter->swr_ctx);
        if (exporter->format_context != NULL) {
            if (exporter->format_context->pb != NULL &&
                !(exporter->format_context->oformat->flags & AVFMT_NOFILE)) {
                avio_closep(&exporter->format_context->pb);
            }
            avformat_free_context(exporter->format_context);
        }
        return;
    }

    for (;;) {
        int out_samples = swr_convert(exporter->swr_ctx,
            exporter->audio_frame->data, exporter->audio_frame_size,
            NULL, 0); // no new input — just drain what's buffered

        if (out_samples <= 0) break;

        exporter->audio_frame->pts = exporter->audio_pts;
        exporter->audio_pts += out_samples;

        if (avcodec_send_frame(exporter->audio_codec_ctx, exporter->audio_frame) == 0) {
            exporter_flush_audio_packets(exporter);
        }
    }

    // Flush encoders (send NULL to signal EOF, drain remaining packets)
    avcodec_send_frame(exporter->video_codec_ctx, NULL);
    exporter_flush_video_packets(exporter);

    avcodec_send_frame(exporter->audio_codec_ctx, NULL);
    exporter_flush_audio_packets(exporter);

    av_write_trailer(exporter->format_context);

    avcodec_free_context(&exporter->video_codec_ctx);
    avcodec_free_context(&exporter->audio_codec_ctx);
    av_frame_free(&exporter->video_frame);
    av_frame_free(&exporter->audio_frame);
    sws_freeContext(exporter->sws_ctx);
    swr_free(&exporter->swr_ctx);

    if (!(exporter->format_context->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&exporter->format_context->pb);
    }
    avformat_free_context(exporter->format_context);
}