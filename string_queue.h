#include <stdlib.h>

typdef struct
{
	char *string;
	struct string_node *next;
} string_node;


typedef struct 
{
	struct string_node *front;
	struct string_node *back;
	size_t size;
} string_queue;

void string_queue_push(struct string_queue *queue, char *string);

char *string_queue_pop(struct string_queue *queue);
