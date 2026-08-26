#include "frame_queue.h"

#include <errno.h>
#include <time.h>

void frame_queue_init(Frame_Queue* fq)
{
    fq->head = 0;
    fq->tail = 0;
    fq->count = 0;
    SDL_SetAtomicInt(&fq->aborted, 0);
    
    fq->mutex = SDL_CreateMutex();

    fq->mutex = SDL_CreateMutex();
    fq->mutex_initialized = (fq->mutex != NULL);

    fq->not_empty = SDL_CreateCondition();
    fq->not_empty_initialized = (fq->not_empty != NULL);

    fq->not_full = SDL_CreateCondition();
    fq->not_full_initialized = (fq->not_full != NULL);
}

void frame_queue_destroy(Frame_Queue* fq)
{
    if (fq->mutex_initialized) {
        SDL_LockMutex(fq->mutex);
    }

    while (fq->count > 0) {
        AVFrame* frame = fq->frames[fq->head];

        av_frame_free(&frame);

        fq->head = (fq->head + 1) % FRAME_QUEUE_COUNT;
        fq->count--;
    }

    if (fq->mutex_initialized) {
        SDL_UnlockMutex(fq->mutex);
        SDL_DestroyMutex(fq->mutex);
    }

    if (fq->not_empty_initialized) {
        SDL_DestroyCondition(fq->not_empty);
    }
    if (fq->not_full_initialized) {
        SDL_DestroyCondition(fq->not_full);
    }
}

void frame_queue_push(Frame_Queue* fq, AVFrame* frame)
{
    SDL_LockMutex(fq->mutex);

    while (fq->count == FRAME_QUEUE_COUNT && !SDL_GetAtomicInt(&fq->aborted))
        SDL_WaitCondition(fq->not_full, fq->mutex);

    if (SDL_GetAtomicInt(&fq->aborted)) {
        SDL_UnlockMutex(fq->mutex);
        av_frame_free(&frame);   // we own it, don't leak it on shutdown
        return;
    }

    fq->frames[fq->tail] = frame;
    fq->tail = (fq->tail + 1) % FRAME_QUEUE_COUNT;
    fq->count++;

    SDL_SignalCondition(fq->not_empty);

    SDL_UnlockMutex(fq->mutex);
}

AVFrame* frame_queue_try_pop(Frame_Queue* fq)
{
    SDL_LockMutex(fq->mutex);

    if (fq->count == 0) {
        SDL_UnlockMutex(fq->mutex);
        return NULL;
    }

    AVFrame* frame = fq->frames[fq->head];
    fq->head = (fq->head + 1) % FRAME_QUEUE_COUNT;
    fq->count--;

    SDL_SignalCondition(fq->not_full);

    SDL_UnlockMutex(fq->mutex);
    return frame;
}

void frame_queue_abort(Frame_Queue* fq)
{
    SDL_LockMutex(fq->mutex);
    SDL_SetAtomicInt(&fq->aborted, 1);
    SDL_BroadcastCondition(fq->not_full);
    SDL_BroadcastCondition(fq->not_empty);
    SDL_UnlockMutex(fq->mutex);
}