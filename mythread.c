/*
 * mythread.c

 *
 *  Created on: Sep 24, 2016
 *      Author: akanksha
 */
#include "queue.h"
#include "mythread.h"

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)


#define STACK_SIZE 8192

struct My_Thread *current;

struct Queue *children;
struct Queue *joinedOn;

ucontext_t *fcontext;
ucontext_t *maincontext;
ucontext_t *exitcontext;

struct Queue *readyQueue;
struct Queue *blockedQueue;

struct My_Semaphore *mysem;
struct Queue *mysemQueue;

/**
 * Creates the new thread
 * Sets the parameters context,children,status,invokedOn and joinedOn for the thread
 * No Return value
 */
struct My_Thread* makeThread(){
	struct My_Thread *newthread;

	newthread = malloc(sizeof(struct My_Thread));
	fcontext = (ucontext_t *)malloc(sizeof(ucontext_t));
	exitcontext = (ucontext_t *)malloc(sizeof(ucontext_t));


	getcontext(exitcontext);

	if ((exitcontext->uc_stack.ss_sp = (char *) malloc(STACK_SIZE)) != NULL) {
	  		exitcontext->uc_stack.ss_size = STACK_SIZE;
	  		exitcontext->uc_stack.ss_flags = 0;
	  		errno = 0;
	}

	makecontext(exitcontext,(void*)MyThreadExit,1);

	if (errno != 0){
	  	  perror("Error reported by makecontext()");
	  	  exit(EXIT_FAILURE);
	 }

	getcontext(fcontext);
  	if ((fcontext->uc_stack.ss_sp = (char *) malloc(STACK_SIZE)) != NULL) {
	    fcontext->uc_stack.ss_size = STACK_SIZE;
	    fcontext->uc_stack.ss_flags = 0;
	    fcontext->uc_link = exitcontext;
				errno = 0;
  	}



	else {
		perror("not enough storage for stack");
		abort();
	}

	newthread->context = *fcontext;
	newthread->status = "new";

	children = malloc(sizeof(struct Queue));
	newthread->children = children;

	joinedOn = malloc(sizeof(struct Queue));
	newthread->joinedOn = joinedOn;


	return newthread;
}

/**
 * Free the thread blocked by the exitingthread
 * Parameters: exitingthread
 * No return value
 */
void freeBlockedThread(struct My_Thread *exitingthread){
	struct My_Thread *blockedThread = exitingthread->invokedOn;

	removeFromQueue(blockedThread->joinedOn,exitingthread);
	removeFromQueue(blockedThread->children,exitingthread);

	if(isQueueEmpty(blockedThread->joinedOn)==0){
		removeFromQueue(blockedQueue,blockedThread);
		Enqueue(readyQueue,blockedThread);
	}

}

/**
 * Update the invokedOn property of the thread when join is called
 * Parameters: joiningthread, currentthread
 * No return value
 */
void updateInvokedOn(struct My_Thread *readyThread,struct My_Thread *invokedOn){
	struct My_Thread *thread = readyQueue->front;
	while(thread!=NULL){
		if(thread==readyThread){
			thread->invokedOn = invokedOn;
			break;
		}
		thread = thread->next;

	}
}

/**
 * Creates the main thread
 * Invoked by the unix process
 * Parameters: function in which the thread will execute and arguments to the function
 */
void MyThreadInit(void(*start_funct)(void *), void *args){
	maincontext = (ucontext_t *)malloc(sizeof(ucontext_t));

	current = makeThread();

	readyQueue = malloc(sizeof(struct Queue));
	initializeQueue(readyQueue);

	blockedQueue = malloc(sizeof(struct Queue));
	initializeQueue(blockedQueue);


	makecontext(&current->context,(void*)start_funct,1,args);
				if (errno != 0){
					perror("Error reported by makecontext()");
					exit(EXIT_FAILURE);
	}

	swapcontext(maincontext,&current->context);
}

/**
 * Creates a new thread and enqueues it in readyQueue
 * Parameters: function in which the thread will execute and arguments to the function
 */
void* MyThreadCreate(void(*start_funct)(void *), void *args){
	struct My_Thread *newthread = makeThread();

	makecontext(&newthread->context,(void*)start_funct,1,args);
				if (errno != 0){
					perror("Error reported by makecontext()");
					exit(EXIT_FAILURE);
	}

	Enqueue(current->children,newthread);
	Enqueue(readyQueue,newthread);
	return (void*)newthread;
}

/**
 * Yield to the next thread in the readyQueue
 * The current thread is added to the readyQueue
 */
void MyThreadYield(){
	struct My_Thread *yieldingthread = current;

	current = Front(readyQueue);
	Dequeue(readyQueue);

	Enqueue(readyQueue,yieldingthread);

	swapcontext(&(yieldingthread->context),&(current->context));

}

/**
 * Join the current thread on all its children
 */
void MyThreadJoinAll(void){

	struct Queue *children = current->children;
	struct My_Thread *child = children->front;

	MyThreadJoin(child);
	child = child->next;

	if(child!=NULL){
		MyThreadJoin(child);
		child = child->next;
	}
}

/**
 * Join the current thread on the joining thread
 * Parameters: Joining thread
 * Return value : 0 if join was successful else -1
 */
int MyThreadJoin(void* thread){
	struct My_Thread *jointhread = thread;
	int ischild = -1;

	if(strcmp(jointhread->status,"Terminated")!=0){

		ischild = isChild(current->children,jointhread);

		if(ischild==0){
			updateInvokedOn(jointhread,current);

			ucontext_t *currctxt = &current->context;
			Enqueue(current->joinedOn,jointhread);


			Enqueue(blockedQueue,current);
			current = Front(readyQueue);
			Dequeue(readyQueue);
			swapcontext(currctxt,&(current->context));
		}
	}
	return ischild;

}
/**
 * Called explicitly by every thread
 * Schedules the next thread for execution
 */
void MyThreadExit(){

	struct My_Thread *exitingthread = current;
	exitingthread->status = "Terminated";

		if(exitingthread->invokedOn!=NULL){
			freeBlockedThread(exitingthread);
		}

		current = Front(readyQueue);

		if(current!=NULL){
			Dequeue(readyQueue);
			setcontext(&(current->context));
		}
		else{
			setcontext(maincontext);
		}
}
/**
 * Creates a thread for the unix process
 */
void MyThreadInitExtra(){
	struct My_Thread *mainthread = makeThread();

	readyQueue = malloc(sizeof(struct Queue));
	initializeQueue(readyQueue);

	blockedQueue = malloc(sizeof(struct Queue));
	initializeQueue(blockedQueue);

	current = mainthread;

	swapcontext(&current->context,&current->context);
}
/**
 *Initialize a semaphore with the given value
 */
MySemaphore MySemaphoreInit(int initialValue){

	mysem = malloc(sizeof(struct My_Semaphore));
	mysem->value = initialValue;

	mysemQueue = malloc(sizeof(struct Queue));
	initializeQueue(mysemQueue);

	mysem->semQueue = mysemQueue;

	return (void*)mysem;


}
/**
 * Increase the semaphore value by 1
 * If any thread in blockedQueue of semaphore release the first thread and add it to readyQueue
 */
void MySemaphoreSignal(void* sem){

	struct My_Thread *unblock;
	struct My_Semaphore *mysem = sem;

	mysem->value++;

	if(mysem->value <= 0) {
			unblock = Front(mysem->semQueue);
			Dequeue(mysem->semQueue);
			Enqueue(readyQueue,unblock);
	}

}

/**
 * Decrease the semaphore value by 1
 * If value is less than 0, then add the current thread to the blockedQueue of semaphore
 */
void MySemaphoreWait(void* sem){

	struct My_Semaphore *mysem = sem;


		mysem->value--;

		if(mysem->value < 0){

			ucontext_t *currctxt = &(current->context);
			Enqueue(mysem->semQueue,current);

			current = Front(readyQueue);
			Dequeue(readyQueue);

			swapcontext(currctxt,&(current->context));
		}

}
/**
 * Destroy the semaphore if no threads in blockedQueue
 * return 0 on success and -1 on failure
 */
int MySemaphoreDestroy(void* sem){
	struct My_Semaphore *mysem = sem;

	if(mysem->value >= 0)
		return 0;
	else
		return -1;
}
