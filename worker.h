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

typedef struct Threads_stats{
    int id;
    int static_count;
    int dynamic_count;
} * thread_stats;


void* worker_thread(void* arg) {
    thread_stats stats;
    queue_t *wait_q = (queue_t*) arg;
    while (1) {
        request* req = dequeue(wait_q);
        assert(req != NULL);

        gettimeofday(&req->dispatch_time, NULL); // Record dispatch time

        long dispatch_interval = (req->dispatch_time.tv_sec - req->arrival_time.tv_sec) * 1000 +
                                 (req->dispatch_time.tv_usec - req->arrival_time.tv_usec) / 1000;

        /* TODO - Update thread statistics
        if (is_static_request(req)) {
            stats->static_count++;
        } else {
            stats->dynamic_count++;
        }
        */

        // Process the request
        requestHandle(req->fd);
        //TODO - ^MAYBE CHANGE RETURN VALUE OF HANDLE TO BE IS_STATIC (updated by requestParseURI in request.c) TO UPDATE STATS!!!!!!!!!!!!!!!!!!!!!!!!!!


        // Embed statistics in the response headers
        char headers[1024];
        snprintf(headers, sizeof(headers),
                 "Stat-Req-Arrival: %ld.%06ld\r\n"
                 "Stat-Req-Dispatch: %ld\r\n"
                 "Stat-Thread-Id: %d\r\n"
                 "Stat-Thread-Count: %d\r\n"
                 "Stat-Thread-Static: %d\r\n"
                 "Stat-Thread-Dynamic: %d\r\n",
                 req->arrival_time.tv_sec, req->arrival_time.tv_usec,
                 dispatch_interval,
                 stats->id,
                 (stats->static_count + stats->dynamic_count),
                 stats->static_count,
                 stats->dynamic_count);
        //TODO - rio_writen???

        //send_response(req->fd, headers); // Function to send response with headers

        // Discard the request
        Close(req->fd);
        free(req);
        decrementRunningRequests(wait_q);

    }
}

#endif //SERVERCLIENTPROJECT_WORKER_H
