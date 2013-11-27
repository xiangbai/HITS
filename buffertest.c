#include <stdio.h>
#include "string_queue.h"
#include "int_queue.h"

int main(void)
{
	// test strings
	struct string_queue queue;
	queue.size = 0;	
	fputs("::generating strings\n", stdout);
	
	char word1[] = "hello";
	char word2[] = "world";
	char word3[] = "three";
	char word4[] = "four!";

	fputs("::pushing strings\n", stdout);
		
	string_queue_push(&queue, word1);
	string_queue_push(&queue, word2);
	string_queue_push(&queue, word3);
	string_queue_push(&queue, word4);

	fputs("::printing strings\n", stdout);
	
	while(queue.size)
		printf("%s\n", 	string_queue_pop(&queue));

	// test ints	
	struct int_queue intqueue;
	intqueue.size = 0;
	fputs("::generating ints\n", stdout);
	
	int int1 = 1;
	int int2 = 2;
	int int3 = 3;
	int int4 = 5;
	int int5 = 8;

	fputs("::pushing ints\n", stdout);
		
	int_queue_push(&intqueue, &int1);
	int_queue_push(&intqueue, &int2);
	int_queue_push(&intqueue, &int3);
	int_queue_push(&intqueue, &int4);
	int_queue_push(&intqueue, &int5);

	fputs("::printing ints\n", stdout);	
	while(intqueue.size)
		printf("%d\n", 	*int_queue_pop(&intqueue));
	
	return 0;
}
