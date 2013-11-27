all: test

test: buffertest.o string_queue.o int_queue.o
	gcc buffertest.o string_queue.o int_queue.o -o test

buffertest.o: buffertest.c
	gcc -c buffertest.c

string_queue.o: string_queue.c
	gcc -c string_queue.c

int_queue.o: int_queue.c
	gcc -c int_queue.c
