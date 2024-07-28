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

//enum sched_alg_type {block, dt, dh, bf, rnd};

/* declared in worker.h
typedef struct Threads_Stats{
    int id;
    int static_count;
    int dynamic_count;
} * thread_stats;
 */

//void* worker_routine(void* arg);
request_t* createRequest(int fd);


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

    queue_t* wait_q = (queue_t*)malloc(sizeof (queue_t));
    queueInit(wait_q, queue_size);

    //array of thread stats that will pass as arguments to worker threads
    threads_stats* t_stats_array = (threads_stats*) malloc(sizeof(threads_stats) * num_threads);
    if(t_stats_array == NULL){
        fprintf(stderr, "malloc threads_stats_array failed");
        exit(1);
    }

    // HW3: Create some threads...
    for (int i = 0; i < num_threads; i++) {
        t_stats_array[i] = (threads_stats)malloc(sizeof(struct Threads_stats));
        if(t_stats_array[i] == NULL){
            fprintf(stderr, "malloc threads_stats failed");
            exit(1);
        }
        threads_stats t_stats = t_stats_array[i];
        t_stats->id = i;
        t_stats->wait_q = wait_q;
        pthread_create(&threads[i], NULL, worker_routine, (void*)t_stats);
        //TODO-- what if pthread_create fails??
    }

    struct timeval temp_arrival_time; //temporary variable for immediate time recording

    listenfd = Open_listenfd(port);
    while (1) {
	    clientlen = sizeof(clientaddr);
	    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        gettimeofday(&temp_arrival_time, NULL); // Record arrival time
        request_t* new_request = createRequest(connfd);  //request is defined in queue.h
        new_request->arrival_time = temp_arrival_time; //update the arrival time to be exact
        enqueueAccordingToAlgorithm(wait_q, new_request, sched_alg);

        // HW3: In general, don't handle the request in the main thread.
        // Save the relevant info in a buffer and have one of the worker threads
        // do the work.
        //requestHandle(connfd); --in the thread
        //Close(connfd); --in the thread

        if (connfd < 0) { //TODO - get rid of this. I wrote this so that code after while is reachable
            fprintf(stderr, "connfd < 0, accept failed");
            continue;
            //TODO -when to exit while????????????????????????????????????????????
        }
    }
    /*
    for (int i = 0; i < num_threads; i++) {
        free(t_stats_array[i]);
    }
    free(t_stats_array);
    free(threads);
    queueDestroy(wait_q);
    return 0;
     */
}

request_t* createRequest(int fd) {
    request_t* new_request = (request_t*)malloc(sizeof(request_t));
    if (new_request == NULL) {
        exit(1);
        //TODO -what to do if malloc fails???????????
    }
    new_request->fd = fd;
    new_request->next_request =NULL;
    return new_request;
}