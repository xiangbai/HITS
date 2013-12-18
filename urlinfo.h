#ifndef URLINFO_H
#define URLINFO_H

/****************************
 * urlinfo.h
 * Holds struct urlinfo
 *  and methods for using it
 ****************************/

#define HOST_LENGTH 128
#define PATH_LENGTH 2048

/*
 * Regex pattern used to parse urls
 * 
 * pcre will match the entire pattern, and save substrings enclosed in parentheses
 * more substrings are returned than desired, because parentheses are also part of regex syntax
 * assume output[] is returned by parser.substrings_to_array(), which uses pcre
 * output[0]: string matching the entire string
 * output[1]: domain
 * output[2]: domain extension
 * output[3]: a sequence of 0 or more "../", indicating how many folders to go up
 * output[4]: the last instance of "../" if it exists
 * output[5]: the path and filename following the domain and any instances of "../"
		functions as the path for relative paths with no filename and not ending in '/'
 * output[6]: path, assuming there is a filename, or the path ends in '/'
 * output[7]: filename
 * 
 * Macros should always be used to extract info, because this pattern is likely to change
 */
#define URL_REGEX "(.+?\\.(com|net|org))?/?((\\.\\./)*)(([^\\.]+/)?([^/]+\\..+)?.*)"

#define URL_REGEX_NUM_SUBSTRINGS 8
#define URL_REGEX_DOMAIN 1
#define URL_REGEX_UP_FOLDER 3
#define URL_REGEX_PATH 6
#define URL_REGEX_PATH2 5
#define URL_REGEX_FILE 7

typedef struct urlinfo
{
	char *host;
	char *path;
	char *filename;
	struct urlinfo *next;
} urlinfo;

/*
 * construct a url
 * if givenAddress is a relative address, currentURL be non-null and contain a domain in host
 */
urlinfo *makeURL(char *givenAddress, urlinfo *currentURL);

/*
 * free memory allocated to a url and its data members
 * returns a pointer to url->next
 */
urlinfo *freeURL(urlinfo *url);


#endif
