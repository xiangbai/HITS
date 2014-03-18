#ifndef SEARCHCACHE_H
#define SEARCHCACHE_H

/*************************************************************
 *
 * searchcache.h
 *
 * Contains functions for caching search results
 * A separate file is created for each search, consisting of 
 *	the urls the search finds, and what urls those link to
 * The first line of the file must contain the number of urls
 * Every url is listed on a line, followed by how many urls
 * 	it links to.
 * If the url had outgoing links, it is followed by a line
 * 	containing the indexes of the urls it links to
 * Reading and writing must therefore be done using 2 passes
 * The first pass indexes the urls
 * The second pass converts indexes to urls or vise versa
 * Each file is named by its search string, 
 *	with '_' replacing  ' '
 *
 *************************************************************/

#include "url_linked_list.h"
/*
 * Gets the urls found from a given search string
 * Returns NULL if the search isn't cached
 */
url_llist *getcache(char *folder, char *searchstring);

/*
 * Creates file to hold search results
 * Updates master file to reference the search results file
 * If the masterfile doesn't exist, it is created
 * If the url file exists, it is overwritten
 */
void setcache(char *folder, char *searchstring, struct url_llist *urls);

void setcache2(char *folder, char *searchstring, struct url_llist *urls);

#endif
