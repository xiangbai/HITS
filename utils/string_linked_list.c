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
	list->num_chars = 0;
}

void string_llist_push_front(string_llist *list, char *string)
{
	// create new node
	string_node *node = (string_node *) malloc(sizeof(string_node));
	char *newstring = malloc(sizeof(char) * strlen(string));
	strcpy(newstring, string);
	node->string = newstring;
	node->prev = NULL;

	// put it into the list
	if (!list->size)
		list->back = node;
	else
		list->front->prev = node;
	node->next = list->front;
	list->front = node;
	list->size++;
	list->num_chars += strlen(string);
}

void string_llist_push_back(string_llist *list, char *string)
{
	// create new node
	string_node *node = (string_node *) malloc(sizeof(string_node));
	char *newstring = malloc(sizeof(char) * strlen(string));
	strcpy(newstring, string);
	node->string = newstring;
	node->next = NULL;

	// put it into the list
	if (!list->size)
		list->front = node;
	else
		list->back->next = node;
	node->prev = list->back;
	list->back = node;
	list->size++;
	list->num_chars += strlen(string);
}

int string_llist_pop_front(string_llist *list, char *destination)
{
	if (list->size)
	{
		// remove elements
		string_node *node = list->front;
		list->front = node->next;
		list->size--;
		list->num_chars -= strlen(node->string);

		// if there is a first node, point its prev to null
		if (list->size)
			list->front->prev = NULL;
		else
			list->back = NULL;

		// get the string
		strcpy(destination, node->string);
		
		// free the node
		free(node->string);
		free(node);

		return 0;
	}
	return -1;
}

int string_llist_pop_back(string_llist *list, char *destination)
{
	if (list->size)
	{
		// remove elements
		string_node *node = list->back;
		list->back = node->prev;
		list->size--;
		list->num_chars -= strlen(node->string);

		// if there is a first node, point its prev to null
		if (list->size)
			list->back->next = NULL;
		else
			list->front = NULL;

		// get the string
		strcpy(destination, node->string);
		
		// free the node
		free(node->string);
		free(node);

		return 0;
	}
	return -1;
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
