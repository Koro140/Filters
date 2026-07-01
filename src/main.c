#include <stdio.h>
#include <stdlib.h>

#include <glad/glad.h>
#include <SDL3/SDL.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage : ./filters -v [video_name]\n");
        fprintf(stdout, "Use ./filters --help to learn more\n");
        return 1;
    }

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_Window *window = SDL_CreateWindow("Filters", 1280, 720, SDL_WINDOW_OPENGL);

    SDL_GLContext sdl_gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, sdl_gl_context);
    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
    
    bool appRunning = true;
    SDL_Event e;
    while (appRunning)
    {
        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
            case SDL_EVENT_QUIT:
                appRunning = false;
                break;
            }
        }

        glClearColor(0.1, 0.3, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DestroyContext(sdl_gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}