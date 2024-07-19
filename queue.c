//
// Created by royda on 7/19/2024.
//

#include "queue.h"

#include <stdlib.h>

void queue_init(queue_t *q, int n) {
    q->buf = (int *)malloc(sizeof(int) * n);
    q->max = n;
    q->front = 0;
    q->rear = 0;
    q->count = 0;
    q->running_requests = 0;
    pthread_mutex_init(&(q->mutex), NULL);
    pthread_cond_init(&(q->cond_full), NULL);
    pthread_cond_init(&(q->cond_empty), NULL);
}

void queue_destroy(queue_t *q) {
    free(q->buf);
    pthread_mutex_destroy(&(q->mutex));
    pthread_cond_destroy(&(q->cond_full));
    pthread_cond_destroy(&(q->cond_empty));
}

void queue_insert(queue_t *q, int item) {
    pthread_mutex_lock(&(q->mutex));
    while (q->count + q->running_requests == q->max) {
        pthread_cond_wait(&(q->cond_full), &(q->mutex));
    }
    q->buf[q->rear] = item;
    q->rear = (q->rear + 1) % q->max;
    q->count++;
    pthread_cond_signal(&(q->cond_empty));
    pthread_mutex_unlock(&(q->mutex));
}

int queue_remove(queue_t *q) {
    pthread_mutex_lock(&(q->mutex));
    while (q->count == 0) {
        pthread_cond_wait(&(q->cond_empty), &(q->mutex));
    }
    int item = q->buf[q->front];
    q->front = (q->front + 1) % q->max;
    q->count--;
    q->running_requests++;
    pthread_cond_signal(&(q->cond_full));
    pthread_mutex_unlock(&(q->mutex));
    return item;
}




