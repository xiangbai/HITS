/****************************
 * urlinfo.h
 * Holds struct urlinfo
 *  and methods for using it
 ****************************/

#define HOST_LENGTH 128
#define PATH_LENGTH 2048

struct urlinfo
{
	char *host;
	char *path;
	struct urlinfo *next;
};

/*
 * construct a url from a (possibly) relative address
 */
struct urlinfo makeURL(char *givenAddress, char *currentURL);
