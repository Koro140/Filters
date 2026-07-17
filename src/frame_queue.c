#include "frame_queue.h"

#include <errno.h>
#include <time.h>

#ifdef _WIN32

#include <windows.h>

static inline void frame_queue_set_abort(Frame_Queue* fq)
{
    InterlockedExchange(&fq->aborted, 1);
}

static inline BOOL frame_queue_aborted(Frame_Queue* fq)
{
    return InterlockedCompareExchange(&fq->aborted, 0, 0) != 0;
}

void frame_queue_init(Frame_Queue* fq)
{
    fq->head = 0;
    fq->tail = 0;
    fq->count = 0;

    fq->aborted = 0;

    InitializeCriticalSection(&fq->mutex);

    InitializeConditionVariable(&fq->not_empty);
    InitializeConditionVariable(&fq->not_full);
}

void frame_queue_destroy(Frame_Queue* fq)
{
    EnterCriticalSection(&fq->mutex);

    while (fq->count > 0)
    {
        AVFrame* frame = fq->frames[fq->head];

        av_frame_free(&frame);

        fq->head = (fq->head + 1) % FRAME_QUEUE_COUNT;
        fq->count--;
    }

    LeaveCriticalSection(&fq->mutex);

    DeleteCriticalSection(&fq->mutex);
}

void frame_queue_push(Frame_Queue* fq, AVFrame* frame)
{
    EnterCriticalSection(&fq->mutex);

    while (fq->count == FRAME_QUEUE_COUNT &&
        !frame_queue_aborted(fq))
    {
        SleepConditionVariableCS(
            &fq->not_full,
            &fq->mutex,
            INFINITE);
    }

    if (frame_queue_aborted(fq))
    {
        LeaveCriticalSection(&fq->mutex);

        av_frame_free(&frame);
        return;
    }

    fq->frames[fq->tail] = frame;
    fq->tail = (fq->tail + 1) % FRAME_QUEUE_COUNT;
    fq->count++;

    WakeConditionVariable(&fq->not_empty);

    LeaveCriticalSection(&fq->mutex);
}

AVFrame* frame_queue_try_pop(Frame_Queue* fq)
{
    EnterCriticalSection(&fq->mutex);

    if (fq->count == 0)
    {
        LeaveCriticalSection(&fq->mutex);
        return NULL;
    }

    AVFrame* frame = fq->frames[fq->head];

    fq->head = (fq->head + 1) % FRAME_QUEUE_COUNT;
    fq->count--;

    WakeConditionVariable(&fq->not_full);

    LeaveCriticalSection(&fq->mutex);

    return frame;
}

void frame_queue_abort(Frame_Queue* fq)
{
    EnterCriticalSection(&fq->mutex);

    frame_queue_set_abort(fq);

    WakeAllConditionVariable(&fq->not_full);
    WakeAllConditionVariable(&fq->not_empty);

    LeaveCriticalSection(&fq->mutex);
}

#else
void frame_queue_init(Frame_Queue* fq)
{
    fq->head = 0;
    fq->tail = 0;
    fq->count = 0;
    atomic_init(&fq->aborted, false);

    if (pthread_mutex_init(&fq->mutex, NULL) == 0) {
        fq->mutex_initialized = true;
    }
    if (pthread_cond_init(&fq->not_empty, NULL) == 0) {
        fq->not_empty_initialized = true;
    }
    if (pthread_cond_init(&fq->not_full, NULL) == 0) {
        fq->not_full_initialized = true;
    }
}

void frame_queue_destroy(Frame_Queue* fq)
{
    if (fq->mutex_initialized) {
        pthread_mutex_lock(&fq->mutex);
    }

    while (fq->count > 0) {
        AVFrame* frame = fq->frames[fq->head];

        av_frame_free(&frame);

        fq->head = (fq->head + 1) % FRAME_QUEUE_COUNT;
        fq->count--;
    }

    if (fq->mutex_initialized) {
        pthread_mutex_unlock(&fq->mutex);
        pthread_mutex_destroy(&fq->mutex);
    }

    if (fq->not_empty_initialized) {
        pthread_cond_destroy(&fq->not_empty);
    }
    if (fq->not_full_initialized) {
        pthread_cond_destroy(&fq->not_full);
    }
}

void frame_queue_push(Frame_Queue* fq, AVFrame* frame)
{
    pthread_mutex_lock(&fq->mutex);

    while (fq->count == FRAME_QUEUE_COUNT && !atomic_load(&fq->aborted))
        pthread_cond_wait(&fq->not_full, &fq->mutex);

    if (atomic_load(&fq->aborted)) {
        pthread_mutex_unlock(&fq->mutex);
        av_frame_free(&frame);   // we own it, don't leak it on shutdown
        return;
    }

    fq->frames[fq->tail] = frame;
    fq->tail = (fq->tail + 1) % FRAME_QUEUE_COUNT;
    fq->count++;

    pthread_cond_signal(&fq->not_empty);

    pthread_mutex_unlock(&fq->mutex);
}

AVFrame* frame_queue_try_pop(Frame_Queue* fq)
{
    pthread_mutex_lock(&fq->mutex);

    if (fq->count == 0) {
        pthread_mutex_unlock(&fq->mutex);
        return NULL;
    }

    AVFrame* frame = fq->frames[fq->head];
    fq->head = (fq->head + 1) % FRAME_QUEUE_COUNT;
    fq->count--;

    pthread_cond_signal(&fq->not_full);

    pthread_mutex_unlock(&fq->mutex);
    return frame;
}

void frame_queue_abort(Frame_Queue* fq)
{
    pthread_mutex_lock(&fq->mutex);
    atomic_store(&fq->aborted, true);
    pthread_cond_broadcast(&fq->not_full);
    pthread_cond_broadcast(&fq->not_empty);
    pthread_mutex_unlock(&fq->mutex);
}
#endif // _WIN32