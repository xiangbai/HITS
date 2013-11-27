#include "url_queue.h"
#include <stdlib.h>
#include "urlinfo.h"

/*
 * push a url to the back of a queue
 */ 
size_t push_url(struct url_queue *queue, struct urlinfo *new_url)
{
	if (!queue->size)
		queue->front = new_url;
	else
		queue->back->next = new_url;
	queue->back = new_url;
	queue->size++;
}

/*
 * pop a url from a queue
 */
struct urlinfo *pop_url(struct url_queue *queue)
{
	if (queue->size)
	{
		struct urlinfo *output = queue->front;
		queue->front = output->next;

		// if now empty, point queue.back to null
		if (!queue->size)
			queue->back = NULL;
		
		return output;
	}
	
	return NULL;
}

/*
 * print each url in a queue
 */
void print_url(struct url_queue *queue)
{
	
}
