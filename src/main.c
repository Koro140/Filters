#include <stdio.h>
#include <stdlib.h>

#include <glad/glad.h>
#include <SDL3/SDL.h>

#include "settings.h"
#include "player.h"
#include "video_renderer.h"
#include "audio_renderer.h"


typedef struct App {
    Frame_Queue video_frame_queue;
    Frame_Queue audio_frame_queue;
    Player p;
    SDL_Window *window;
    SDL_GLContext sdl_gl_context;
}App;

// global app state
App g_app = {0};

void window_resize_callback(SDL_Window* window) {
    int w, h;
    SDL_GetWindowSizeInPixels(window, &w, &h);
    glViewport(0, 0, w, h);
}

void app_initialize(int argc, char** argv);
void app_run();
void app_abort(int status);

void app_initialize(int argc, char** argv) {

    Settings settings = {0};
    settings_get(&settings, argc, argv);
    
    frame_queue_init(&g_app.video_frame_queue);

    frame_queue_init(&g_app.audio_frame_queue);
    
    if (player_init(&g_app.p, &g_app.video_frame_queue, &g_app.audio_frame_queue, settings.video_name) == false) {
        app_abort(1);
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == false) {
        fprintf(stderr, "ERROR::SDL::Initialization failed ... %s\n", SDL_GetError());
        app_abort(1);
    }

    g_app.window = SDL_CreateWindow("Filters", 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (g_app.window == NULL) {
        fprintf(stderr, "ERROR::SDL::%s\n", SDL_GetError());
        app_abort(1);
    }

    g_app.sdl_gl_context = SDL_GL_CreateContext(g_app.window);
    if (g_app.sdl_gl_context == NULL) {
        fprintf(stderr, "ERROR::SDL::%s\n", SDL_GetError());
        app_abort(1);
    }
    
    SDL_GL_MakeCurrent(g_app.window, g_app.sdl_gl_context);
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        fprintf(stderr, "ERROR::GLLoad::Failed to load GL functions\n");
        app_abort(1);
    }

    if (SDL_GL_SetSwapInterval(1) == false) {
        fprintf(stderr, "ERROR::SDL_GL::Failed to enable vsync\n");
    }
    window_resize_callback(g_app.window);

    if (g_app.p.video_decoder == NULL || g_app.p.video_decoder->pix_fmt != AV_PIX_FMT_YUV420P) {
        fprintf(stderr, "ERROR::VIDEO::Unsupported pixel format for display\n");
        app_abort(1);
        return;
    }
    
    video_renderer_init(g_app.window, &settings, g_app.p.video_decoder->width, g_app.p.video_decoder->height);
    audio_renderer_init(g_app.p.audio_decoder->ch_layout.nb_channels, g_app.p.audio_decoder->sample_rate);

    // freeing settings at the end of initialization
    settings_free(&settings);
}

void app_run() {
    player_thread_run(&g_app.p);
    
    AVFrame* vid_frame = NULL;
    AVFrame* audio_frame = NULL;

    bool appRunning = true;
    SDL_Event e;

    while (appRunning) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_EVENT_QUIT:
                appRunning = false;
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                window_resize_callback(g_app.window);
                break;
            }
        }

        // Video processing
        if (vid_frame == NULL) {
            vid_frame = frame_queue_try_pop(&g_app.video_frame_queue);
        }

        AVFrame* frame_to_display = NULL;
        double audio_clock= audio_renderer_get_clock();
        
        while (vid_frame != NULL) {
            double pts = vid_frame->best_effort_timestamp * g_app.p.video_timebase;
            if (pts > audio_clock) {
                break;
            }

            if (frame_to_display != NULL) {
                av_frame_free(&frame_to_display);
            }
            
            frame_to_display = vid_frame;

            vid_frame = frame_queue_try_pop(&g_app.video_frame_queue);
        }
        
        if (frame_to_display != NULL) {
            video_renderer_process_frame(frame_to_display);
            av_frame_free(&frame_to_display);
        }
        
        // Audio processing        
        while (audio_renderer_get_queued_seconds() < 0.5)  {
            audio_frame = frame_queue_try_pop(&g_app.audio_frame_queue);

            if (audio_frame == NULL) {
                break;
            }
            double pts = audio_frame->best_effort_timestamp * g_app.p.audio_timebase;
            
            audio_renderer_update(audio_frame, pts);
            av_frame_free(&audio_frame);
        }
        
        video_renderer_present();
        SDL_GL_SwapWindow(g_app.window);
    }

    // freeing frames when the loop done
    if (vid_frame != NULL) { av_frame_free(&vid_frame); }
    if (audio_frame != NULL) { av_frame_free(&audio_frame); }

    player_thread_stop(&g_app.p);

    app_abort(0);
}

void app_abort(int status) {
    video_renderer_destroy();
    audio_renderer_destroy();
    player_destroy(&g_app.p);
    frame_queue_destroy(&g_app.video_frame_queue);
    frame_queue_destroy(&g_app.audio_frame_queue);

    if (g_app.sdl_gl_context != 0) {
        SDL_GL_DestroyContext(g_app.sdl_gl_context);
    }
    
    if (g_app.window != NULL) {
        SDL_DestroyWindow(g_app.window);
    }
    
    SDL_Quit();

    exit(status);
}

int main(int argc, char **argv)
{
    app_initialize(argc, argv);
    app_run();
}