// A C program to demonstrate linked list based
// implementation of queue

#include "Queue.h"


struct QNode* newNode(Request_info val)
{
	struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
	temp->key = val;
	
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
void enQueue(struct Queue* q, Request_info val)
{
	// Create a new LL node
	struct QNode* temp = newNode(val);
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

Request_info popQueue(struct Queue* q){
	Request_info tmp;
	memset(&tmp, 0, sizeof(tmp));
	if(q->front != NULL){ 
		tmp = q->front->key; 
	}
	deQueue(q);
	return tmp;
}

void deQueueFromEnd(struct Queue *q) {

    if (q->front == NULL) {
        return; 
    }

    if (q->front == q->rear) {
        struct QNode *temp = q->front;
        free(temp);
        q->front = NULL;
		q->rear = NULL;
        q->size = 0;
        return;
    }

    struct QNode *current = q->front;
    while (current->next != q->rear) {
        current = current->next;
    }

    // 'current' is now the second last node
    struct QNode *temp = q->rear;
    free(temp);
    q->rear = current;
    q->rear->next = NULL;
    q->size--;
}

Request_info popQueueFromEnd(struct Queue* q){
	Request_info tmp;
	memset(&tmp, 0, sizeof(tmp));
	if(q->rear != NULL){ 
		tmp = q->rear->key; 
	}
	deQueueFromEnd(q);
	return tmp;
}

int delete_by_value(struct Queue* q, int target){
	if(q == NULL){
		return 1;
	}
	struct QNode *node = q->front, *prev = q->front;
	if(node != NULL && node->key.fd == target){
		if(q->rear == q->front){
			q->rear = NULL;
		}
		q->front = q->front->next;
		free(node);
		(q->size)--;
		return 0;
	}
	while(node != NULL){
		if(node->key.fd == target){
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

int* delete_random(struct Queue* q, int* size) {
    if (q == NULL || q->size == 0) return NULL;

    int *arr = (int*)calloc(q->size, sizeof(int));
    if (arr == NULL) return NULL;

    struct QNode *node = q->front;
    int counter = 0;
    while (node != NULL) {
        arr[counter++] = node->key.fd;
        node = node->next;
    }
    srand(time(NULL));  
    int counter_removed = 0, target_removals = (counter+1)/2;
	int *res = (int*)calloc(target_removals, sizeof(int));
	if(res == NULL){
		free(arr);
		return NULL;
	}
    while (counter_removed < target_removals) {
        int tmp = rand() %counter;
        if (arr[tmp] != 0) { 
            delete_by_value(q, arr[tmp]);
            res[counter_removed++] = arr[tmp];
            arr[tmp] = 0;
        }
    }
    free(arr);
	(*size) = counter_removed;
	return res;
}

