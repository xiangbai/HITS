#include "urlinfo.h"
#include <stdlib.h>
#include <stdio.h>

/*
 * Program to test makeURL() 
 * 
 * The previous output of makeURL() is used as the current address
 * The first call to makeURL must include a domain; 
 *	if there is no current address, relative urls will throw errors
 * Any subsequent calls may use relative urls
 */ 

urlinfo *getparts(char *string)
urlinfo *current;
urlinfo *prev;

{
	printf("Analyzing %s\n----------------------------\n", string);
	
	// free memory from previous urls
	if (prev)
		freeURL(prev);
	
	// set prev to current, making room for the new url to be stored in current
	prev = current;
	
	// make the url (if prev != NULL, relative addresses may be passed in string
	current = makeURL(string, prev);
	
	// print selected substrings
	printf("Domain: %s\n", current->host);
	printf("Path: %s\n", current->path);
	printf("File: %s\ni\n", current->filename);

	return current;
}

int main(void)
{
	getparts("yep.net/path/path2/test.html");
	getparts("google.com/index.html");
	getparts("test.php");
	getparts("folder/folded.html");
	getparts("test.com/folder");
	getparts("folder2/newfile.txt");
	getparts("/folder3/newerfile.txt");
	getparts("../newfolder/horiz.php");
	getparts("/../../upone.php");

}
