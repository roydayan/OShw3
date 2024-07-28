//
// Created by royda on 7/19/2024.
//

#include "queue.h"

#include <stdlib.h>


void queueInit(queue_t *q, int max_size) {
    q->max_size = max_size;
    //q->size = 0;
    q->front = NULL;
    q->back = NULL;
    q->waiting_requests = 0;
    q->running_requests = 0;
    pthread_mutex_init(&(q->mutex), NULL);
    pthread_cond_init(&(q->cond_full), NULL);
    pthread_cond_init(&(q->cond_empty), NULL);
    pthread_cond_init(&(q->cond_all_done), NULL);
}

void queueDestroy(queue_t *q) {
    while (q->front != NULL){
        request_t* tmp = q->front;
        dropRequest(tmp);//also frees tmp
        q->front = q->front->next_request;
    }
    pthread_mutex_destroy(&(q->mutex));
    pthread_cond_destroy(&(q->cond_full));
    pthread_cond_destroy(&(q->cond_empty));
    pthread_cond_destroy(&(q->cond_all_done));
}


void insertAtBack(queue_t *q, request_t* new_request){
    if(q->back == NULL){
        q->front = new_request;
        q->back = new_request;
    }
    else{
        new_request->next_request = q->back;
        q->back = new_request;
    }
    //q->size++;
    q->waiting_requests++;
}

void dropRequest(request_t* request){
    if(request != NULL){
        //TODO -- close socket!
        Close(request->fd);
        free(request);
    }
    return;
}

void removeFront(queue_t *q){
    request_t* last_front = q->front;
    if(last_front == NULL){
        return;
    }

    if (q->front == q->back){
        q->front = NULL;
        q->back = NULL;
    }
    else{
        request_t* it = q->back;
        while(it->next_request != q->front){
            it = it->next_request;
        }
        q->front = it;
        it->next_request = NULL;
    }
    //dropRequest(last_front);
}


//TODO -- implement a different queue function for each of the algorithms: block, drop_tail , etc
void enqueueBlock(queue_t *q, request_t* new_request) {
    pthread_mutex_lock(&(q->mutex));
    while (q->waiting_requests + q->running_requests == q->max_size) {
        pthread_cond_wait(&(q->cond_full), &(q->mutex));
    }

    insertAtBack(q, new_request);

    pthread_cond_signal(&(q->cond_empty));
    pthread_mutex_unlock(&(q->mutex));
}


void enqueueDropTail(queue_t *q, request_t* new_request) {
    pthread_mutex_lock(&(q->mutex));
    if(q->waiting_requests + q->running_requests == q->max_size){
        pthread_mutex_unlock(&(q->mutex));
        return;
    }
    insertAtBack(q, new_request);
    pthread_cond_signal(&(q->cond_empty));
    pthread_mutex_unlock(&(q->mutex));
}

void enqueueDropHead(queue_t *q, request_t* new_request) {
    pthread_mutex_lock(&(q->mutex));
    if(q->waiting_requests + q->running_requests == q->max_size){
        if(q->waiting_requests > 0){
            request_t * oldest_request = q->front;
            removeFront(q);
            dropRequest(oldest_request);
        }
        else{//TODO-- according to piazza this case wont be tested...
            dropRequest(new_request);
            pthread_mutex_unlock(&(q->mutex));
            return;
        }

    }
    insertAtBack(q, new_request);
    pthread_cond_signal(&(q->cond_empty));
    pthread_mutex_unlock(&(q->mutex));
}

void enqueueBlockFlush(queue_t *q, request_t* new_request){
    pthread_mutex_lock(&(q->mutex));
    while (q->waiting_requests + q->running_requests != 0) {//TODO -- does this cause mutual exclosion ???
        pthread_cond_wait(&(q->cond_all_done), &(q->mutex));
    }
    //TODO -- do i need to add the new request to the queue? or discard it?
    dropRequest(new_request);
    //pthread_cond_signal(&(q->cond_empty));
    pthread_mutex_unlock(&(q->mutex));
}

void enqueueDropRandom(queue_t *q, request_t *new_request){
    pthread_mutex_lock(&(q->mutex));
    if(q->waiting_requests + q->running_requests == q->max_size){
        if(q->waiting_requests == 0){//TODO-- according to piazza this shouldn't happen
            dropRequest(new_request);
            pthread_mutex_unlock(&(q->mutex));
            return;
        }
        //remove randomly half of elements in waiting queue
        int half_initial_size = 1;
        if( (q->waiting_requests % 2) == 0){
            half_initial_size = (int) ((q->waiting_requests) / 2);
        }
        else{
            half_initial_size = (int) ((q->waiting_requests+1) / 2);
        }
        //half_initial_size = half_initial_size > 0 ? half_initial_size : 1;

        for( int i = 0 ; i < half_initial_size; i++) {
            if(q->waiting_requests == 0){
                break;
            }
            int random_index = rand() % q->waiting_requests;
            removeAtIndex(q, random_index);
            q->waiting_requests--;
        }
    }

    insertAtBack(q, new_request);
    pthread_cond_signal(&(q->cond_empty));
    pthread_mutex_unlock(&(q->mutex));
}

void removeAtIndex(queue_t* q, int index){
    if(q->front == q->back){
        q->back = NULL;
        q->front = NULL;
        return;
    }
    if(index == 0){
        request_t* tmp = q->back;
        q->back = q->back->next_request;
        dropRequest(tmp);//also frees
        return;
    }

    request_t * current = q->back->next_request;//this will hold the element to remove
    request_t * before = q->back;
    int i  = index-1;
    while(i != 0){
        before = current;
        current = current->next_request;
        i--;
    }

    before->next_request = current->next_request;

    if(current == q->front){
        q->front = before;
    }
    dropRequest(current);
    return;
}


//dequeue latest request (performed when request filename suffix is .skip)
request_t* dequeueLatest(queue_t *q) {
    pthread_mutex_lock(&(q->mutex));
    while (q->waiting_requests == 0) {
        pthread_mutex_unlock(&(q->mutex)); //TODO -check need for cond_wait!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        return NULL; //TODO - if during skip the queue is empty then "ignore the skip action and move on" (piazza 435) - does it mean to return NULL here???
        //pthread_cond_wait(&(q->cond_empty), &(q->mutex));
    }
    request_t* tmp = q->back;
    q->back = q->back->next_request;
    gettimeofday(&tmp->dispatch_time, NULL);
    if (q->back == NULL){
        q->front = NULL;
    }
    q->waiting_requests--;
    q->running_requests++;
    pthread_mutex_unlock(&(q->mutex));
    return tmp;
}



//enum sched_alg_type {block, dt, dh, bf, rnd};
void enqueueAccordingToAlgorithm(queue_t* q, request_t *new_request, enum sched_alg_type sched_alg){
    switch (sched_alg) {
        case block:
            enqueueBlock(q, new_request);
            return;

        case dt:
            enqueueDropTail(q, new_request);
            return;

        case dh:
            enqueueDropHead(q, new_request);
            return;

        case bf:
            enqueueBlockFlush(q, new_request);
            return;

        case rnd:
            enqueueDropRandom(q, new_request);
            return;

        default:
            printf("No congestion algorithm fitted! this should'nt happen!");
            enqueueBlock(q, new_request);
            return;
    }
}





request_t* dequeue(queue_t *q) {
    pthread_mutex_lock(&(q->mutex));
    while (q->waiting_requests  == 0) {
        pthread_cond_wait(&(q->cond_empty), &(q->mutex));
    }
    request_t* request = q->front;
    removeFront(q);
    gettimeofday(&request->dispatch_time, NULL);
    q->waiting_requests--;
    q->running_requests++;
    //pthread_cond_signal(&(q->cond_full)); TODO - ensure this is safe (no race condition, deadlock...)!!!!!!!!!
    pthread_mutex_unlock(&(q->mutex));
    return request;
}


void UpdateQueueAfterFinishingRequest(queue_t* q){
    pthread_mutex_lock(&(q->mutex));

    q->running_requests--;
    pthread_cond_signal(&(q->cond_full));
    if(q->running_requests+q->waiting_requests == 0){
        pthread_cond_signal(&(q->cond_all_done));
    }

    pthread_mutex_unlock(&(q->mutex));
}