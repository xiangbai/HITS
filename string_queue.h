#include <stdlib.h>

struct string_node
{
	char *string;
	struct string_node *next;
};


struct string_queue
{
	struct string_node *front;
	struct string_node *back;
	size_t size;
};

void string_queue_push(struct string_queue *queue, char *string);

char *string_queue_pop(struct string_queue *queue);
