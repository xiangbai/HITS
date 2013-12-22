#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils/string_linked_list.h"
#include "utils/parser.h"
#include "utils/general_utils.h"
#include "utils/socket_utils.h"
#include "url_queue.h"
#include "urlinfo.h"

#define BUFFER_SIZE 1024

char *loadPage(int socket);
void getRequest(urlinfo *url, char *request);
int get_links(char *code, parser *p, string_llist *list);
void getRequest(urlinfo *url, char *request);
void getUserSearchQuery(char *path);

/* main routine for testing our crawler's funcitonality */
int main(void)
{
	char pattern[] = "<a [^>]*?href *?= *?[\'\"]([^\">]+)[\'\"].*?>";
	parser *regexparser;
	int socket;
	int port = 80;
	char request[PATH_LENGTH + 100]; //max path size + space for get request
	//char request[20];
    struct url_queue linkstocheck;
	struct url_queue allURLs;
	struct urlinfo *newURL;
	struct urlinfo seedURL;
    //following for testing
    struct urlinfo seedURL2;
	
	char port_string[3];
	sprintf(port_string, "%d", port);

    // initialize parser
	regexparser = init_parser(pattern);

	// initialize queues
	linkstocheck.size = 0;
	allURLs.size = 0;

	//seed list
	seedURL.host = "www.google.com";
    //seedURL.path = "/";
    getUserSearchQuery(seedURL.path);
	
    //following lines for testing
    seedURL2.host = "www.yahoo.com";
    seedURL2.path = "/";
    
    
	push_url(&linkstocheck, &seedURL);
    //following line for testing
    push_url(&linkstocheck, &seedURL2);
	//push_url(&allURLs, &seedURL);
	
	int i;
	for(i = 0; i < 2; i++)
	{
		//socket = socket(AF_INET, SOCK_STREAM, 0);
		newURL = pop_url(&linkstocheck);
		
		// temporarily replaced the following line for testing
		getRequest(newURL, request);
		
		socket = connect_socket(newURL->host, port_string, stdout);
		if (socket >= 0)
		{
			// send http requrest
			fputs("sending http request: ", stdout);
			fputs(request, stdout);
			send(socket, request, strlen(request), 0);
			
			// get code
			char *code = loadPage(socket);
				
            //following line for testing
            //fputs(code, stdout);
            
			// create linked list to hold hyperlinks from code
			string_llist *links_in_code = malloc(sizeof(string_llist));
			string_llist_init(links_in_code);
			get_links(code, regexparser, links_in_code);

			// parse links, push to both queues, -1 for failure
			//if (readHTML(socket, host, path, &linkstocheck) == -1)
			//	report_error("readHTML error");

		}
		else
			report_error("socket_connect() failed");
		
		// close the socket
		close(socket);
	}

	print_queue(&allURLs);
	
	return 0;
}

void getRequest(urlinfo *url, char *request)
{
	strcpy(request, "GET ");
	strcat(request, url->path);
	strcat(request, " HTTP/1.0\n");
    strcat(request, "Host: ");
    strcat(request, url->host);
    strcat(request, "\n\n");
}

void getUserSearchQuery(char *path)
{
    char templine[(PATH_LENGTH/2)];
    char *token;
    const char delims[2] = {' ', '\n'};//"\n";
    
    fputs("enter a line of text\n", stdout);
    
    if (fgetline(templine, (int)sizeof(templine)-1) > 0) //read a line
    {
        token = strtok(templine, delims);
        strcpy(path, "/search?q=");
        
        if (token != NULL)
        {
            strcat(path, token);
            token = strtok(NULL, delims);
        }
        
        while (token != NULL)
        {
            strcat(path, "+");
            strcat(path, token);
            token = strtok(NULL, delims);
        }
        
        //attempts to search google with more organized xml output
        //strcat(path, "&site=default_collection&client=default_frontend&output=xml_no_dtd&proxystylesheet=default_frontend");

        
        
        //this works but is in html and has way too many superfluous links
        //strcat(path, "&site=default_collection&client=default_fronted&output=html");
        
    }
    else
        report_error("getUserSearchQuery failed");

}

int get_links(char *code, parser *p, string_llist *list)
{
	fputs("parsing links..\n\n", stdout);
	// get list of links using regex
	int codelen = strlen(code);
	parse_all(p, code, codelen, list);
	string_llist_printforward(list);	

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
