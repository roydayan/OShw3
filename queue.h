//
// Created by royda on 7/19/2024.
//

#ifndef SERVERCLIENTPROJECT_QUEUE_H
#define SERVERCLIENTPROJECT_QUEUE_H

#include <pthread.h>

typedef struct {
    int *buf;
    int max;
    int front;
    int rear;
    int count;
    int running_requests;
    pthread_mutex_t mutex;
    pthread_cond_t cond_full;
    pthread_cond_t cond_empty;
} queue_t;

void queue_init(queue_t *q, int n);
void queue_destroy(queue_t *q);
void queue_insert(queue_t *q, int item);
int queue_remove(queue_t *q);


#endif //SERVERCLIENTPROJECT_QUEUE_H
