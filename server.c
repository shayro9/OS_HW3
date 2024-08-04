#include "segel.h"
#include "request.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

/// /////
struct Queues_container{
    struct Queue * m_waiting_ptr;
    struct Queue * m_running_ptr;
    int index;
};
// HW3: Parse the new arguments too
void getargs(int *port, int *pool_size, int *queue_size, char **schedalg, int argc, char *argv[])
{
    //TODO: ADD VALIDATION
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    if(argc < 5) exit(1);

    *port = atoi(argv[1]);
    *pool_size = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    *schedalg = argv[4];
}

void *thread_function(void* Container);

pthread_cond_t cnd, master_cnd, empty_cnd;
pthread_mutex_t mtx;

int main(int argc, char *argv[])
{   
    int listenfd, connfd, port, clientlen, queue_size, pool_size;
    char* schedalg = NULL;
    struct sockaddr_in clientaddr;
    struct Queue * waiting_ptr = createQueue();
    struct Queue * running_ptr = createQueue();

    getargs(&port, &pool_size, &queue_size, &schedalg, argc, argv);

    pthread_mutex_init(&mtx, NULL);
    pthread_cond_init(&cnd, NULL);
    pthread_cond_init(&master_cnd, NULL);
    pthread_cond_init(&empty_cnd, NULL);

    pthread_t *thread_pool = (pthread_t *)malloc(pool_size * sizeof(pthread_t));
    struct Queues_container *containers = (struct Queues_container *)malloc(pool_size * sizeof(struct Queues_container));
    for (int i = 0; i < pool_size; i++) {
        containers[i].m_waiting_ptr = waiting_ptr;
        containers[i].m_running_ptr = running_ptr;
        containers[i].index = i;

        if (pthread_create(&thread_pool[i], NULL, thread_function, &containers[i]) != 0) {
            perror("pthread_create failed");
            exit(EXIT_FAILURE);
        }
    }

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        struct timeval arrival_time;
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        gettimeofday(&arrival_time, NULL);

        pthread_mutex_lock(&mtx);
        while(waiting_ptr->size + running_ptr->size  >= queue_size){
            if(strcmp(schedalg, "block") == 0){
                //printf("Waiting because of SIZE\n");
                pthread_cond_wait(&master_cnd, &mtx);
            }

            else if(strcmp(schedalg, "dt") == 0){
                pthread_mutex_unlock(&mtx);
                Close(connfd);
                connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
                gettimeofday(&arrival_time, NULL);
                pthread_mutex_lock(&mtx);
            }

            else if(strcmp(schedalg, "dh") == 0){
                Request_info oldest_request = popQueue(waiting_ptr);
                if(oldest_request.fd == 0){
                    continue;
                }
                Request_info val = {connfd, arrival_time, {-1}};
                enQueue(waiting_ptr, val );
                // 
                //printf("Request\n");
                pthread_cond_signal(&cnd);
                //
                pthread_mutex_unlock(&mtx);
                Close(oldest_request.fd);
                connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
                gettimeofday(&arrival_time, NULL);
                //printf("Request__\n");
                pthread_mutex_lock(&mtx);
            }

            else if(strcmp(schedalg, "bf") == 0){
                while(isEmpty(waiting_ptr) == 0 && isEmpty(running_ptr) == 0){
                    pthread_cond_wait(&empty_cnd, &mtx);
                }
            }

            else if(strcmp(schedalg, "random") == 0){
                int size = 0;
                int * removed = delete_random(waiting_ptr, &size);
                pthread_mutex_unlock(&mtx);
                if(removed != NULL){
                    for(int i = 0; i < size; i++){
                        Close(removed[i]);
                    }
                    free(removed);
                }
                pthread_mutex_lock(&mtx);
            }
        }

        Request_info request = {connfd, arrival_time, {-1}};
        enQueue(waiting_ptr, request);
        pthread_cond_signal(&cnd);
        pthread_mutex_unlock(&mtx);
        //Close(connfd);
        //
        // HW3: In general, don't handle the request in the main thread.
        // Save the relevant info in a buffer and have one of the worker threads
        // do the work.
        //
        //requestHandle(connfd);
    }

}

void *thread_function(void* Container){
    struct Queues_container* container = (struct Queues_container*)Container;
    struct Queue * waiting_ptr = container->m_waiting_ptr;
    struct Queue * running_ptr = container->m_running_ptr;
    int id = container->index;

    struct Threads_stats temp = {id,0,0,0};
    threads_stats t_stats = &temp;
    struct timeval working_time; // the time the thread started to work on the request
    while(1){
        pthread_mutex_lock(&mtx);
        while(isEmpty(waiting_ptr) == 1){
            //printf("%ld Waiting\n", pthread_self());
            pthread_cond_wait(&cnd, &mtx);
        }
        gettimeofday(&working_time, NULL);
        Request_info top_request_info = popQueue(waiting_ptr);
        enQueue(running_ptr, top_request_info);
        pthread_mutex_unlock(&mtx);

        timersub(&working_time, &top_request_info.arrival_time, &top_request_info.dispatch_time);

        int top_request = top_request_info.fd;

        //sleep(1);
        requestHandle(top_request_info, waiting_ptr, running_ptr, t_stats);

        pthread_mutex_lock(&mtx);
        Close(top_request);
        delete_by_value(running_ptr, top_request);
        pthread_cond_signal(&master_cnd);
        if(isEmpty(waiting_ptr) == 1 && isEmpty(running_ptr) == 1){
            pthread_cond_signal(&empty_cnd);
        }
        pthread_mutex_unlock(&mtx);
    }
}