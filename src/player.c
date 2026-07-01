#include "player.h"

#include <stdio.h>

static void print_av_error(const char *context, int err) {
    char errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
    av_strerror(err, errbuf, sizeof(errbuf));
    fprintf(stderr, "%s: %s\n", context, errbuf);
}

void player_init(Player *player, Frame_Queue* queue, const char* url) {
    player->queue_referenece = queue;
    player->format_context = NULL;
    player->packet = av_packet_alloc();
    player->video_codec = NULL;
    player->audio_codec = NULL;
    player->video_decoder = NULL;
    player->audio_decoder = NULL;
    player->frame = NULL;
    player->audio_frame = NULL;
    player->resampled_frame = NULL;
    player->video_stream_idx = -1;
    player->audio_stream_idx = -1;
    
    atomic_store(&player->quit, false);

    int ret = avformat_open_input(&player->format_context, url, NULL, NULL);
    if (ret < 0) {
        print_av_error("avformat_open_input", ret);
        return;
    }

    ret = avformat_find_stream_info(player->format_context, NULL);
    if (ret < 0) {
        print_av_error("avformat_find_stream_info", ret);
        return;
    }

    // Initializing video stream/decoder
    player->video_stream_idx = av_find_best_stream(
        player->format_context, AVMEDIA_TYPE_VIDEO, -1, -1,
        &player->video_codec, 0
    );

    if (player->video_stream_idx >= 0 && player->video_codec != NULL) {
        player->video_decoder = avcodec_alloc_context3(player->video_codec);
        ret = avcodec_parameters_to_context(
            player->video_decoder,
            player->format_context->streams[player->video_stream_idx]->codecpar
        );
        if (ret >= 0) {
            ret = avcodec_open2(player->video_decoder, player->video_codec, NULL);
            if (ret < 0) {
                print_av_error("avcodec_open2(video)", ret);
            } else {
                player->last_fmt = player->video_decoder->pix_fmt;
            }
        } else {
            print_av_error("avcodec_parameters_to_context(video)", ret);
        }
    }

    player->frame = av_frame_alloc();

    // Initializing audio stream/decoder
    player->audio_stream_idx = av_find_best_stream(
        player->format_context, AVMEDIA_TYPE_AUDIO, -1, -1,
        &player->audio_codec, 0
    );

    if (player->audio_stream_idx >= 0 && player->audio_codec != NULL) {
        player->audio_decoder = avcodec_alloc_context3(player->audio_codec);
        ret = avcodec_parameters_to_context(
            player->audio_decoder,
            player->format_context->streams[player->audio_stream_idx]->codecpar
        );
        if (ret >= 0) {
            ret = avcodec_open2(player->audio_decoder, player->audio_codec, NULL);
            if (ret < 0) {
                print_av_error("avcodec_open2(audio)", ret);
            }
        } else {
            print_av_error("avcodec_parameters_to_context(audio)", ret);
        }
    }

    player->audio_frame = av_frame_alloc();
    player->resampled_frame = av_frame_alloc();

    player->video_timebase = av_q2d(player->format_context->streams[player->video_stream_idx]->time_base);
    player->audio_timebase = av_q2d(player->format_context->streams[player->audio_stream_idx]->time_base);
}

void player_destroy(Player* p) {
    av_packet_free(&p->packet);

    av_frame_free(&p->frame);

    av_frame_free(&p->audio_frame);
    av_frame_free(&p->resampled_frame);

    avcodec_free_context(&p->video_decoder);
    avcodec_free_context(&p->audio_decoder);

    avformat_close_input(&p->format_context);
    avformat_free_context(p->format_context);
}

static void player_update(Player* p) {
    if (p->format_context == NULL || p->packet == NULL) {
        atomic_store(&p->quit, true);
        return;
    }

    int ret = av_read_frame(p->format_context, p->packet);
    if (ret == AVERROR_EOF) {
        atomic_store(&p->quit, true);
        return;
    }
    else if (ret < 0) {
        print_av_error("av_read_frame", ret);
        atomic_store(&p->quit, true);
        return;
    }

    // Video packet
    if (p->packet->stream_index == p->video_stream_idx && p->video_decoder != NULL) {
        ret = avcodec_send_packet(p->video_decoder, p->packet);
        if (ret < 0) {
            print_av_error("avcodec_send_packet(video)", ret);
        } else {
            while ((ret = avcodec_receive_frame(p->video_decoder, p->frame)) == 0) {
                AVFrame* copy = av_frame_clone(p->frame);
                if (copy != NULL) {
                    frame_queue_push(p->queue_referenece, copy);
                }
                av_frame_unref(p->frame);
            }
            if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                print_av_error("avcodec_receive_frame(video)", ret);
            }
        }
    }

    // Audio packet
    if (p->packet->stream_index == p->audio_stream_idx && p->audio_decoder != NULL) {
        ret = avcodec_send_packet(p->audio_decoder, p->packet);
        if (ret < 0) {
            print_av_error("avcodec_send_packet(audio)", ret);
        } else {
            while ((ret = avcodec_receive_frame(p->audio_decoder, p->audio_frame)) == 0) {
                av_frame_unref(p->audio_frame);
            }
            if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                print_av_error("avcodec_receive_frame(audio)", ret);
            }
        }
    }

    av_packet_unref(p->packet);
}

static void *player_thread(void* arg) {
    Player* p = arg;
    while (!atomic_load(&p->quit))
    {
        player_update(p);
    }

    return NULL;
}

void player_thread_run(Player* p) {
    pthread_create(&p->thread, NULL, player_thread, p);
}

void player_thread_stop(Player* p) {
    atomic_store(&p->quit, true);
    frame_queue_abort(p->queue_referenece);
    pthread_join(p->thread, NULL);
}