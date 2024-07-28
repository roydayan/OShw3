//
// Created by mayan on 26/07/2024.
//

#include "worker.h"

void* worker_routine(void* arg) {
    thread_stats stats;
    queue_t *wait_q = (queue_t*) arg;
    while (1) {
        request_t* req = dequeue(wait_q);
        //assert(req != NULL);

        //gettimeofday(&req->dispatch_time, NULL); // Record dispatch time

//        long dispatch_interval = (req->dispatch_time.tv_sec - req->arrival_time.tv_sec) * 1000 +
//                                 (req->dispatch_time.tv_usec - req->arrival_time.tv_usec) / 1000;

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
//        char headers[1024];
//        snprintf(headers, sizeof(headers),
//                 "Stat-Req-Arrival: %ld.%06ld\r\n"
//                 "Stat-Req-Dispatch: %ld\r\n"
//                 "Stat-Thread-Id: %d\r\n"
//                 "Stat-Thread-Count: %d\r\n"
//                 "Stat-Thread-Static: %d\r\n"
//                 "Stat-Thread-Dynamic: %d\r\n",
//                 req->arrival_time.tv_sec, req->arrival_time.tv_usec,
//                 dispatch_interval,
//                 stats->id,
//                 (stats->static_count + stats->dynamic_count),
//                 stats->static_count,
//                 stats->dynamic_count);
        //TODO - rio_writen???

        //send_response(req->fd, headers); // Function to send response with headers

        // Discard the request
        Close(req->fd);
        free(req);
        UpdateQueueAfterFinishingRequest(wait_q);//this will update q->running_requests and signal

    }
}
