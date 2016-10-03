#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <errno.h>

struct My_Thread;

struct Queue {
	struct My_Thread* front;
	struct My_Thread* rear;
};

struct My_Thread {
	ucontext_t context;
	struct My_Thread* next;
	struct Queue* children;
	struct Queue* joinedOn;
	struct My_Thread* invokedOn;
	char* status;
};

struct My_Semaphore{
	int value;
	struct Queue* semQueue;
};
// Two glboal variables to store address of front and rear nodes.

void initializeQueue(struct Queue* queue);

// To Enqueue an integer
void Enqueue(struct Queue* queue,struct My_Thread* thread);

// To Dequeue an integer.
void Dequeue(struct Queue* queue);

struct My_Thread* Front(struct Queue* queue);

int isChild(struct Queue* queue,struct My_Thread* thread);

void removeFromQueue(struct Queue* queue,struct My_Thread* thread);

int isQueueEmpty(struct Queue* queue);
