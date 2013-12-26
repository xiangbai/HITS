#ifndef URL_LINKED_LIST_H
#define URL_LINKED_LIST_H

/***************************************************************
 * url_linked_list.h
 * 
 * Implementation of linked lists for urlinfo structs.
 * It is recommended to call url_llist_init() to clear data
 * 	before use.
 ***************************************************************/

#include "../urlinfo.h"
#include <stdlib.h>

// struct url_node for internal use
typedef struct url_node {
	urlinfo *url;
	struct url_node *next;
	struct url_node *prev;
} url_node;

// struct for the linked list
typedef struct url_llist {
	struct url_node *front;
	struct url_node *back;
	size_t size;
} url_llist;

/*
 * Initialize a linked list.
 * Sets front and back pointers to null and size to 0.
 */
void url_llist_init(url_llist *list);

/*
 * Push a url to the front of the linked list
 */
void url_llist_push_front(url_llist *list, urlinfo *url);

/*
 * Push a url to the back of the linked list
 */
void url_llist_push_back(url_llist *list, urlinfo *url);

/*
 * Pop from the front of the linked list
 */
urlinfo *url_llist_pop_front(url_llist *list);

/*
 * Pop from the back of the linked list
 */
urlinfo *url_llist_pop_back(url_llist *list);

#endif
