#include "segel.h"
#include "request.h"
//#include "queue.h"
//#include "worker.h"
#include "assert.h" //TODO - remove this before submission!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

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
void* worker_thread(void* arg);

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
        fprintf(stderr, "malloc threads failed");
        exit(1);
        //TODO - if malloc fails then (piazza) print error message and exit
    }

    queue_t* wait_q = queueInit(queue_size);

    // HW3: Create some threads...
    for (int i = 0; i < num_threads; i++) {
        threads_stats t_stats = (threads_stats) malloc(sizeof(struct Threads_stats));
        if(threads == NULL){
            fprintf(stderr, "malloc threads_stats failed");
            exit(1);
        }
        t_stats->id = i;
        t_stats->wait_q = wait_q;
        pthread_create(&threads[i], NULL, worker_thread, (void*)t_stats);
    }
    struct timeval temp_arrival_time; //for immediate time recording

    listenfd = Open_listenfd(port); //error - bind failed 7/20/2024 14:46, server ran without client might be reason, simply for checking validity
    while (1) {
	    clientlen = sizeof(clientaddr);
	    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        gettimeofday(&temp_arrival_time, NULL); // Record arrival time
        request* new_request = createRequest(connfd);  //request is defined in queue.h
        new_request->arrival_time = temp_arrival_time; //update the arrival time to be exact
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


//--------------------------------------------------------------------
//WORKER THREADS
//--------------------------------------------------------------------

void* worker_thread(void* arg) {
    //init thread stats
    threads_stats t_stats = (threads_stats) arg;
    t_stats->stat_req = 0;
    t_stats->dynm_req = 0;
    t_stats->total_req = 0;
    t_stats->next_req = NULL; //for special suffix policy

    //thread routine:
    while (1) {
        request* req = NULL;
        //check if skipping:
        if (t_stats->next_req == NULL) {
            req = dequeue(t_stats->wait_q);
        }
        else {
            req = t_stats->next_req;
            t_stats->next_req = NULL; //reset before next iteration!, freed when req is freed
        }
        assert(req != NULL); //maybe if req==NULL then break??

        struct timeval dispatch_interval;
        timersub(&req->dispatch_time, &req->arrival_time, &dispatch_interval); // Calculate dispatch interval (according to chatgpt)

        // Process the request, thread stats are updated in requestHandle through the t_stats ptr, response will be embedded in fd
        requestHandle(req->fd, req->arrival_time, dispatch_interval, t_stats);

        // Discard the request
        Close(req->fd);
        free(req);
        decrementRunningRequests(t_stats->wait_q);
    }
    free(t_stats);
}