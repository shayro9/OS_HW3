
#include "segel.h"


// A linked list (LL) node to store a queue entry
struct QNode {
	int key;
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
struct QNode* newNode(int k);
void enQueue(struct Queue* q, int k);
void deQueue(struct Queue* q);
int popQueue(struct Queue* q);
int delete_by_value(struct Queue* q, int target);
int* delete_random(struct Queue* q,int* size);