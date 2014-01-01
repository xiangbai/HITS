#ifndef LLIST_H
#define LLIST_H

/***************************************************************
 * llist.h
 * 
 * Implementation of linked lists using null pointers.
 * It is recommended to call llist_init() to clear data
 * 	before use.
 ***************************************************************/

#include "../urlinfo.h"
#include <stdlib.h>

// struct lnode for internal use
typedef struct lnode {
	void *data;
	struct lnode *next;
	struct lnode *prev;
} lnode;

// struct for the linked list
typedef struct llist {
	struct lnode *front;
	struct lnode *back;
	size_t size;
	int (*equalsfunction)(void *a, void *b);
} llist;

/*
 * Initialize a linked list.
 * Sets front and back pointers to null and size to 0.
 */
void llist_init(llist *list, int (*equalsfunction)(void *a, void *b));

/*
 * Push data to the front of the linked list
 */
void llist_push_front(llist *list, void *data);

/*
 * Push data to the back of the linked list
 */
void llist_push_back(llist *list, void *data);

/*
 * Pop from the front of the linked list
 */
void *llist_pop_front(llist *list);

/*
 * Pop from the back of the linked list
 */
void *llist_pop_back(llist *list);

void *llist_find(llist *list, void *obj);

/*
 * Find an element in the list.
 * Returns 1 if element is found; else 0.
 * A equals function must be provided to identify when a match is found.
 * The equals function must return 1 if two objects are equal; else 0.
 */
//int llist_find(llist *list, void *obj, int (*equalsfunction)(void *a, void *b));

#endif
