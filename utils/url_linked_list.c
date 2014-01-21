#include "url_linked_list.h"
#include "../urlinfo.h"
#include <stdlib.h>
#include <stdio.h>

void url_llist_init(url_llist *list)
{
	// set pointers to null and size to 0
	list->front = NULL;
	list->back = NULL;
	list->size = 0;
}

void url_llist_push_front(url_llist *list, urlinfo *url)
{
	// create new node
	url_node *node = (url_node *) malloc(sizeof(url_node));
	node->url = url;
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

void url_llist_push_back(url_llist *list, urlinfo *url)
{
	// create new node
	url_node *node = (url_node *) malloc(sizeof(url_node));
	node->url = url;
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

urlinfo *url_llist_pop_front(url_llist *list)
{
	urlinfo *url = NULL;
	
	if (list->size)
	{
		// remove elements
		url_node *node = list->front;
		list->front = node->next;
		list->size--;

		// if there is a first node, point its prev to null
		if (list->size)
			list->front->prev = NULL;
		else
			list->back = NULL;

		// get the url
		url = node->url;
		
		// free the node
		free(node);
	}
	return url;
}

urlinfo *url_llist_pop_back(url_llist *list)
{
	urlinfo *url = NULL;
	
	if (list->size)
	{
		// remove elements
		url_node *node = list->back;
		list->back = node->prev;
		list->size--;

		// if there is a first node, point its prev to null
		if (list->size)
			list->back->next = NULL;
		else
			list->front = NULL;

		// get the url
		url = node->url;
		
		// free the node
		free(node);
	}
	return url;
}

void url_llist_free(url_llist *list)
{
	url_node *current = list->front;
	url_node *next;
	
	// traverse list, removing and freeing url_nodes
	while (current)
	{
		next = current->next;
		free(current);
		current = next;
	}
	
	// list itself is not freed, as it may be on the stack
}

void url_llist_free_all(url_llist *list)
{
	url_node *current = list->front;
	url_node *next;
	
	// traverse list, removing and freeing url_nodes
	while (current)
	{
		next = current->next;
		free(current->url);
		free(current);
		current = next;
	}

	// free list
	free(list);
}
