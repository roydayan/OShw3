//
// Created by royda on 7/19/2024.
//

#ifndef SERVERCLIENTPROJECT_WORKER_H
#define SERVERCLIENTPROJECT_WORKER_H

#include <pthread.h>
#include "queue.h"
#include "request.h"
#include "segel.h"
#include "assert.h"

/* defined in request.h:
typedef struct Thread_stats {
    int id;
    int stat_req;
    int dynm_req;
    int total_req;
} *thread_stats;
 */

//struct to pass arguments to thread
typedef struct {
    queue_t* queue_ptr;
    int index;
} arg_array;

//check special suffix
int checkSkipSuffix(int fd) {//TODO - implement this!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    return 0;
}
//remove special suffix for current request
void removeSkipSuffix(int fd) {//TODO - implement this!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

void* worker_thread(void* arg) {
    //init thread stats
    threads_stats t_stats = (threads_stats) malloc(sizeof(struct Threads_stats));
    t_stats->stat_req = 0;
    t_stats->dynm_req = 0;
    t_stats->total_req = 0;
    arg_array *args = (arg_array*) arg;
    t_stats->id = args->index;

    queue_t *wait_q = args->queue_ptr;
    //queue_t *wait_q = (queue_t*) arg;

    request* next_req = NULL; //for special suffix policy
    //thread routine:
    while (1) {
        request* req = NULL;
        //check if skipping:
        if (next_req == NULL) {
            req = dequeue(wait_q);
        }
        else {
            req = next_req;
            next_req = NULL; //reset before next iteration!, freed when req is freed
        }
        assert(req != NULL);
        t_stats->total_req++; //TODO - update total requests here or in requestHandle??
        if (checkSkipSuffix(req->fd)) {
            removeSkipSuffix(req->fd);
            next_req = dequeueLatest(wait_q);
        }

        gettimeofday(&req->dispatch_time, NULL); // Record dispatch time

        long dispatch_interval = (req->dispatch_time.tv_sec - req->arrival_time.tv_sec) * 1000 +
                                 (req->dispatch_time.tv_usec - req->arrival_time.tv_usec) / 1000;

        // Process the request, thread stats are updated in requestHandle through the t_stats ptr
        requestHandle(req->fd, req->arrival_time, req->dispatch_time, t_stats); //TODO - change dispatch time to dispatch interval (long or struct timeval???) !!!!!!!!
        // response is embedded in fd



        // Discard the request
        Close(req->fd);
        free(req);
        decrementRunningRequests(wait_q);
    }
    free(t_stats);
}

#endif //SERVERCLIENTPROJECT_WORKER_H
