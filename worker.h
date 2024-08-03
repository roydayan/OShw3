//
// Created by royda on 7/19/2024.
//

#ifndef WORKER_H
#define WORKER_H

#include <pthread.h>
#include "queue.h"
#include "request.h"
#include "segel.h"

void* worker_routine(void* arg);


#endif
