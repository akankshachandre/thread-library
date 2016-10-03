mythread.a: mythread.o queue.o
	ar rcs mythread.a mythread.o queue.o 
	
mythread.o: mythread.c mythreadextra.h queue.c queue.h
	gcc -c mythread.c queue.c

queue.o: queue.c queue.h
	gcc -c queue.c -o queue.o