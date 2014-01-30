#ifndef URLINFO_H
#define URLINFO_H

/****************************
 * urlinfo.h
 * Holds struct urlinfo
 *  and methods for using it
 ****************************/

#include "utils/llist.h"
#include "utils/string_linked_list.h"

#define HOST_LENGTH 128
#define PATH_LENGTH 2048

typedef struct urlinfo
{
	char *host;
	char *path;
	char *filename;
	int searchdepth;
	int redirectdepth;
	struct urlinfo *next;
	llist outlinks;
	double hubScore;
	double authScore;
} urlinfo;

typedef struct string_redirect
{
	char *bad_link;
	urlinfo *valid_url;
} string_redirect;

typedef struct url_w_string_links
{
	urlinfo *url;
	string_llist outlinks;
} url_w_string_links;

/*
 * Initialise a url_w_string_links
 */
url_w_string_links *url_w_links_init(urlinfo *url);

/*
 * Free the data held by url_w_string_links
 */
void url_w_links_free(url_w_string_links *url, int free_pointers);

/*
 * Compare two string_redirects based on their bad_link string only
 */
int compare_redirects(string_redirect *a, string_redirect *b);

/*
 * Initializes a string_redirect
 * The string bad_link is copied, but valid_url is pointed to
 */
string_redirect *redirect_init(char *bad_link, urlinfo *valid_url);

/*
 * Free the data pointed to by a string_redirect
 */
void redirect_free(string_redirect *redirect);

/*
 * construct a url from link
 * if givenAddress is a relative address, currentURL be non-null and contain a domain in host
 */
urlinfo *makeURLfromlink(char *givenAddress, urlinfo *currentURL);

/*
 * construct a url using redirect location from 3xx error
 */
urlinfo *makeURLfromredirect(char *givenAddress, urlinfo *currentURL);

/*
 * returns url as a c-style string
 */
char *url_tostring(urlinfo *url);

/*
 * free memory allocated to a url and its data members
 * returns a pointer to url->next
 */
urlinfo *freeURL(urlinfo *url);

int urlcompare(urlinfo *a, urlinfo *b);

#endif
