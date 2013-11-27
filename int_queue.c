#include "int_queue.h"
#include <stdlib.h>

void int_queue_push(struct int_queue *queue, int *number)
{
	// create new node
	struct int_node *node = (struct int_node *) malloc(sizeof(struct int_node));
	node->number = number;
	node->next = NULL;

	// put it into the queue
	if (!queue->size)
		queue->front = node;
	else				// if non-empty, point back.next to node
		queue->back->next = node;
	queue->back = node;
	queue->size++;
}

int *int_queue_pop(struct int_queue *queue)
{
	if (queue->size)
	{
		// remove front element
		struct int_node *node = queue->front;
		queue->front = node->next;
		queue->size--;

		// if now empty, point queue.back to null
		if (!queue->size)
			queue->back = NULL;
		
		// get the int
		int *output = node->number;
		
		// free the struct
		free(node);

		return output;
	}

	return NULL;
}
