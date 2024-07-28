//
// Created by royda on 7/19/2024.
//

#ifndef WORKER_H
#define WORKER_H

#include <pthread.h>
#include "queue.h"
#include "request.h"
#include "segel.h"

typedef struct Threads_stats{
    int id;
    int static_count;
    int dynamic_count;
} * thread_stats;

void* worker_routine(void* arg);


#endif
