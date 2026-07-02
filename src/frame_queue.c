#include "frame_queue.h"

#include <errno.h>
#include <time.h>

void frame_queue_init(Frame_Queue *fq)
{
    fq->head = 0;
    fq->tail = 0;
    fq->count = 0;
    atomic_init(&fq->aborted, false);

    pthread_mutex_init(&fq->mutex, NULL);
    pthread_cond_init(&fq->not_empty, NULL);
    pthread_cond_init(&fq->not_full, NULL);
}

void frame_queue_destroy(Frame_Queue *fq)
{
    pthread_mutex_lock(&fq->mutex);

    while (fq->count > 0) {
        AVFrame *frame = fq->frames[fq->head];

        av_frame_free(&frame);

        fq->head = (fq->head + 1) % FRAME_QUEUE_COUNT;
        fq->count--;
    }

    pthread_mutex_unlock(&fq->mutex);

    pthread_mutex_destroy(&fq->mutex);
    pthread_cond_destroy(&fq->not_empty);
    pthread_cond_destroy(&fq->not_full);
}

void frame_queue_push(Frame_Queue *fq, AVFrame *frame)
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

AVFrame *frame_queue_try_pop(Frame_Queue *fq)
{
    pthread_mutex_lock(&fq->mutex);

    if (fq->count == 0) {
        pthread_mutex_unlock(&fq->mutex);
        return NULL;
    }

    AVFrame *frame = fq->frames[fq->head];
    fq->head = (fq->head + 1) % FRAME_QUEUE_COUNT;
    fq->count--;

    pthread_cond_signal(&fq->not_full);

    pthread_mutex_unlock(&fq->mutex);
    return frame;
}

void frame_queue_abort(Frame_Queue *fq)
{
    pthread_mutex_lock(&fq->mutex);
    atomic_store(&fq->aborted, true);
    pthread_cond_broadcast(&fq->not_full);
    pthread_cond_broadcast(&fq->not_empty);
    pthread_mutex_unlock(&fq->mutex);
}