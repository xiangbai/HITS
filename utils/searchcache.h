/*************************************************************
 *
 * sitecache.h
 *
 * Contains functions for caching search results
 * A master file holds the names of all cached pages
 * The master file and all urls are held in a separate folder
 * A separate file is created for each search, consisting
 *	of only the urls the search finds
 * Each file is named by its search string, 
 *	with '_' replacing  ' '
 *
 *************************************************************/

#include "string_linked_list.h"

/*
 * Gets the urls found from a given search string
 * Returns NULL if the search isn't cached
 */
string_llist *getcache(char *folder, char *mastername, char *searchstring);

/*
 * Creates file to hold search results
 * Updates master file to reference the search results file
 * If the masterfile doesn't exist, it is created
 * If the url file exists, it is overwritten
 */
void setcache(char *folder, char *mastername, char *searchstring, string_llist *urls);

/*
 * Remove a file holding search results
 * That search string will be removed from the master file as well
 */
void removecache(char *folder, char *mastername, char *searchstring);


