#include "llist.h"
#include <stdlib.h>
#include <stdio.h>

void llist_init(llist *list, int (*equalsfunction)(void *a, void *b))
{
	// set pointers to null and size to 0
	list->front = NULL;
	list->back = NULL;
	list->equalsfunction;
	list->size = 0;
}

void llist_push_front(llist *list, void *data)
{
	// create new node
	lnode *node = (lnode *) malloc(sizeof(lnode));
	node->data = data;
	node->prev = NULL;

	// put it into the list
	if (!list->size)
		list->back = node;
	else
		list->front->prev = node;

	node->next = list->front;
	list->front = node;
	list->size++;
}

void llist_push_back(llist *list, void *data)
{
	// create new node
	lnode *node = (lnode *) malloc(sizeof(lnode));
	node->data = data;
	node->next = NULL;
	
	// put it into the list
	if (!list->size)
		list->front = node;
	else
		list->back->next = node;
	
	node->prev = list->back;
	list->back = node;
	list->size++;
}

void *llist_pop_front(llist *list)
{
	void *data = NULL;
	
	if (list->size)
	{
		// remove elements
		lnode *node = list->front;
		list->front = node->next;
		list->size--;

		// if there is a first node, point its prev to null
		if (list->size)
			list->front->prev = NULL;
		else
			list->back = NULL;

		// get the data
		data = node->data;
		
		// free the node
		free(node);
	}
	return data;
}

void *llist_pop_back(llist *list)
{
	void *data = NULL;
	
	if (list->size)
	{
		// remove elements
		lnode *node = list->back;
		list->back = node->prev;
		list->size--;

		// if there is a first node, point its prev to null
		if (list->size)
			list->back->next = NULL;
		else
			list->front = NULL;

		// get the data
		data = node->data;
		
		// free the node
		free(node);
	}
	return data;
}

int llist_find(llist *list, void *obj)//, int (*equalsfunction)(void *a, void *b))
{
	// begin by assuming it is not found
	int output = 0;

	// iterate through list
	lnode *current = list->front;
	while (current)
	{
		// mark as found and break if lnode.data == obj
		//if ((*equalsfunction)(current->data, obj))
		if (list->equalsfunction(current->data, obj))
		{
			output = 1;
			break;
		}
		current = current->next;
	}

	return output;
}
