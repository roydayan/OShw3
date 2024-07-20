//
// Created by royda on 7/19/2024.
//

#include "queue.h"

#include <stdlib.h>

void queueInit(queue_t *q, int n) {
    q->buf = (request **)malloc(sizeof(request*) * n);
    q->max = n;
    q->front = 0;
    q->rear = 0;
    q->count = 0;
    q->running_requests = 0;
    pthread_mutex_init(&(q->mutex), NULL);
    pthread_cond_init(&(q->cond_full), NULL);
    pthread_cond_init(&(q->cond_empty), NULL);
}

void queueDestroy(queue_t *q) {
    for (int i = 0; i < q->max; i++) {
        if(q->buf[i] != NULL) {
            free(q->buf[i]);
        }
    }
    free(q->buf);
    pthread_mutex_destroy(&(q->mutex));
    pthread_cond_destroy(&(q->cond_full));
    pthread_cond_destroy(&(q->cond_empty));
}
//TODO -- implement a different queue function for each of the algorithms: block, drop_tail , etc
void enqueue(queue_t *q, request* item) {
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

int dequeue(queue_t *q) {
    pthread_mutex_lock(&(q->mutex));
    while (q->count == 0) {
        pthread_cond_wait(&(q->cond_empty), &(q->mutex));
    }
    int item = q->buf[q->front];
    q->front = (q->front + 1) % q->max;
    q->count--;
    q->running_requests++;
    //pthread_cond_signal(&(q->cond_full)); TODO - ensure this is safe (no race condition, deadlock...)!!!!!!!!!
    pthread_mutex_unlock(&(q->mutex));
    return item;
}




