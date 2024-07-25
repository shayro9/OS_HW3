// A C program to demonstrate linked list based
// implementation of queue
#include <stdio.h>
#include <stdlib.h>
#include "Queue.h"


struct QNode* newNode(int k)
{
	struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
	temp->key = k;
	temp->next = NULL;
	return temp;
}

// A utility function to create an empty queue
struct Queue* createQueue()
{
	struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
	q->front = q->rear = NULL;
	q->size  = 0;
	return q;
}

// The function to add a key k to q
void enQueue(struct Queue* q, int k)
{
	// Create a new LL node
	struct QNode* temp = newNode(k);
	(q->size) += 1;

	// If queue is empty, then new node is front and rear
	// both
	if (q->rear == NULL) {
		q->front = q->rear = temp;
		return;
	}

	// Add the new node at the end of queue and change rear
	q->rear->next = temp;
	q->rear = temp;
}

// Function to remove a key from given queue q
void deQueue(struct Queue* q)
{
	// If queue is empty, return NULL.
	if (q->front == NULL)
		return;

	// Store previous front and move front one node ahead
	struct QNode* temp = q->front;

	q->front = q->front->next;

	// If front becomes NULL, then change rear also as NULL
	if (q->front == NULL)
		q->rear = NULL;

	free(temp);
	(q->size)--;
}

int isEmpty(struct Queue* q){
	// If queue is empty, return 1
	return (q->front == NULL) ? 1 : 0;
}

int popQueue(struct Queue* q){
	int tmp = q->front != NULL ? q->front->key : 0;
	deQueue(q);
	return tmp;
}

int delete_by_value(struct Queue* q, int target){
	if(q == NULL){
		return 1;
	}
	struct QNode *node = q->front, *prev = q->front;
	if(node != NULL && node->key == target){
		if(q->rear == q->front){
			q->rear = NULL;
		}
		q->front = q->front->next;
		free(node);
		(q->size)--;
		return 0;
	}
	while(node != NULL){
		if(node->key == target){
			prev->next = node->next;
			free(node);
			if(q->rear == node){
				q->rear = prev;
			}
			(q->size)--;
			return 0;
		}
		prev = node;
		node = node->next;
	}
	return 1;
}
