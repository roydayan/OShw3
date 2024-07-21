#include "segel.h"
#include "request.h"
#include "queue.h"
#include "worker.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

enum sched_alg_type {block, dt, dh, bf, rnd};


// HW3: Parse the new arguments too
//./server [portnum] [threads] [queue_size] [sched_alg]
void getargs(int *port,int* queue_size, enum sched_alg_type* sched_alg, int* num_threads, int argc, char *argv[])
{
    if (argc < 5) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *num_threads = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    if (strcmp(argv[4], "block") == 0) {
        *sched_alg = block;
    } else if (strcmp(argv[4], "dt") == 0) {
        *sched_alg = dt;
    } else if (strcmp(argv[4], "dh") == 0) {
        *sched_alg = dh;
    } else if (strcmp(argv[4], "bf") == 0) {
        *sched_alg = bf;
    } else if (strcmp(argv[4], "random") == 0) {
        *sched_alg = rnd;
    } else {
        fprintf(stderr, "Usage: %s <port> <queue_size> <sched_alg>\n", argv[0]);
        exit(1);
        //TODO - add error message!!!!!!!!!!!!
    }

}


int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, num_threads, queue_size;
    enum sched_alg_type sched_alg;
    struct sockaddr_in clientaddr;

    getargs(&port, &queue_size, &sched_alg, &num_threads, argc, argv);

    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
    if(threads == NULL){
        exit(1);
        //TODO -what to do if malloc fails?
    }

    queue_t* wait_q = queueInit(queue_size);

    // HW3: Create some threads...
    arg_array args; //pass arguments to thread, arg_array is defined in worker.h
    args.queue_ptr = wait_q;
    for (int i = 0; i < num_threads; i++) {
        args.index = i;
        pthread_create(&threads[i], NULL, worker_thread, &args);
    }

    listenfd = Open_listenfd(port); //error - bind failed 7/20/2024 14:46, server ran without client might be reason, simply for checking validity
    while (1) {
	    clientlen = sizeof(clientaddr);
	    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        request* new_request = createRequest(connfd);  //request is defined in queue.h
        enqueue(wait_q, new_request);

        // HW3: In general, don't handle the request in the main thread.
        // Save the relevant info in a buffer and have one of the worker threads
        // do the work.
        //requestHandle(connfd); --in the thread
        //Close(connfd); --in the thread

        if (connfd < 0) { //TODO - get rid of this. I wrote this so that code after while is reachable
            break;
            //TODO -when to exit while????????????????????????????????????????????
        }
    }
    free(threads);
    queueDestroy(wait_q);

    return 0;
}

/*
void* worker_routine(void* arg) {
    thread_stats stats;
    queue_t * wait_q = (queue_t*) arg;

    while (1) {
        request* req = dequeue(wait_q);
        assert(req != NULL);
        if (req == NULL) {
            continue; // No request to process
        }

        gettimeofday(&req->dispatch_time, NULL); // Record dispatch time

        long dispatch_interval = (req->dispatch_time.tv_sec - req->arrival_time.tv_sec) * 1000 +
                                 (req->dispatch_time.tv_usec - req->arrival_time.tv_usec) / 1000;

         TODO - Update thread statistics
        if (is_static_request(req)) {
            stats->static_count++;
        } else {
            stats->dynamic_count++;
        }


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

        //send_response(req->fd, headers); // Function to send response with headers

        // Discard the request
        //TODO - lower num of running requests in queue
        Close(req->fd);
        free(req);
    }
}
*/