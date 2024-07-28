//
// Created by royda on 7/19/2024.
//

#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include "segel.h"

//typedef struct {
//    int fd; // File descriptor for the client connection
//    struct timeval arrival_time; // Arrival time
//    struct timeval dispatch_time; // Dispatch time
//    // Other request-related fields
//} request;
//
//typedef struct {
//    request **buf;
//    int max;
//    int front;
//    int rear;
//    int waiting_requests;
//    int running_requests;
//    pthread_mutex_t mutex;
//    pthread_cond_t cond_full;
//    pthread_cond_t cond_empty;
//} queue_t;
//
//void queueInit(queue_t *q, int n);
//void queueDestroy(queue_t *q);
//void enqueue_block(queue_t *q, request* item);
//void enqueue_drop_tail(queue_t *q, request* item);
//void enqueue_drop_head(queue_t *q, request *item);
//void enqueue_block_flush(queue_t *q, request *item);
//
//request* dequeue(queue_t *q);
//void decrementRunningRequests(queue_t *q);

enum sched_alg_type {block, dt, dh, bf, rnd};

typedef struct request{
    int fd; // File descriptor for the client connection
    struct timeval arrival_time; // Arrival time
    struct timeval dispatch_time; // Dispatch time
    struct request *next_request;
    // Other request-related fields
} request_t;

typedef struct {
    request_t *front;
    request_t *back;
    int max_size;
    //int size;
    int waiting_requests;
    int running_requests;
    pthread_mutex_t mutex;
    pthread_cond_t cond_full;
    pthread_cond_t cond_empty;
    pthread_cond_t cond_all_done;
} queue_t;

void queueInit(queue_t *q, int max_size);
void queueDestroy(queue_t *q);
void insertAtBack(queue_t *q, request_t* new_request);
void dropRequest(request_t* request);
void removeFront(queue_t *q);
void removeAtIndex(queue_t* q, int index);
void enqueueBlock(queue_t *q, request_t* new_request);
void enqueueDropTail(queue_t *q, request_t* new_request);
void enqueueDropHead(queue_t *q, request_t *new_request);
void enqueueBlockFlush(queue_t *q, request_t *new_request);
void enqueueDropRandom(queue_t *q, request_t *new_request);
request_t* dequeueLatest(queue_t *q);

void enqueueAccordingToAlgorithm(queue_t* q, request_t *new_request, enum sched_alg_type sched_alg);


request_t* dequeue(queue_t *q);
void UpdateQueueAfterFinishingRequest(queue_t* q);



#endif
