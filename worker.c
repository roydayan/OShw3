//
// Created by mayan on 26/07/2024.
//

#include "worker.h"

void* worker_routine(void* arg) {
    //init thread stats
    threads_stats t_stats = (threads_stats) arg;
    t_stats->stat_req = 0;
    t_stats->dynm_req = 0;
    t_stats->total_req = 0;
    t_stats->next_req = NULL; //for special suffix policy

    //thread routine:
    while (1) {
        request_t* req = NULL;
        //check if skipping:
        if (t_stats->next_req == NULL) {
            req = dequeue(t_stats->wait_q);
        }
        else {
            req = t_stats->next_req;
            t_stats->next_req = NULL; //reset before next iteration!, freed when req is freed
        }

        struct timeval dispatch_interval;
        timersub(&req->dispatch_time, &req->arrival_time, &dispatch_interval); // Calculate dispatch interval (according to chatgpt)

        // Process the request, thread stats are updated in requestHandle through the t_stats ptr, response will be embedded in fd
        requestHandle(req->fd, req->arrival_time, dispatch_interval, t_stats);

        // Discard the request
        Close(req->fd);
        free(req);
        UpdateQueueAfterFinishingRequest(t_stats->wait_q);//this will update q->running_requests and signal
    }
    free(t_stats);
}
