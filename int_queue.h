#include <stdlib.h>

struct int_node
{
	int *number;
	struct int_node *next;
};


struct int_queue
{
	struct int_node *front;
	struct int_node *back;
	size_t size;
};

void int_queue_push(struct int_queue *queue, int *number);

int *int_queue_pop(struct int_queue *queue);
