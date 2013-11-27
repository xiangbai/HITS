/****************************
 * urlqueue.h
 * Holds struct urlqueue
 *  and methods for using it
 ****************************/

struct urlqueue
{
	urlinfo *front;
	urlinfo *back;
	size_t length;
};

/*
 * push a url to the back of a queue
 */ 
size_t push_url(struct urlqueue *queue, struct urlinfo *new_url);

/*
 * pop a url from a queue
 */
struct urlinfo pop_url(struct urlqueue *queue);

/*
 * print each url in a queue
 */
void print_url(struct urlqueue *queue);
