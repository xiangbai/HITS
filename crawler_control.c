#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "utils/string_linked_list.h"
#include "utils/parser.h"
#include "utils/general_utils.h"
#include "utils/socket_utils.h"
#include "utils/status_code_util.h"
#include "utils/url_linked_list.h"
#include "urlinfo.h"

#define BUFFER_SIZE 1024

char *loadPage(int socket);
void getRequest(urlinfo *url, char *request);
int get_links(char *code, parser *p, string_llist *list, int *substrings, int num_substrings);
void getRequest(urlinfo *url, char *request);

/* main routine for testing our crawler's funcitonality */
int main(int argc, char **argv)
{
	char pattern[] = "<a [^>]*?href *?= *?[\'\"]([^\">]+)[\'\"].*?>";
	parser *regexparser;
	int socket;
	int port = 80;
	char request[BUFFER_SIZE + 100];
	struct url_llist linkstocheck;
	struct url_llist allURLs;
	string_llist hostsfound;
	urlinfo *newURL = NULL;
	urlinfo seedURL;
	
	char port_string[3];
	sprintf(port_string, "%d", port);

	// initialize parser
	regexparser = init_parser(pattern);

	// initialize queues
	url_llist_init(&linkstocheck);
	url_llist_init(&allURLs);
	string_llist_init(&hostsfound);

	//seed list
	if (argc > 1)
		seedURL.host = argv[1];
	else
		seedURL.host = "reddit.com";
	seedURL.path = "";
	seedURL.filename = "";

	int sitestocheck = (argc > 2)? atoi(argv[2]): 3;

	url_llist_push_back(&linkstocheck, &seedURL);
	string_llist_push_back(&hostsfound, seedURL.host);
	//url_llist_push_back(&allURLs, &seedURL);
	
	int i;
	for(i = 0; i < sitestocheck; i++)
	{
		if (!linkstocheck.size)
			break;
			
		
		// convert port# to string
		sprintf(port_string, "%d", port);
		
		newURL = url_llist_pop_front(&linkstocheck);
		
		getRequest(newURL, request);
		//printf("[request to %s] \n%s", newURL->host, request);
		
		socket = connect_socket(newURL->host, port_string, stdout);
		if (socket >= 0)
		{
			// send http requrest
			send(socket, request, strlen(request), 0);
			
			// get code
			char *code = loadPage(socket);
			
			// test if the page loaded properly
			int statuscode = get_status_code(code);
			
			if (statuscode >= 200 && statuscode < 300)
			{
				// create linked list to hold hyperlinks from code
				string_llist *links_in_code = malloc(sizeof(string_llist));
				string_llist_init(links_in_code);
				int substrings[] = {1};
				get_links(code, regexparser, links_in_code, substrings, 1);
				
				// load urls
				printf("links: ");
				char urlstring[BUFFER_SIZE];
				while (links_in_code->size)
				{
					string_llist_pop_front(links_in_code, urlstring);
					
					urlinfo *urlfromstring = makeURL(urlstring, newURL);
					
					int stringindex = string_llist_find(&hostsfound, urlfromstring->host);
					if (stringindex < 0)
					{
						url_llist_push_back(&linkstocheck, urlfromstring);
						url_llist_push_back(&allURLs, urlfromstring);
						string_llist_push_back(&hostsfound, urlfromstring->host);
						printf("x");
					}
				}
				printf("\n");
			}
			else if (statuscode >= 300 && statuscode < 400)
				printf("%s\n", code);
		}
		else
			report_error("socket_connect() failed");	

		//close the socket
		int closed = close(socket);
		if (closed)
			puts("error closing socket..");
		//else
		//	puts("closed socket");
				
		// pause for user input
		fputs("hit enter to continue..", stdout);
		char input[] = "";
		fgets(input, 2, stdin);
		printf("-----------------------------\n");
	}

	printf("\n%d URLs\n---------\n", allURLs.size);
	char *urlstring;
	while (allURLs.size)
	{
		newURL = url_llist_pop_front(&allURLs);
		urlstring = url_tostring(newURL);
	
		printf("%s\n", urlstring);
			
		free(urlstring);
		freeURL(newURL);
	}	

	return 0;
}

void getRequest(urlinfo *url, char *request)
{
	strcpy(request, "GET /");
	
	// construct the path
	if (strlen(url->path))
	{
		strcat(request, url->path);
		if (strlen(url->filename))
			strcat(request, "/");
	}
	if (strlen(url->filename))
		strcat(request, url->filename);
	
	strcat(request, " HTTP/1.0\n");

	// construct headers
	strcat(request, "From: thoffma7@emich.edu\n");
	strcat(request, "User-Agent: crawler/0.40\n");

	// end it
	strcat(request, "\n");
}

int get_links(char *code, parser *p, string_llist *list, int *substrings, int num_substrings)
{
	//fputs("parsing links..\n\n", stdout);
	
	// get list of links using regex
	int codelen = strlen(code);
	parse_all(p, code, codelen, list, substrings, num_substrings);
	//string_llist_printforward(list);	
	return 0;
}

/*
 * Load the code from a webpage.
 * Assumes http request has already been sent
 * Allocates memory; free it when you are done
 */
char *loadPage(int socket)
{
	fputs("loading code..\n", stdout);
	// declare variables
	char buffer[BUFFER_SIZE];
	int bytes_received = 0;
	string_llist list;

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
	char *code = (char *) malloc((list.num_chars + 1) * sizeof(char));

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
