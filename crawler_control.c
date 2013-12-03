#include <stdlib.h>
#include "string_linked_list.h"

#define BUFFER_SIZE 1024

/* main routine for testing our crawler's funcitonality */
int main(void)
{
	int socket;
	int port = 80;
	url_queue linkstocheck;
	url_queue allURLs;
	url_info newURL;
	
	//seed list
	newURL.host = "www.google.com";
	newURL.path = "/";
	push_url(newURL);

	int i;
	for(i = 0; i < 20; ++)
	{
		socket = socket(AF_INET, SOCK_STREAM, 0);
		
		newURL = pop_url(&linkstocheck);

		// returns non 0 upon failure
		if (connecttohost(socket, newURL.host, port) == 0)
		{	//send request, parse links, push to both queues, -1 for failure
			if (readHTML(socket, host, path, &linkstocheck) == -1)
				report_error("readHTML error");
		}
		else
			report_error("connecttohost() failed");
	}

	display_queue(*allURLs);
	return 0;
}

/*
 * Load the code from a webpage.
 * Assumes http request has already been sent
 * Allocates memory; free it when you are done
 */
char *loadPage(int socket)
{
	// declare variables
	char buffer[] = new char[BUFFER_SIZE];
	int bytes_received = 0;
	string_ll list;

	// clear list
	string_llist_init(&list);

	// read data and push to linked list, avoiding the need to know code length beforehand
	do
	{
		memset(buffer, 0, sizeof(buffer));			
		bytes_received = read(socket, buffer, BUFFER_SIZE - 1);
		buffer[BUFFER_SIZE - 1] = '\0';				
		string_llist_push_back(&list, buffer);
	} while (bytes_received);

	// allocate enough space to store code in linked list (and null char)
	char code[] = (char *) malloc(list.num_chars + 1) * sizeof(char));

	// load code from linked list to string
	size_t current_index = 0;
	while (list.size)
	{
		string_llist_pop_front(&list, buffer);
		strcpy(code + current_index, buffer);
		current_index += strlen(buffer);
	}

	return code;
}
