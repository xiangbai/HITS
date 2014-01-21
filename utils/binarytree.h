#ifndef BINARYTREE_H
#define BINARYTREE_H

#include <stdlib.h>

/********************************
 * binarytree.h
 * binary tree implementation
 *	using void pointers
 *******************************/

typedef struct treenode
{
	void *data;
	struct treenode *left;
	struct treenode *right;
	int leftthread;
	int rightthread;
} treenode;

typedef struct btree
{
	treenode *root;
	size_t numElems;
	int (*comparefunction)(void *a, void *b);
} btree;

/*
 * Initialize a binary tree
 */
void btree_init(btree *tree, int (*comparefunction)(void *a, void *b));

/*
 * Free the lnodes of a btree
 * if free_pointers != 0 the data pointed to by the btree will be freed as well
 * btree itself must be freed externally if it is in the heap
 */
void btree_free(btree *tree, int free_pointers);

/*
 * Insert an object into the tree
 */
void btree_insert(btree *tree, void *data);//, int (*comparefunction)(void *a, void *b));

/*
 * Remove an element from the tree
 * Returns the element removed
 */
void *btree_remove(btree *tree, void *data);//, int (*comparefunction)(void *a, void *b));

/*
 * Find an element if it exists in the tree
 * Returns that element
*/ 
void *btree_find(btree *tree, void *data);//, int(*comparefunction)(void *a, void *b));

/*
 * Create an array holding the data in order
 * Assumes the tree has at least one element
 * Array will be of size tree->numElems
 */
void **btree_toarray(btree *tree);

#endif
