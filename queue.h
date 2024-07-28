//
// Created by royda on 7/19/2024.
//

#ifndef SERVERCLIENTPROJECT_QUEUE_H
#define SERVERCLIENTPROJECT_QUEUE_H

#include <pthread.h>
#include "segel.h"
#include "assert.h" //TODO - remove this before submission!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

typedef struct {
    int fd; // File descriptor for the client connection
    struct timeval arrival_time; // Arrival time
    struct timeval dispatch_time; // Dispatch time
    // Other request-related fields
} request;

request* createRequest(int fd);


typedef struct {
    request **buf;
    int max;
    int front;
    int rear;
    int waiting_requests;
    int running_requests;
    pthread_mutex_t mutex;
    pthread_cond_t cond_full;
    pthread_cond_t cond_empty;
} queue_t;

queue_t* queueInit(int n);
void queueDestroy(queue_t *q);
void enqueue(queue_t *q, request* item);
request* dequeue(queue_t *q);
void decrementRunningRequests(queue_t *q);
request* dequeueLatest(queue_t *q);


#endif //SERVERCLIENTPROJECT_QUEUE_H
