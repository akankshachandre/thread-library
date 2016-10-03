/*
 * queue.c
 *
 *  Created on: Sep 24, 2016
 *      Author: akanksha
 */
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <errno.h>

#include "queue.h"

void initializeQueue(struct Queue* queue){
	queue->front = malloc(sizeof(struct My_Thread*));
	queue->rear = malloc(sizeof(struct My_Thread*));
	queue->front = NULL;
	queue->rear = NULL;
}

void Enqueue(struct Queue* queue,struct My_Thread* thread) {
	if(queue->front==NULL){
		queue->front = thread;
		queue->rear = thread;
	}
	else{
		queue->rear->next = thread;
		queue->rear = thread;
	}
}

void Dequeue(struct Queue* queue){
	struct My_Thread* temp = queue->front;
	if(queue->front == NULL) {
		printf("Queue is Empty\n");
	}
	if(queue->front == queue->rear)
		queue->front = queue->rear = NULL;
	else
		queue->front = queue->front->next;
}



struct My_Thread* Front(struct Queue* queue) {
	if(queue->front == NULL) {
		return NULL;
	}
	return queue->front;
}

int isQueueEmpty(struct Queue* queue){
	if(queue->front==NULL){
		return 0;
	}
	else
		return -1;
}



int isChild(struct Queue* queue,struct My_Thread* thread){
	if(queue->front==NULL)
		return -1;

	else{
		struct My_Thread *curr = queue->front;
		while(curr!=NULL){
			if(curr==thread){
				return 0;
				break;
			}
			curr = curr->next;
		}
	}
	return -1;
}

void removeFromQueue(struct Queue* queue,struct My_Thread* thread){
	struct My_Thread* curr = queue->front;

	if(queue->front==thread || queue->front==NULL)
		queue->front=NULL;

	else{
		while(curr!=NULL){
			if(curr->next==thread){
				curr->next = curr->next->next;
				break;
			}
			curr = curr->next;
		}
	}

}


