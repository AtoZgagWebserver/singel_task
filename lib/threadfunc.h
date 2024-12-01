#include "headerlist.h"

#ifndef THREADSTRUCT
#define THREADSTRUCT

#define QUEUE_SIZE 24

typedef struct {
    int* client_sockets[QUEUE_SIZE];  // 포인터를 저장
    int front, rear, count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} TaskQueue;

#endif

void init_queue(TaskQueue *q);
void enqueue(TaskQueue *q, int *client_socket);
int* dequeue(TaskQueue *q);
void *client();