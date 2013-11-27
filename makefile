all: test

test: buffertest.o string_queue.o int_queue.o
	g++ buffertest.o string_queue.o int_queue.o -o test

buffertest.o: buffertest.c
	g++ -c buffertest.c

string_queue.o: string_queue.c
	g++ -c string_queue.c

int_queue.o: int_queue.c
	g++ -c int_queue.c
