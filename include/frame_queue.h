// Multithreaded frame queue .. it just keeps the pointers you have to manually allocate / free them
#pragma once

#include <libavutil/frame.h>
#include <stdbool.h>

#define FRAME_QUEUE_COUNT 5

#ifdef _WIN32
#include <windows.h>

typedef struct Frame_Queue
{
    AVFrame* frames[FRAME_QUEUE_COUNT];

    int head;
    int tail;
    int count;

    CRITICAL_SECTION mutex;
    CONDITION_VARIABLE not_empty;
    CONDITION_VARIABLE not_full;

    bool mutex_initialized;
    bool not_empty_initialized;
    bool not_full_initialized;
    
    volatile LONG aborted;
} Frame_Queue;

#else

#include <stdatomic.h>
#include <pthread.h>

typedef struct Frame_Queue {
    AVFrame* frames[FRAME_QUEUE_COUNT];
    int head;
    int tail;
    int count;

    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;

    bool mutex_initialized;
    bool not_empty_initialized;
    bool not_full_initialized;

    atomic_bool aborted;
} Frame_Queue;

#endif // _WIN32

void frame_queue_init(Frame_Queue* fq);
void frame_queue_destroy(Frame_Queue* fq);
void frame_queue_push(Frame_Queue* fq, AVFrame* frame);
AVFrame* frame_queue_try_pop(Frame_Queue* fq);

void frame_queue_abort(Frame_Queue* fq);