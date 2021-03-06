#include "urlinfo.h"
#include "utils/string_linked_list.h"
#include "utils/general_utils.h"
#include "utils/parser.h"
#include "utils/llist.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Regex pattern used to parse urls
 * 
 * pcre will match the entire pattern, and save substrings enclosed in parentheses
 * more substrings are returned than desired, because parentheses are also part of regex syntax
 * assume output[] is returned by parser.substrings_to_array(), which uses pcre
 * output[0]: string matching the entire string
 * output[1]: "//" - indicates to use the same scheme as used for the current site
 * output[2]: scheme: https, https
 * output[3]: 
 * output[4]: domain
 * output[5]: domain extension
 * output[6]: a sequence of 0 or more "../", indicating how many folders to go up
 * output[7]: the last instance of "../" if it exists
 * output[8]: the path and filename following the domain and any instances of "../"
		functions as the path for relative paths with no filename and not ending in '/'
 * output[9]: path, assuming there is a filename, or the path ends in '/'
 * output[10]: last folder in path
 * output[11]: filename (excludes % and anything after - google uses this)
 * 
 * Macros should always be used to extract info, because this pattern is likely to change
 */
#define URL_REGEX "(//)?(.+://)?([^/\\.]\\.)?(.+?\\.(com|coop|edu|net|org|uk))?/?((\\.\\./)*)((([^/]+/)*)?([^/\\.]+\\.[^%&]+)?[^%&]+)?"
#define URL_REGEX_NUM_SUBSTRINGS 12
#define URL_REGEX_DOMAIN 4
#define URL_REGEX_UP_FOLDER 6
#define URL_REGEX_PATH 9
#define URL_REGEX_PATH2 8
#define URL_REGEX_FILE 11

urlinfo *makeURL(char *givenAddress, urlinfo *currentURL);

// Regex pattern to be used. Initialize once
parser *regex = NULL;

int compare_redirects(string_redirect *a, string_redirect *b)
{
	return strcmp(a->bad_link, b->bad_link);
}

url_w_string_links *url_w_links_init(urlinfo *url)
{
	url_w_string_links *output = malloc(sizeof(url_w_string_links));
	output->url = url;
	string_llist_init(&output->outlinks);
	return output;
}

void url_w_links_free(url_w_string_links *url, int free_pointers)
{
	string_llist_free(&url->outlinks);
}

string_redirect *redirect_init(char *bad_link, urlinfo *valid_url)
{
	string_redirect *output = malloc(sizeof(string_redirect));
	output->bad_link = malloc((strlen(bad_link)+1) * sizeof(char));
	strcpy(output->bad_link, bad_link);
	output->valid_url = valid_url;
	return output;
}

void redirect_free(string_redirect *redirect)
{
	free(redirect->bad_link);
}

urlinfo *makeURLfromlink(char *givenAddress, urlinfo *currentURL)
{
        urlinfo *newurl = makeURL(givenAddress, currentURL);
        
        newurl->searchdepth = (currentURL)? currentURL->searchdepth + 1: 0;
        newurl->redirectdepth = currentURL->redirectdepth;
        
        return newurl;
}

urlinfo *makeURLfromredirect(char *givenAddress, urlinfo *currentURL)
{
	urlinfo *newurl = makeURL(givenAddress, currentURL);

	newurl->searchdepth = (currentURL)? currentURL->searchdepth: 0;
	newurl->redirectdepth = currentURL->redirectdepth + 1;
	
	return newurl;
}

// function used by makeURLfromlink and makeURLfromredirect
int urlcompare(urlinfo *a, urlinfo *b)
{
	int compare = strcmp(a->host, b->host);
	if (compare)
		return compare;
	compare = strcmp(a->path, b->path);
	if (compare)
		return compare;
	return strcmp(a->filename, b->filename);	
}

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
	int found = substrings_to_array(regex, givenAddress, strlen(givenAddress), 0, array);
	
	if (found < 0)
		return NULL;
	
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
		int filelen = strlen(array[URL_REGEX_FILE]);
		if (pathlen)
			array[URL_REGEX_PATH][pathlen - 1] = '\0';

		// get the path from the given address;
		char *newpath;
		if (!filelen)//!pathlen && strcmp(array[URL_REGEX_PATH2], array[URL_REGEX_FILE]))
		{
			//if (array[URL_REGEX_PATH2][filelen - 1] == '/')
			//	array[URL_REGEX_PATH2][filelen - 1] = '\0';
			newpath = array[URL_REGEX_PATH2];
		}
		else
		{
			newpath = array[URL_REGEX_PATH];
		}

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
	
	// make linked list of outgoing links
	llist_init(&newurl->outlinks, (void *)urlcompare);
	
	return newurl;	
}

char *url_tostring(urlinfo *url)
{
	size_t hostlen = strlen(url->host);
	size_t pathlen = strlen(url->path);
	size_t filelen = strlen(url->filename);
	int haspath = (pathlen)? 1: 0;
	int hasfile = (filelen)? 1: 0;
	char *string = (char *)malloc(hostlen + pathlen + filelen + 1 + haspath + hasfile);
	
	// domain
	strcpy(string, url->host);
	
	// path and file
	if (haspath)
	{
		string[hostlen] = '/';
		strcpy(string + hostlen + 1, url->path);
	}
	if (hasfile)
	{
		strcpy(string + hostlen + pathlen + haspath, "/");
		strcpy(string + hostlen + pathlen + haspath + 1, url->filename);
	}

	return string;
     /*
    size_t hostlen = strlen(url->host);
	size_t pathlen = strlen(url->path);
	size_t filelen = strlen(url->filename);
	int haspath = (pathlen)? 1: 7;
	int hasfile = (filelen)? 1: 7;
	char *string = (char *)malloc(hostlen + pathlen + filelen + 1 + haspath + hasfile + 20);
	
	// domain
    strcpy(string, "HOST: ");
	strcat(string, url->host);
	
	// path and file
	if (haspath)
	{
		strcat(string, " PATH: ");
		strcat(string, url->path);
	}
	if (hasfile)
	{
		strcat(string, " FILE: ");
		strcat(string, url->filename);
	}
    
	return string;
     */
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
