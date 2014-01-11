#ifndef URLINFO_H
#define URLINFO_H

/****************************
 * urlinfo.h
 * Holds struct urlinfo
 *  and methods for using it
 ****************************/

#define HOST_LENGTH 128
#define PATH_LENGTH 2048

typedef struct urlinfo
{
	char *host;
	char *path;
	char *filename;
	int searchdepth;
	struct urlinfo *next;
    double hubScore;
    double authScore;
} urlinfo;

/*
 * construct a url
 * if givenAddress is a relative address, currentURL be non-null and contain a domain in host
 */
urlinfo *makeURL(char *givenAddress, urlinfo *currentURL);

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
