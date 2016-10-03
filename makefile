mythread.a: mythread.o queue.o
	ar rcs mythread.a mythread.o queue.o 
	
mythread.o: 
	gcc -c mythread.c queue.c

queue.o:
	gcc -c queue.c
