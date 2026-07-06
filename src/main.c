#include <stdio.h>
#include <stdlib.h>

#include <glad/glad.h>
#include <SDL3/SDL.h>

#include "player.h"
#include "video_renderer.h"
#include "audio_renderer.h"

int main(int argc, char **argv)
{
    // if (argc != 2) {
    //     fprintf(stderr, "Usage : ./filters -v [video_name]\n");
    //     fprintf(stdout, "Use ./filters --help to learn more\n");
    //     return 1;
    // }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == false) {
        fprintf(stderr, "ERROR::SDL::Initialization failed ... %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Filters", 1280, 720, SDL_WINDOW_OPENGL);
    if (window == NULL) {
        fprintf(stderr, "ERROR::SDL::%s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GLContext sdl_gl_context = SDL_GL_CreateContext(window);
    if (sdl_gl_context == NULL) {
        fprintf(stderr, "ERROR::SDL::%s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    SDL_GL_MakeCurrent(window, sdl_gl_context);
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        fprintf(stderr, "ERROR::GLLoad::Failed to load GL functions\n");
        SDL_GL_DestroyContext(sdl_gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_GL_SetSwapInterval(1);
    
    bool appRunning = true;
    SDL_Event e;

    Frame_Queue video_frame_queue;
    frame_queue_init(&video_frame_queue);

    Frame_Queue audio_frame_queue;
    frame_queue_init(&audio_frame_queue);

    Player p = {0};
    player_init(&p, &video_frame_queue, &audio_frame_queue, "/home/koro/Assets/video.mp4");

    if (p.video_decoder == NULL || p.video_decoder->pix_fmt != AV_PIX_FMT_YUV420P) {
        fprintf(stderr, "ERROR::VIDEO::Unsupported pixel format for display\n");
        player_destroy(&p);
        frame_queue_destroy(&video_frame_queue);
        SDL_GL_DestroyContext(sdl_gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    video_renderer_init(p.video_decoder->width, p.video_decoder->height);
    audio_renderer_init(p.audio_decoder->ch_layout.nb_channels, p.audio_decoder->sample_rate);

    pthread_t player_thread;
    player_thread_run(&p);

    uint64_t playback_start = SDL_GetPerformanceCounter();
    double freq = (double)SDL_GetPerformanceFrequency();
    
    AVFrame* vid_frame = NULL;
    AVFrame* audio_frame = NULL;
    while (appRunning) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_EVENT_QUIT:
                appRunning = false;
                break;
            }
        }

        double elapsed = (double)(SDL_GetPerformanceCounter() - playback_start) / freq;
        
        if (vid_frame == NULL) {
            vid_frame = frame_queue_try_pop(&video_frame_queue);
        }
        
        if (audio_frame == NULL) {
            audio_frame = frame_queue_try_pop(&audio_frame_queue);
        }

        if (vid_frame != NULL) {
            double pts = vid_frame->best_effort_timestamp * p.video_timebase;

            if (pts <= elapsed) {
                video_renderer_draw(vid_frame);
                av_frame_free(&vid_frame);
            }
        }

        if (audio_frame != NULL) {
            double pts = audio_frame->best_effort_timestamp * p.audio_timebase;

            if (pts <= elapsed) {
                audio_renderer_update(audio_frame);
                av_frame_free(&audio_frame);
            }
        }

        SDL_GL_SwapWindow(window);
    }

    player_thread_stop(&p);
    if (vid_frame != NULL) {
        av_frame_free(&vid_frame);
    }
    
    video_renderer_destroy();
    audio_renderer_destroy();
    player_destroy(&p);
    frame_queue_destroy(&video_frame_queue);
    
    SDL_GL_DestroyContext(sdl_gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}