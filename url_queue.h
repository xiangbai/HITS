/****************************
 * urlqueue.h
 * Holds struct urlqueue
 *  and methods for using it
 ****************************/

#include <stdlib.h>
#include "urlinfo.h"

struct url_queue
{
	struct urlinfo *front;
	struct urlinfo *back;
	size_t size;
};

/*
 * push a url to the back of a queue
 */ 
size_t push_url(struct url_queue *queue, struct urlinfo *new_url);

/*
 * pop a url from a queue
 */
struct urlinfo *pop_url(struct url_queue *queue);

/*
 * print each url in a queue
 */
void print_url(struct url_queue *queue);
