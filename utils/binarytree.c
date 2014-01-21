#include "binarytree.h"
#include <stdlib.h>

// construct a node with threads to null
treenode *maketreenode(void *data)
{
	treenode *output = (treenode *)malloc(sizeof(treenode));
	output->left = NULL;
	output->right = NULL;
	output->leftthread = 1;
	output->rightthread = 1;
	output->data = data;
	return output;
}

// extract data from a node, and free the node
void *killtreenode(treenode *node)
{
	void *output = node->data;
	free(node);
	return output;
}

void btree_init(btree *tree, int (*comparefunction)(void *a, void *b))
{
	tree->root = NULL;
	tree->numElems = 0;
	tree->comparefunction = comparefunction;
}

void btree_free(btree *tree, int free_pointers)
{
	treenode *node = tree->root;
	treenode *prevnode;
	void *data;
	
	// get the left-most node
	while (node->left)
		node = node->left;
	
	// traverse linked list in order
	while (node)
	{
		prevnode = node;
		
        	if (node->rightthread)
        	    node = node->right;
        	else
        	{
        	    node = node->right;
        	    while(!node->leftthread)
        	        node = node->left;
        	}
		// free node
		data = killtreenode(prevnode);
		
		// free data if desired
		if (free_pointers)
			free(data);
	}
}

void btree_insert(btree *tree, void *data)//, int (*comparefunction)(void *a, void *b))
{
	treenode *node = maketreenode(data);
	treenode *current = tree->root;
	treenode *prev;
	int wentleft = 0;
	
	// traverse down the list until prev is a leaf, and current is one past the leaf
	if (tree->root)
	{
		while (1)
		{
			prev = current;
			int compare = tree->comparefunction(data, current->data);
			if (compare < 0)
			{	// go left if data < current->data
				wentleft = 1;
				current = current->left;
				if (prev->leftthread)
					break;
			}
			else
			{	// go right if data > current->data
				wentleft = 0;
				current = current->right;
				if (prev->rightthread)
					break;
			}
		}
		
		// insert the node
		if (wentleft)
		{
			node->left = current;
			node->right = prev;
			prev->left = node;
			prev->leftthread = 0;
			if (current && current->rightthread)
				current->right = node;
		}
		else
		{
			node->right = current;
			node->left = prev;
			prev->right = node;
			prev->rightthread = 0;
			if (current && current->leftthread)
				current->left = node;
		}
	}
	else	// node is first element; set as root
		tree->root = node;
	
	// increment numElems
	tree->numElems++;
}

void *btree_remove(btree *tree, void *data)//, int (*comparefunction)(void *a, void *b))
{
	void *output = NULL;
	
	if (tree->root)
	{
		treenode *current = tree->root;
		treenode *parent;
		int wentleft = 0;
		
		// find node to remove and its parent
		while (1)
		{
			int compare = tree->comparefunction(data, current->data);
			if (!compare)
				break;	// found it
			else 
			{
				parent = current;
				if (compare < 0)
				{	// go left
					if (current->leftthread)
					{	// break if following a thread
						current = NULL;
						break;
					}
					current = current->left;
				}
				else
				{	// go right
					if (current->rightthread)
					{	// break if following a thread
						current = NULL;
						break;
					}
					current = current->right;
				}
			}
		}
		// if node was found, current points to it
		if (current)
		{
			treenode *next;
			if (current->leftthread && current->rightthread)
			{	// node is a leaf
				next = (wentleft)? current->left: current->right;
				
				if (!parent)
					tree->root = next;
				else if (wentleft)
				{
					parent->left = next;
					parent->leftthread = 1;
					if (next && next->rightthread)
						next->right = parent;
				}
				else
				{
					parent->right = next;
					parent->rightthread = 1;
					if (next && next->leftthread)
						next->left = parent;
				}
			}
			else if (current->leftthread)
			{	// current only has right child
				next = current->right;
				if (!parent)
					tree->root = next;
				else if (wentleft)
					parent->left = next;
				else
					parent->right = next;
				while (!next->leftthread)
					next = next->left;
				if (next->left == current)
					next->left = current->left;
			}
			else if (current->rightthread)
			{	// current only has left child
				next = current->left;
				if (!parent)
					tree->root = next;
				else if (wentleft)
					parent->left = next;
				else
					parent->right = next;	
				while (!next->rightthread)
					next = next->right;
				if (next->right == current)
					next->right = current->right;
			}
			else
			{	// current has 2 children
				treenode *parentofnext = current;
				next = current->right;
				
				// find successor and its parent
				while (!next->leftthread)
				{
					parentofnext = next;
					next = next->left;
				}
				
				// set next's left pointer
				next->left = current->left;
				next->leftthread = 0;
				
				// set parent's pointer
				if (!parent)
					tree->root = next;
				else if (wentleft)
					parent->left = next;
				else
					parent->right = next;
				
				// set next's right pointer
				if (next->rightthread && current != parentofnext)
				{
					next->right = current->right;
					next->rightthread = 0;
				}

				// if there was a thread to current from the left, update it
				treenode *prev = current->left;
				while (!prev->rightthread)
					prev = prev->right;
				if (prev->right == current)
					prev->right = next;
			}
			// extract data from node and kill node
			output = killtreenode(current);
			tree->numElems--;
		}
	}
	
	return output;
}

void *btree_find(btree *tree, void *data)
{
	treenode *current = tree->root;
	
	// traverse tree until data is found
	if (tree->root)	
		while (1)
		{
			int compare = tree->comparefunction(data, current->data);
            
			if (!compare)
			{	// found
				break;
			}
			else if (compare < 0)
			{	// go left
				if (current->leftthread)
				{
					current = NULL;
					break;
				}
				current = current->left;
			}
			else
			{	// go right
				if (current->rightthread)
				{
					current = NULL;
					break;
				}
				current = current->right;
			}
		}
	if (current)
		return current->data;
	
	return NULL;
}

void **btree_toarray(btree *tree)
{
	size_t size = tree->numElems;
	void **array = calloc(size, sizeof(void *));
	treenode *node = tree->root;

	// get the left-most node
	while (node->left)
		node = node->left;
	
	// add all elements to the array
	size_t i;
	for (i = 0; i < size; i++)
	{
		array[i] = node->data;
		
        	if (node->rightthread)
        	    node = node->right;
        	else
        	{
        	    node = node->right;
        	    while(!node->leftthread)
        	        node = node->left;
        	}
	}
	
	// return the array
	return array;
}
