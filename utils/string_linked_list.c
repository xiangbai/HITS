#include "string_linked_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void string_llist_init(string_llist *list)
{
	// set pointers to null and size to 0
	list->front = NULL;
	list->back = NULL;
	list->size = 0;
}

void string_llist_push_front(string_llist *list, char *string)
{
	// create new node
	string_node *node = (string_node *) malloc(sizeof(string_llist));
	node->string = string;
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

void string_llist_push_back(string_llist *list, char *string)
{
	// create new node
	string_node *node = (string_node *) malloc(sizeof(char) * (strlen(string) + 1));
	node->string = string;
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

char *string_llist_pop_front(string_llist *list)
{
	if (list->size)
	{
		// remove elements
		string_node *node = list->front;
		list->front = node->next;
		list->size--;	
		// if there is a first node, point its prev to null
		if (list->size)
			list->front->prev = NULL;
		else
			list->back = NULL;

		// get the string
		char *output = node->string;
		
		// free the node
		free(node);

		return output;
	}
	return NULL;
}

char *string_llist_pop_back(string_llist *list)
{
	if (list->size)
	{
		// remove elements
		string_node *node = list->back;
		list->back = node->prev;
		list->size--;	
		// if there is a first node, point its prev to null
		if (list->size)
			list->back->next = NULL;
		else
			list->front = NULL;

		// get the string
		char *output = node->string;
		
		// free the node
		free(node);

		return output;
	}
	return NULL;
}


size_t string_llist_count_chars(string_llist *list)
{
	size_t count = 0;
	string_node *node = list->front;

	// traverse the list, adding to the count
	while (node != NULL)
	{
		count += strlen(node->string);
		node = node->next;
	}
	return count;
}

void string_llist_printforward(string_llist *list)
{
	string_node *node = list->front;
	while (node != NULL)
	{
		fputs(node->string, stdout);
		fputc('\n', stdout);
		node = node->next;
	}
}
void string_llist_printbackward(string_llist *list)
{
	string_node *node = list->back;
	while (node != NULL)
	{
		fputs(node->string, stdout);
		fputc('\n', stdout);
		node = node->prev;
	}
}
