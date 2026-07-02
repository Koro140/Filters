#include <stdio.h>
#include <stdlib.h>

#include <glad/glad.h>
#include <SDL3/SDL.h>

#include "player.h"
#include "video_renderer.h"
#include "texture.h"
#include "shader.h"

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

    Frame_Queue fq;
    frame_queue_init(&fq);

    Player p = {0};
    player_init(&p, &fq,"/home/koro/Assets/video.mp4");
    if (p.video_decoder == NULL || p.video_decoder->pix_fmt != AV_PIX_FMT_YUV420P) {
        fprintf(stderr, "ERROR::VIDEO::Unsupported pixel format for display\n");
        player_destroy(&p);
        frame_queue_destroy(&fq);
        SDL_GL_DestroyContext(sdl_gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    video_renderer_init(p.video_decoder->width, p.video_decoder->height);

    pthread_t player_thread;
    player_thread_run(&p);

    uint64_t playback_start = SDL_GetPerformanceCounter();
    double freq = (double)SDL_GetPerformanceFrequency();

    unsigned int y_tex = get_y_tex();
    unsigned int u_tex = get_u_tex();
    unsigned int v_tex = get_v_tex();
    
    AVFrame* f = NULL;
    while (appRunning) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_EVENT_QUIT:
                appRunning = false;
                break;
            }
        }

        double elapsed = (double)(SDL_GetPerformanceCounter() - playback_start) / freq;
        
        if (f == NULL) {
            f = frame_queue_try_pop(&fq);
        }
        
        if (f != NULL) {
            double pts = f->best_effort_timestamp * p.video_timebase;

            if (pts <= elapsed) {
                video_renderer_draw(f);
                av_frame_free(&f);
            }
        }
        SDL_GL_SwapWindow(window);
    }

    player_thread_stop(&p);
    if (f != NULL) {
        av_frame_free(&f);
    }
    
    video_renderer_destroy();
    player_destroy(&p);
    frame_queue_destroy(&fq);
    
    SDL_GL_DestroyContext(sdl_gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}