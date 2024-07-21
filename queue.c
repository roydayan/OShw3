//
// Created by royda on 7/19/2024.
//

#include "queue.h"

#include <stdlib.h>

queue_t* queueInit(int n) {
    queue_t* q = (queue_t*)malloc(sizeof(queue_t));
    q->buf = (request **)malloc(sizeof(request*) * n);
    q->max = n;
    q->front = 0;
    q->rear = 0;
    q->waiting_requests = 0;
    q->running_requests = 0;
    pthread_mutex_init(&(q->mutex), NULL);
    pthread_cond_init(&(q->cond_full), NULL);
    pthread_cond_init(&(q->cond_empty), NULL);
    return q;
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
    free(q);
}
//TODO -- implement a different queue function for each of the algorithms: block, drop_tail , etc
void enqueue(queue_t *q, request* item) {
    pthread_mutex_lock(&(q->mutex));
    while (q->waiting_requests + q->running_requests == q->max) {
        pthread_cond_wait(&(q->cond_full), &(q->mutex));
    }
    q->buf[q->rear] = item;
    q->rear = (q->rear + 1) % q->max;
    q->waiting_requests++;
    pthread_cond_signal(&(q->cond_empty));
    pthread_mutex_unlock(&(q->mutex));
}

request* dequeue(queue_t *q) { //TODO - make sure valid when there is one request left - can front be less than rear? is it okay?!!!!
    pthread_mutex_lock(&(q->mutex));
    while (q->waiting_requests + q->running_requests == 0) {
        pthread_cond_wait(&(q->cond_empty), &(q->mutex));
    }
    request *item = q->buf[q->front];
    q->buf[q->front] = NULL;
    q->front = (q->front + 1) % q->max;
    q->waiting_requests--;
    q->running_requests++;
    //pthread_cond_signal(&(q->cond_full)); TODO - ensure this is safe (no race condition, deadlock...)!!!!!!!!!
    pthread_mutex_unlock(&(q->mutex));
    return item;
}

void decrementRunningRequests(queue_t *q) {
    pthread_mutex_lock(&(q->mutex));
    q->running_requests--;
    pthread_cond_signal(&(q->cond_full));
    pthread_mutex_unlock(&(q->mutex));
}

//dequeue latest request (performed when request filename suffix is .skip)
request* dequeueLatest(queue_t *q) {
    pthread_mutex_lock(&(q->mutex));
    while (q->waiting_requests + q->running_requests == 0) {
        pthread_cond_wait(&(q->cond_empty), &(q->mutex));
    }
    request *item = q->buf[q->rear];
    q->buf[q->rear] = NULL;
    q->rear = (q->rear - 1) % q->max;
    q->waiting_requests--;
    q->running_requests++;
    //pthread_cond_signal(&(q->cond_full)); TODO - ensure this is safe (no race condition, deadlock...)!!!!!!!!!
    pthread_mutex_unlock(&(q->mutex));
    return item;
}


request* createRequest(int fd) {
    request* new_request = (request*)malloc(sizeof(request));
    if (new_request == NULL) {
        exit(1);
        //TODO -what to do if malloc fails???????????
    }
    new_request->fd = fd;
    gettimeofday(&new_request->arrival_time, NULL); // Record arrival time
    return new_request;
}