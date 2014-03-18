/********************************
 * searchcache.c
 *******************************/

#include "searchcache.h"
#include "url_linked_list.h"
#include "llist.h"
#include "binarytree.h"
#include "general_utils.h"
#include "../urlinfo.h"
#include <stdio.h>
#include <string.h>

#define MAXLENGTH 2048

typedef struct indexed_url
{
	urlinfo *url;
	long index;
} indexed_url;

int indexed_urlcompare(indexed_url *a, indexed_url *b)
{
	int compare = strcmp(a->url->host, b->url->host);
	if (compare)
		return compare;
	compare = strcmp(a->url->path, b->url->path);
	if (compare)
		return compare;
	return strcmp(a->url->filename, b->url->filename);	
}

int comparelong(long *a, long *b)
{
	if (*a == *b)
		return 0;
	else if (*a < *b)
		return -1;
	else
		return 1;
}

int equalslong(long *a, long *b)
{
	if (comparelong(a, b))
		return 0;
	else
		return 1;
}

// helper function to concat path and filename
char *getpath(char *folder, char *filename)
{
	char *path = malloc(strlen(folder) + strlen(filename) + 2);
	strcpy(path, folder);
	strcat(path, "/");
	strcat(path, filename);
	return path;
}

char *tounderline(char *str)
{
	int len = strlen(str);
	char *output = malloc(len + 1);
	strcpy(output, str);

	// replace all spaces w/ underlines
	int i;
	for (i = 0; i < len; i++)
		if (output[i] == ' ')
			output[i] = '_';
	return output;
}

url_llist *getcache(char *folder, char *searchstring)
{
	char *modifiedstring = tounderline(searchstring);
	char *path = getpath(folder, modifiedstring);
	FILE *file = fopen(path, "r");
	
	// return null if unable to open for read (indicating file doesn't exist
	if (!file)
		return NULL;
	
	// get number of urls
	int numlinks;
	fscanf(file, "%d\n", &numlinks);
	
	// creat array of urls, and a corresponding array holding indexes of urls each url points to
	urlinfo *urls[numlinks];
	llist outlink_indexes[numlinks];
	
	// initialize output list
	url_llist *output = malloc(sizeof(url_llist));
	url_llist_init(output);
	
	// read each url (each is on a separate line)
	// push to linked list
	char urlstring[MAXLENGTH];
	int numoutlinks;
	urlinfo *url;
	
	// pass 1: index urls
	unsigned long *outlink_index;
	unsigned long i, j;
	
	while(fscanf(file, "%s %d\n", urlstring, &numoutlinks) != EOF)
	{
		// construct url (without outlinks)
		url = makeURLfromlink(urlstring, NULL);
		
		// push url and a linked list for its outlinks
		url_llist_push_back(output, url);
		urls[i] = url;
		llist_init(&(outlink_indexes[i]), (void *)equalslong);
		
		// read and push each outlink
		for (j = 0; j < numoutlinks; j++)
		{
			outlink_index = malloc(sizeof(unsigned long));
			fscanf(file, "%lu ", outlink_index);//outlink_index))
			llist_push_back(&(outlink_indexes[i]), outlink_index);
		}
		
		// read in '\n'
		fscanf(file, "\n");
		
		i++;
	}
	lnode *current_node, *prev_node;
	// pass 2: get outlinks by their indexes
	for (i = 0; i < numlinks; i++)
	{
		url = urls[i];
		current_node = outlink_indexes[i].front;
		while (current_node)
		{
			outlink_index = current_node->data;
			prev_node = current_node;
			llist_push_back(&urls[i]->outlinks, urls[(long)*outlink_index]);
			current_node = current_node->next;
			free(outlink_index);
			free(prev_node);
		}
	}
	free(path);
	free(modifiedstring);
	fclose(file);
	
	return output;	
}

void setcache(char *folder, char *searchstring, url_llist *urls)
{
	long i, j;
	urlinfo *url;
	url_node *node = urls->front;
	btree indexed_urls;
	btree_init(&indexed_urls, (void *)indexed_urlcompare);
	
	// loop 1: index urls
	for (i = 0; i < urls->size; i++)
	{
		indexed_url *iurl = malloc(sizeof(indexed_url));
		iurl->url = node->url;
		iurl->index = i;

		btree_insert(&indexed_urls, iurl);
				
		node = node->next;
	}

	// open file for write
	char *modifiedsearch = tounderline(searchstring);
	char *path = getpath(folder, modifiedsearch);
	
	FILE *file = fopen(path, "w");
	
	// write number of links
	fprintf(file, "%d\n", urls->size);

	// loop 2: write data
	node = urls->front;
	indexed_url urltofind;
	for (i = 0; i < urls->size; i++)
	{
		url = node->url;
		
		/*
		 * Write url data
		 * format:	index url numlinks
		 *		link1 link2 link3
		 */
		// write header
		char *urlstring = url_tostring(url);
		fprintf(file, "%s %d\n ", urlstring, url->outlinks.size);
		free(urlstring);
		// write links
		lnode *outlink_node = url->outlinks.front;
		for (j = 0; j < url->outlinks.size; j++)
		{	
			// find indexed url so its index can be recorded
			urltofind.url = outlink_node->data;
			indexed_url *iurl = btree_find(&indexed_urls, &urltofind);
			
			// write it, followed by a space
			fprintf(file, "%lu ", iurl->index);
			outlink_node = outlink_node->next;
		}
		//if (url->outlinks.size)
		fprintf(file, "\n");
		node = node->next;
	}

	btree_free(&indexed_urls, 1);
	free(modifiedsearch);
	free(path);
	fclose(file);
}

void setcache2(char *folder, char *searchstring, url_llist *urls)
{
	long i, j;
	urlinfo *url;
	url_node *node = urls->front;
	btree indexed_urls;
	btree_init(&indexed_urls, (void *)indexed_urlcompare);
	
	// loop 1: index urls
	for (i = 0; i < urls->size; i++)
	{
		indexed_url *iurl = malloc(sizeof(indexed_url));
		iurl->url = node->url;
		iurl->index = i;
		
		btree_insert(&indexed_urls, iurl);
		
		node = node->next;
	}
	
	// open file for write
	char *modifiedsearch = tounderline(searchstring);
	char *path = getpath(folder, modifiedsearch);
	path = realloc(path, strlen(path) + 2);
	strcat(path, "+");
	
	FILE *file = fopen(path, "w");
	
	// write number of links
	fprintf(file, "%d Total URLS\n", urls->size);
	
	// loop 2: write data
	node = urls->front;
	indexed_url urltofind;
	for (i = 0; i < urls->size; i++)
	{
		url = node->url;
		
		/*
		 * Write url data
		 * format:	index url numlinks
		 *		link1 link2 link3
		 */
		// write header
		char *urlstring = url_tostring(url);
		indexed_url *this_url = btree_find(&indexed_urls, &url);
		
		fprintf(file, "INDEX %lu - %s\n\tOUTLINKS = %d | ", this_url->index, urlstring, url->outlinks.size);
		free(urlstring);
		// write links
		lnode *outlink_node = url->outlinks.front;
		for (j = 0; j < url->outlinks.size; j++)
		{
			// find indexed url so its index can be recorded
			urltofind.url = outlink_node->data;
			indexed_url *iurl = btree_find(&indexed_urls, &urltofind);
			
			// write it, followed by a space
			fprintf(file, "%lu ", iurl->index);
			outlink_node = outlink_node->next;
		}
		//if (url->outlinks.size)
		fprintf(file, "\n\tINLINKS  = %d | ", url->inlinks.size);
		
		lnode *inlink_node = url->inlinks.front;
		for (j = 0; j < url->inlinks.size; j++)
		{
			// find indexed url so its index can be recorded
			urltofind.url = inlink_node->data;
			indexed_url *iurl = btree_find(&indexed_urls, &urltofind);
			
			// write it, followed by a space
			fprintf(file, "%lu ", iurl->index);
			inlink_node = inlink_node->next;
		}
		
		fprintf(file, "\n");
		node = node->next;
	}
	
	btree_free(&indexed_urls, 1);
	free(modifiedsearch);
	free(path);
	fclose(file);
}