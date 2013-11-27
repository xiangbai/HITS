#include "string_queue.h"
#include <stdlib.h>

void string_queue_push(struct string_queue *queue, char *string)
{
	// create new node
	struct string_node *node = (struct string_node *) malloc(sizeof(struct string_node));
	node->string = string;
	node->next = NULL;

	// put it into the queue
	if (!queue->size)
		queue->front = node;
	else				// if non-empty, point back.next to node
		queue->back->next = node;
	queue->back = node;
	queue->size++;
}

char *string_queue_pop(struct string_queue *queue)
{
	if (queue->size)
	{
		// remove front element
		struct string_node *node = queue->front;
		queue->front = node->next;
		queue->size--;

		// if now empty, point queue.back to null
		if (!queue->size)
			queue->back = NULL;
		
		// get the string
		char *output = node->string;
		
		// free the struct
		free(node);

		return output;
	}

	return NULL;
}
