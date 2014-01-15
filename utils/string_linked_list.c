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

void string_llist_empty(string_llist *list, int free_strings)
{
	string_node *current = list->front;
	string_node *next;
	while (current)
	{
		next = current->next;
		if (free_strings)
			free(current->string);
		free(current);
		current = next;
	}
	list->front = NULL;
	list->back = NULL;
	list->size = 0;
	list->num_chars = 0;
}

void string_llist_free(string_llist *list)
{
	string_llist_empty(list, 0);
	free(list);
}

void string_llist_free_all(string_llist *list)
{
	string_llist_empty(list, 1);
	free(list);
}

void string_llist_push_front(string_llist *list, char *string)
{
	// create new node
	string_node *node = (string_node *) malloc(sizeof(string_node));
	char *newstring = malloc(sizeof(char) * strlen(string) + 1);
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
	char *newstring = malloc(sizeof(char) * strlen(string) + 1);
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

size_t string_llist_find(string_llist *list, char *string)
{
	size_t output = -1;
	size_t index = 0;
	string_node *node = list->front;
	
	while (node)
	{
		if (!strcmp(node->string, string))
		{
			output = index;
			break;
		}
		node = node->next;
		index++;
	}
	return output;
}

void string_llist_delete_node(string_llist *list, string_node **del_node)
{
    string_node *node = *del_node;
    if (node != NULL)
    {
        if (node->prev == NULL && node->next == NULL) //1 item in list
        {
            list->front = NULL;
            list->back = NULL;
        }
        else if (node->prev == NULL) //if first item in list
        {
            list->front = node->next;
            node->next->prev = NULL;
        }
        else if (node->next == NULL) //if last item in list
        {
            list->back = node->prev;
            node->prev->next = NULL;
        }
        else    //in middle of list
        {
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }
        list->num_chars -= strlen(node->string);
        list->size--;
        free(node->string);
        free(node);
    }
}

void string_llist_printforward(string_llist *list)
{
	printf("printing %d strings:\n", list->size);
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
