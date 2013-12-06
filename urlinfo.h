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
	struct urlinfo *next;
} urlinfo;

/*
 * construct a url from a (possibly) relative address
 */
struct urlinfo makeURL(char *givenAddress, char *currentURL);

#endif
