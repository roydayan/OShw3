#ifndef REQUEST_H
#define REQUEST_H

#include "queue.h"
#include <pthread.h>
#include "segel.h"


//Before merging queue file in:

typedef struct Threads_stats{
    int id;
    int stat_req;
    int dynm_req;
    int total_req;
    queue_t* wait_q;
    request_t* next_req;
} * threads_stats;

void requestHandle(int fd, struct timeval arrival, struct timeval dispatch, threads_stats t_stats);

#endif

/* old version:
#ifndef __REQUEST_H__

void requestHandle(int fd);

#endif
*/