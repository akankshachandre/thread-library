#define _XOPEN_SOURCE_EXTENDED 1
#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include <errno.h>

 #ifdef _LP64
  #define STACK_SIZE 2097152+16384 /* large enough value for AMODE 64 */
 #else
  #define STACK_SIZE 16384  /* AMODE 31 addressing*/
 #endif
void func(int);

ucontext_t fcontext,mcontext;
int x = 0;

int main(void) {

  int  value = 1;

  getcontext(&fcontext);
  if ((fcontext.uc_stack.ss_sp = (char *) malloc(STACK_SIZE)) != NULL) {
    printf("%d",STACK_SIZE);
    fcontext.uc_stack.ss_size = STACK_SIZE;
    fcontext.uc_stack.ss_flags = 0;
			errno = 0;
    makecontext(&fcontext,func,1,value);
			if (errno != 0){
				perror("Error reported by makecontext()");
				return -1;    /* Error occurred exit */
      }
  }
  else {
    perror("not enough storage for stack");
    abort();
  }
  printf("context has been built\n");
  swapcontext(&mcontext,&fcontext);
  if (!x) {
    perror("incorrect return from swapcontext");
    abort();
  }
  else {
    printf("returned from function\n");
  }

}

void func(int arg) {

  printf("function called with value %d\n",arg);
  x++;
  printf("function returning to main\n");
  setcontext(&mcontext);

}