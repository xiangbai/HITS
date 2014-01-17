/********************************
 * searchcache.c
 *******************************/

#include "searchcache.h"
#include "string_linked_list.h"
#include "general_utils.h"
#include <stdio.h>
#include <string.h>

#define MAXLENGTH 2048

// helper function to concat path and filename
char *getpath(char *folder, char *filename)
{
	char *path = malloc(strlen(folder) + strlen(filename) + 2);
	strcpy(path, folder);
	strcat(path, "/");
	strcat(path, filename);
	return path;
}

char *tounderline(char *str)
{
	int len = strlen(str);
	char *output = malloc(len + 1);
	strcpy(output, str);

	// replace all spaces w/ underlines
	int i;
	for (i = 0; i < len; i++)
		if (output[i] == ' ')
			output[i] = '_';
	return output;
}

string_llist *getcache(char *folder, char *mastername, char *searchstring)
{
	string_llist *output = NULL;
	char *modifiedstring = tounderline(searchstring);

	// generate masterfile w/ path
	char *path = getpath(folder, mastername);

	// attempt to open master file for read
	FILE *file = fopen(path, "r");
	// if it doesn't exist, create it while opening for write
	if (!file)
		file = fopen(path, "w+");
	
	// search for string
	char search[MAXLENGTH];
	int found = 0;
	while(fscanf(file, "%s\n", search) != EOF)
		if (!strcmp(modifiedstring, search))
		{
			// mark as found, and break
			found = 1;
			break;
		}
	// close file
	fclose(file);

	// create linked list if the search string is found	
	if (found)
	{
		output = malloc(sizeof(string_llist));
		string_llist_init(output);
		
		// filename w/ path
		char *urlpath = getpath(folder, search);
		
		// read each url (each is on a separate line)
		// push to linked list
		char url[MAXLENGTH];
		FILE *urlfile = fopen(urlpath, "r");
		while(fscanf(urlfile, "%s\n", url) != EOF)
			string_llist_push_back(output, url);
		
		// close file
		fclose(urlfile);
		
		free(urlpath);
	}
	free(path);
	free(modifiedstring);
	
	return output;	
}

void setcache(char *folder, char *mastername, char *searchstring, string_llist *urls)
{
	char *path = getpath(folder, mastername);
	char *modifiedstring = tounderline(searchstring);
	// attempt to open master file for read
	FILE *file = fopen(path, "r");
	
	// search for string in master file
	char search[MAXLENGTH];
	int found = 0;
	while(fscanf(file, "%s\n", search) != EOF)
		if (!strcmp(search, modifiedstring))
		{
			// mark as found, and break
			found = 1;
			break;
		}
	
	// close file
	fclose(file);
	
	// add the search string if it's not in the master file
	if (!found)
	{
		file = fopen(path, "a");
		if (!file)
		{
			report_error("Error opening file for append");
			return;
		}
		fwrite(modifiedstring, sizeof(char), strlen(modifiedstring), file);
		fwrite("\n", sizeof(char), 1, file);
		fclose(file);
	}
	// free path to master file
	free(path);
	
	// write all elements in list to file name=search
	path = getpath(folder, modifiedstring);
	file = fopen(path, "w");

	// iterate through linked list
	string_node *node = urls->front;
	while (node)
	{
		fwrite(node->string, sizeof(char), strlen(node->string), file);
		fwrite("\n", sizeof(char), 1, file);
		node = node->next;
	}

	// close file and free file path
	close(file);
	free(path);
	free(modifiedstring);
}

void removecache(char *folder, char *mastername, char *searchstring)
{
	/*
	 * Writes write all strings not being deleted to a temp file.
	 * Then renames the temp file as the main file.
	 * If this works, the original file should be replaced.
	 */
	
	// generate filename w/ path
	char *inpath = getpath(folder, mastername);
	char *modifiedstring = tounderline(searchstring);
	
	// generate filename for temporary file
	char *outpath = malloc(strlen(inpath) + 5);
	strcpy(outpath, inpath);
	strcat(outpath, ".swp");
	
	// open a file for input and output
	FILE *infile = fopen(inpath, "r");
	FILE *outfile = fopen(outpath, "w");

	// search through file, until pos = position of searchstring
	//	and nextpos = the position one after the end of searchstring
	char search[MAXLENGTH]; // +1 for null

	while(fscanf(infile, "%s\n", search) != EOF)
	{
		// remove '\n' for string compare
		search[strlen(search)] = '\0';
		
		if (strcmp(modifiedstring, search))
		{
			// add the '\n' back for writing
			search[strlen(search)] = '\n';
			fwrite(search, sizeof(char), strlen(search), outfile);
		}
		else
		{
			// generate path for file to delete
			char *searchfile = getpath(folder, search);
			
			// remove the file for the search
			if (remove(searchfile))
				report_error("Unable to remove file");
			free(searchfile);
		}
	}
	
	fclose(infile);
	fclose(outfile);
	
	// replace infile with outfile
	// this may fail, depending on the OS
	if(rename(outpath, inpath))
		report_error("Unable to overwrite file");
	free(modifiedstring);
}
