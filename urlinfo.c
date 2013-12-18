#include "urlinfo.h"
#include "utils/general_utils.h"
#include "utils/parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Regex pattern to be used. Initialize once
parser *regex = NULL;

urlinfo *makeURL(char *givenAddress, urlinfo *currentURL)
{
	urlinfo *newurl = NULL;
	int i;
	
	// make regex if it doesn't exist yet
	if (regex == NULL)
		regex = init_parser(URL_REGEX);
	
	// create array and clear it
	char *array[URL_REGEX_NUM_SUBSTRINGS];
	for (i = 0; i < URL_REGEX_NUM_SUBSTRINGS; i++)
		array[i] = NULL;	
	
	// put substrings into array
	substrings_to_array(regex, givenAddress, strlen(givenAddress), 0, array, URL_REGEX_NUM_SUBSTRINGS);
		
	if (!strlen(array[URL_REGEX_DOMAIN]) && !currentURL)
	{
		// relative path, and no current url
		report_error("No current url provided for relative path");
	}
	else
	{
		newurl = (urlinfo *)malloc(sizeof(urlinfo));
		/*
		 * NOTE:
		 * The regex pattern in urlinfo.h searches for
		 *	paths with '/' at the end.
		 * If this isn't found, array[URL_REGEX_PATH2] will hold the path
		 * If stren(path) > 0, a path with '/' was found
		 * 	remove '/' if this pathtype was found
		 */
		int pathlen = strlen(array[URL_REGEX_PATH]);
		if (pathlen)
			array[URL_REGEX_PATH][pathlen - 1] = '\0';

		// get the path from the given address;
		char *newpath;
		if (!pathlen && strcmp(array[URL_REGEX_PATH2], array[URL_REGEX_FILE]))
			newpath = array[URL_REGEX_PATH2];
		else
			newpath = array[URL_REGEX_PATH];

		// set the filename
		newurl->filename = array[URL_REGEX_FILE];

		if (strlen(array[URL_REGEX_DOMAIN]))
		{
		// domain was provided; construct url using just given address
			newurl->host = array[URL_REGEX_DOMAIN];
			newurl->path = newpath;
		}
		else
		{
		// relative path; construct url using the current url and the given address
			
			// get number of folders to go up
			int folders_up = strlen(array[URL_REGEX_UP_FOLDER]) / 3;

			// get length of path
			int newpathlen = strlen(newpath);
			
			// copy the domain
			char *domain = (char *)malloc(strlen(currentURL->host) + 1);
			strcpy(domain, currentURL->host);
			newurl->host = domain;
			
			// copy the old path
			char *oldpath = (char *)malloc(strlen(currentURL->path) + 2); // leave room for an extra '/'
			strcpy(oldpath, currentURL->path);

			/*
			 * combine paths:
			 * begin assuming new path will be added at the end of current path
			 * as long as the first three characters in new path are "../":
			 *	go up a directory in old url and move past the "../" in new path
			 */
			size_t index_to_concat = strlen(oldpath) - 1;
			//size_t new_path_index = folders_up * 3;
			while(folders_up)
			{
				while (oldpath[index_to_concat] != '/')
					index_to_concat--;
				index_to_concat--;
				folders_up--;
			}
			
			index_to_concat++;
			
			// if 2 paths will be concated, add a '/'
			if (strlen(oldpath) && newpathlen)
				oldpath[index_to_concat++] = '/';
			
			// add terminator so strlen works correctly
			oldpath[index_to_concat] = '\0';
			
			// combine old and new paths
			char *path = (char *)malloc(strlen(oldpath) + newpathlen + 1);
			strcpy(path, oldpath);
			strcpy(path + index_to_concat, newpath);	
			free(oldpath);
			
			// set path
			newurl->path = path;
		}
	}

	return newurl;	
}

urlinfo *freeURL(urlinfo *url)
{
	urlinfo *next = url->next;

	free(url->host);
	free(url->path);
	free(url->filename);
	free(url);

	return next;
}
