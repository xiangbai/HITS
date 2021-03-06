#ifndef STRING_LINKED_LIST_H
#define STRING_LINKED_LIST_H

/***************************************************************
 * string_linked_list.h
 * 
 * Implementation of linked lists for pointers c-style strings.
 * It is recommended to call string_llist_init() to clear data
 * 	before use.
 ***************************************************************/

#include <stdlib.h>

// struct string_node for internal use
typedef struct string_node {
	char *string;
	struct string_node *next;
	struct string_node *prev;
} string_node;

// struct for the linked list
typedef struct string_llist {
	string_node *front;
	string_node *back;
	size_t size;
	size_t num_chars;
} string_llist;

/*
 * Initialize a linked list.
 * Sets front and back pointers to null and size to 0.
 */
void string_llist_init(string_llist *list);

/*
 * Push a string to the front of the linked list
 */
void string_llist_push_front(string_llist *list, char *string);

/*
 * Push a string to the back of the linked list
 */
void string_llist_push_back(string_llist *list, char *string);

/*
 * Pop from the front of the linked list
 */
int string_llist_pop_front(string_llist *list, char *destination);

/*
 * Pop from the back of the linked list
 */
int string_llist_pop_back(string_llist *list, char *destination);

/*
 * Find index within the list of a given string
 */
size_t string_llist_find(string_llist *list, char *string);

/*
 * Delete any node from the linked list
 */
void string_llist_delete_node(string_llist *list, string_node **del_node);

/*
 * Print the linked list from front to back
 * Prints to stdout, one element per line.
 */
void string_llist_printforward(string_llist *list);

/*
 * Print the linked list from back to front
 * Prints to stdout, one element per line.
 */
void string_llist_printbackward(string_llist *list);

/*
 * Free a string_llist
 * This frees all string_nodes
 * NOTE: This does not free associated strings themselves
 */
void string_llist_free(string_llist *list);

/*
 * Free a string_llist and associated strings
 * NOTE: This works like string_llist_free, except it frees the strings as well
 */
void string_llist_free_all(string_llist *list);

#endif
