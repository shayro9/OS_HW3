
#ifndef __QUEUE_HEADER__H

#include "segel.h"

typedef struct request_info{
	int fd;
	struct timeval time_info;
} Request_info;

// A linked list (LL) node to store a queue entry
struct QNode {
	Request_info key;
	struct QNode* next;
};

// The queue, front stores the front node of LL and rear
// stores the last node of LL
struct Queue {
    unsigned int size;
	struct QNode *front, *rear;
};

int isEmpty(struct Queue* q);
struct Queue* createQueue();
struct QNode* newNode(Request_info val);
void enQueue(struct Queue* q, Request_info val);
void deQueue(struct Queue* q);
Request_info popQueue(struct Queue* q);
int delete_by_value(struct Queue* q, int target);
int* delete_random(struct Queue* q,int* size);

#endif // __QUEUE_HEADER__H