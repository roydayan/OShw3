#include "segel.h"
#include "request.h"
#include "queue.h"

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

    queue_t* wait_q;
    queue_init(&wait_q, queue_size);


    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker, NULL);
    }
    // 
    // HW3: Create some threads...
    //

    listenfd = Open_listenfd(port);
    while (1) {
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

	// 
	// HW3: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads
	// do the work. 
	// 
	requestHandle(connfd);

	Close(connfd);
    free(threads);
    }

}