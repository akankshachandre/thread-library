#include "queue.h"
#include "mythread.h"

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)


#define STACK_SIZE 16384

struct My_Thread *current; 
struct My_Thread *unixPthread;
ucontext_t *fcontext;
ucontext_t *mcontext;
struct Queue *readyQueue;
struct Queue *blockedQueue;
struct Queue *children;

struct My_Thread* createThread(){
	struct My_Thread *newthread;

	newthread = malloc(sizeof(struct My_Thread));
	fcontext = (ucontext_t *)malloc(sizeof(ucontext_t));
	
	getcontext(fcontext);
  	if ((fcontext->uc_stack.ss_sp = (char *) malloc(STACK_SIZE)) != NULL) {
	    fcontext->uc_stack.ss_size = STACK_SIZE;
	    fcontext->uc_stack.ss_flags = 0;
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

	return newthread;
}

void freeBlockedThread(struct My_Thread *exitingthread){
	struct My_Thread *blockedThread = exitingthread->invokedOn;
	struct Queue *children = blockedThread->joinedOn;
	removeFromQueue(children,exitingthread);

	if(isQueueEmpty(blockedThread->children)==0){
		removeFromQueue(blockedQueue,blockedThread);
		Enqueue(readyQueue,blockedThread);
	}
}

void deallocate(struct My_Thread *exitingthread){
	free(exitingthread->context.uc_stack.ss_size);
	free(exitingthread->context.uc_stack.ss_sp);
	free(exitingthread->next);
	free(exitingthread->children);
	free(exitingthread->joinedOn);
	free(exitingthread->invokedOn);
	/*exitingthread->next = NULL;
	exitingthread->children = NULL;
	exitingthread->joinedOn = NULL;
	exitingthread->invokedOn = NULL;*/

}

void MyThreadInit(void(*start_funct)(void *), void *args){
	current = createThread();

	unixPthread = current;

	readyQueue = malloc(sizeof(struct Queue));
	initializeQueue(readyQueue);

	blockedQueue = malloc(sizeof(struct Queue));
	initializeQueue(blockedQueue);


	makecontext(&current->context,(void*)start_funct,1,args);
				if (errno != 0){
					perror("Error reported by makecontext()");
					exit(EXIT_FAILURE);
	}

	mcontext = (ucontext_t *)malloc(sizeof(ucontext_t));
	swapcontext(mcontext,&current->context);
}

void* MyThreadCreate(void(*start_funct)(void *), void *args){
	struct My_Thread *newthread = createThread();
	
	makecontext(&newthread->context,(void*)start_funct,1,args);
				if (errno != 0){
					perror("Error reported by makecontext()");
					exit(EXIT_FAILURE);
	}
	Enqueue(current->children,newthread);
	Enqueue(readyQueue,newthread);
	return (void*)newthread;
}

void MyThreadYield(){
	struct My_Thread *yieldingthread = current;
	current = Front(readyQueue);
	Dequeue(readyQueue);
	Enqueue(readyQueue,yieldingthread);
	swapcontext(&(yieldingthread->context),&(current->context));
}

int MyThreadJoin(void* thread){
	struct My_Thread *jointhread = thread;
	jointhread->invokedOn = malloc(sizeof(struct My_Thread));
	int ischild = 0;

	if(strcmp(jointhread->status,"Terminated")!=0){
	printf("joining thread has not terminated");
	jointhread->invokedOn = current;

		ischild = isChild(current->children,jointhread);
		
		if(ischild==0){
			ucontext_t *currctxt = &current->context;

			if(current->joinedOn==NULL){
				current->joinedOn = malloc(sizeof(struct Queue));
			}

			Enqueue(current->joinedOn,jointhread);

			Enqueue(blockedQueue,current);
			current = jointhread;
			swapcontext(currctxt,&(jointhread->context));
		}
	}
	else{
		printf("joining thread has Terminated");
	}
	return ischild;

}

void MyThreadExit(){

	struct My_Thread *exitingthread = current;
	exitingthread->status = "Terminated";
		if(exitingthread->invokedOn!=NULL){
			freeBlockedThread(exitingthread);
		}
		//deallocate(exitingthread);
		current = Front(readyQueue);
		Dequeue(readyQueue);
		swapcontext(&(exitingthread->context),&(current->context));
}

int n, m;
int yield = 0;
int join = 0;

void t2(void * who)
{
  printf("t2 %d start\n", (int)who);
  MyThreadExit();
}

void t1(void *);

int makeThreads(char *me, void (*t)(void *), int many)
{
  MyThread T;
  int i;
  for (i = 0; i < many; i++) {
    printf("%s create %d\n", me, i);
    T = MyThreadCreate(t, (void *)i);
    if (yield)
      MyThreadYield();      
  }
  if (join)
    MyThreadJoin(T);
}

void t1(void * who_)
{
  char me[16];
  int who = (int)who_;
  sprintf(me, "t1 %d", who);
  printf("%s start\n", me);
  makeThreads(me, t2, m);
  printf("t1 %d end\n", who);
  MyThreadExit();
}

void t0(void * dummy)
{
  printf("t0 start\n");
  makeThreads("t0", t1, n);
  printf("t0 end\n");
  MyThreadExit();
}

int main(int argc, char *argv[])
{
  if (argc != 5) {
    return -1;
  }
  n = atoi(argv[1]);
  m = atoi(argv[2]);
  yield = atoi(argv[3]);
  join = atoi(argv[4]);

  MyThreadInit(t0, NULL);
}