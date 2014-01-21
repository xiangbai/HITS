#ifndef URLINFO_H
#define URLINFO_H

/****************************
 * urlinfo.h
 * Holds struct urlinfo
 *  and methods for using it
 ****************************/

#include "utils/llist.h"

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
