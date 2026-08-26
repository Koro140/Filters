// Multithreaded frame queue .. it just keeps the pointers you have to manually allocate / free them
#pragma once

#include <libavutil/frame.h>
#include <SDL3/SDL.h>

#define FRAME_QUEUE_COUNT 5

typedef struct Frame_Queue {
    AVFrame* frames[FRAME_QUEUE_COUNT];
    int head;
    int tail;
    int count;

    SDL_Mutex* mutex;
    SDL_Condition* not_empty;
    SDL_Condition* not_full;

    bool mutex_initialized;
    bool not_empty_initialized;
    bool not_full_initialized;

    SDL_AtomicInt aborted;
} Frame_Queue;

void frame_queue_init(Frame_Queue* fq);
void frame_queue_destroy(Frame_Queue* fq);
void frame_queue_push(Frame_Queue* fq, AVFrame* frame);
AVFrame* frame_queue_try_pop(Frame_Queue* fq);

void frame_queue_abort(Frame_Queue* fq);