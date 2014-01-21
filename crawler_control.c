#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "utils/domaininfo.h"
#include "utils/string_linked_list.h"
#include "utils/parser.h"
#include "utils/general_utils.h"
#include "utils/socket_utils.h"
#include "utils/status_code_util.h"
#include "utils/url_linked_list.h"
#include "urlinfo.h"
#include "utils/llist.h"
#include "utils/binarytree.h"

#define BUFFER_SIZE 1024

//userAgents randomly selected for http requests to avoid getting blocked by google
char *userAgents[9] =
{"Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.57 Safari/537.36",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9) AppleWebKit/537.71 (KHTML, like Gecko) Version/7.0 Safari/537.71",
    "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:25.0) Gecko/20100101 Firefox/25.0",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.57 Safari/537.36",
    "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.101 Safari/537.36",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.101 Safari/537.36",
    "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.57 Safari/537.36",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.9; rv:25.0) Gecko/20100101 Firefox/25.0",
    "Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.57 Safari/537.36"};

char *loadPage(int socket);
void clean_search_results(string_llist *tags_and_urls, string_llist *destination);
void getRequest(urlinfo *url, char *request);
int get_links(char *code, parser *p, string_llist *list, int *substrings, int num_substrings);
void formatSearchRequest(urlinfo *url, char *request);
void getUserSearchQuery(char *path);
void incrementResultsRequest(char *path, char *clone, int start, int numResults);
string_llist *get_base_graph(char *request, char *port_string, parser *regexparser, urlinfo *searchURL);

/* main routine for testing our crawler's funcitonality */
int main(int argc, char **argv)
{
	char pattern[] = "<a [^>]*?href *= *[\'\"]([^\"\'>]+)[\'\"].*?>";
	parser *regexparser;
	int socket;
	int port = 80;
	char request[BUFFER_SIZE + 100];
	
	url_llist linkstocheck;		// queue of urls to check
	btree domains;			// tree of domains, so we can limit how many pages per domain
	btree linksfound;		// for quick finding so we don't add redunant urls to linkstocheck
	
	urlinfo *newURL = NULL;
	urlinfo seedURL;
	
	char port_string[3];
	sprintf(port_string, "%d", port);

	// initialize parser
	regexparser = init_parser(pattern);

	// initialize queues
	url_llist_init(&linkstocheck);
	btree_init(&domains, (void *)&compare_domain_name);
	btree_init(&linksfound, (void *)&urlcompare);

	//seed list
	if (argc > 1)
		seedURL.host = argv[1];
	else
		seedURL.host = "www.google.com";
	seedURL.path = (char *) malloc(100); //move to getSearchQuery
	//seedURL.path = "";
	seedURL.filename = "";
	seedURL.searchdepth = 0;
	
	string_llist *links_in_search;
	links_in_search = get_base_graph(request, port_string, regexparser, &seedURL);
	
	// convert links found for base graph into urlinfos to be checked 
	char link_in_search[BUFFER_SIZE];
	puts("--- links found -------------------------------");
	while (links_in_search->size)
	{
		string_llist_pop_front(links_in_search, link_in_search);
		urlinfo *urlfromsearch = makeURLfromlink(link_in_search, &seedURL);
		char *tostring = url_tostring(urlfromsearch);
		printf("%s\n", tostring);
		free(tostring);
		url_llist_push_back(&linkstocheck, urlfromsearch);
	}
	puts("------------------------------------------------\n");
	// free links_in_search
	string_llist_free_all(links_in_search);

	int maxperdomain = 8;
	int maxlinks = (argc > 2)? atoi(argv[2]): 4;
	int searchdepth = (argc > 3)? atoi(argv[3]): 1;

	int linkcount = 1;
	while (1)
	{
		// break if have max number of links
		if (linkcount > maxlinks)
			break;
		
		// break if no more links in queue
		if (!linkstocheck.size)
			break;
		
		// convert port# to string
		sprintf(port_string, "%d", port);
		
		newURL = url_llist_pop_front(&linkstocheck);
		
		// break if search depth is too high
		if (newURL->searchdepth > searchdepth)
			continue;	//break;
		
		// break if a second-order redirect
		if (newURL->redirectdepth > 1)
			continue;
			
		printf("link #%d: depth=%d, redirect-depth=%d\n", linkcount, newURL->searchdepth, newURL->redirectdepth);
		
		// generate request	
        	getRequest(newURL, request);
		
		// notify user of url
		printf("URL: %s %s %s\n", newURL->host, newURL->path, newURL->filename);
		
		socket = connect_socket(newURL->host, port_string, stdout);
		if (socket >= 0)
		{
			puts("connected");
			// send http requrest
			send(socket, request, strlen(request), 0);
			puts("sent request");
			// get code
			char *code = loadPage(socket);
			puts("grabbed code");
			// test if the page loaded properly
			int statuscode = get_status_code(code);
			puts("a");
			if (statuscode >= 200 && statuscode < 300)
			{
				// success
				linkcount++;
				
				// create linked list to hold hyperlinks from code
				string_llist links_in_code;// = malloc(sizeof(string_llist));
				string_llist_init(&links_in_code);
				int substrings[] = {1};
				get_links(code, regexparser, &links_in_code, substrings, 1);
				
				// load urls
				printf("links: ");
				char urlstring[BUFFER_SIZE];
				while (links_in_code.size)
				{
					string_llist_pop_front(&links_in_code, urlstring);
					urlinfo *urlfromstring = makeURLfromlink(urlstring, newURL);
						
					// do not add duplicates
					if (btree_find(&linksfound, urlfromstring))
					{
						freeURL(urlfromstring);
						continue;
					}
					btree_insert(&linksfound, urlfromstring);
					
					//int stringindex = string_llist_find(&hostsfound, urlfromstring->host);
					domaininfo *newdomain = domaininfo_init(urlfromstring->host);
					domaininfo *domain = btree_find(&domains, newdomain);
                    
					if (domain)
					{
						if (domain->numpages < maxperdomain)
						{
							domaininfo_pushurl(domain, urlfromstring);
							url_llist_push_back(&linkstocheck, urlfromstring);
						}
						freedomain(newdomain);
					}
					else
					{
						url_llist_push_back(&linkstocheck, urlfromstring);
						domaininfo_pushurl(newdomain, urlfromstring);
						btree_insert(&domains, newdomain);
					}
					printf("x");
				}
				
				printf("\n");
			}
			else if (statuscode >= 300 && statuscode < 400)
			{
				// set up new url from redirect info
				char *redirectURL = get_300_location(code);
				printf("REDIRECT TO %s\n", redirectURL);
				urlinfo *redirect = makeURLfromredirect(redirectURL, newURL);
				//redirect->searchdepth = newURL->searchdepth;
				
				// if new, push new url to the front of the list so it will be checked next
				if (!btree_find(&linksfound, redirect))
				{	
					url_llist_push_front(&linkstocheck, redirect);
					btree_insert(&linksfound, redirect);
				}
				else
				{
					freeURL(redirect);
				}
				free(redirectURL);
			}
			free(code);
		}
		else
			report_error("socket_connect() failed");	

		//close the socket
		int closed = close(socket);
		if (closed)
			puts("error closing socket..");
		
		// pause for user input
		fputs("hit enter to continue..", stdout);
		char input[] = "";
		fgets(input, 2, stdin);
		printf("-----------------------------\n");
	}

	printf("\n%d URLs--\n---------\n", linksfound.numElems);
	char *urlstring;
	int i = 0;
	urlinfo **urlarray = (urlinfo **)btree_toarray(&linksfound);
	for (i = 0; i < linksfound.numElems; i++)
	{
		newURL = urlarray[i];//url_llist_pop_front(&allURLs);
		urlstring = url_tostring(newURL);
	
		printf("%s\n", urlstring);
			
		free(urlstring);
		//freeURL(newURL);
	}
	// free main data structures
	btree_free(&domains, 1);
	btree_free(&linksfound, 1);	// free all links found
	url_llist_free(&linkstocheck); 	// subset of links found; pointers already cleared

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
    	strcat(request, "Host: ");
    	strcat(request, url->host);
    	strcat(request, "\n");
	strcat(request, "From: thoffma7@emich.edu\n");
	strcat(request, "User-Agent: crawler/0.40\n");

	// end it
	strcat(request, "\n");
}



/*  GETREQUEST FROM SEARCHQUERY BRANCH
 * Generates a standard get request from the path
 * using the url->path and puts in *request.
 * Random chooses a user agent which helps query
 * search engines without getting blocked.
 */
void formatSearchRequest(urlinfo *url, char *request)
{
    strcpy(request, "GET /");
    strcat(request, url->path);
    strcat(request, " HTTP/1.0\n");
    strcat(request, "Host: ");
    strcat(request, url->host);
    strcat(request, "\nUser Agent: ");
    strcat(request, userAgents[rand() % 9]);
    strcat(request, "\n\n");
}


int get_links(char *code, parser *p, string_llist *list, int *substrings, int num_substrings)
{
	int codelen = strlen(code);
	parse_all(p, code, codelen, list, substrings, num_substrings);
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

/*
 * Builds the starting graph of urls from repeatedly searching google.
 * Asks the user for a search query and then performs a goole search
 * with results 0-99 shown. The valid urls are scraped from this page
 * and the search is performed with the same query but with results 100-199
 * shown. This is continued trhough 1000 search results. It appears that
 * google still catches on some times and blocks subsequent searches.
 *
 * UPDATE: I was getting around 1000 links but now I'm getting 600ish alot? So
 * I'm trying it with trying to pull 500 results now I'm getting about 500.
 * Bad links are removed: adds, dropdown menus, etc.
 */
string_llist *get_base_graph(char *request, char *port_string,
                             parser *regexparser, urlinfo *searchURL)
{
	char *pathclone;
    
	getUserSearchQuery(searchURL->path);
    
	// to hold search query minus the "start=num" part for easy reload and update
	pathclone = malloc(strlen(searchURL->path)+1);
	strcpy(pathclone, searchURL->path);
    
	// create linked list to hold hyperlinks from code
	string_llist *links_in_code = malloc(sizeof(string_llist));
	string_llist_init(links_in_code);
    
	// linked list to hold <a> tags and associated urls
	string_llist *tags_and_urls = malloc(sizeof(string_llist));
	string_llist_init(tags_and_urls);

	int socket;
    
	int resultsPerPage = 100;
	int i;
	for(i = 1; i < 100; i += resultsPerPage)
	{
	        incrementResultsRequest(searchURL->path, pathclone, i, resultsPerPage);
	        
	        getRequest(searchURL, request);
	        //formatSearchRequest(searchURL, request);
	        
	        socket = connect_socket(searchURL->host, port_string, stdout);
	        if (socket >= 0)
	        {
			// send http requrest
			fputs("\nsending http request: \n", stdout);
			fputs(request, stdout);
	            
			send(socket, request, strlen(request), 0);
	            
			// get code
			char *code = loadPage(socket);
	            
			// get array containing <a> tags and associated urls
			int substrings[] = {0, 1};
			get_links(code, regexparser, tags_and_urls, substrings, 2);
	       		free(code); 
	        }
	        else
	     	       report_error("socket_connect() failed");
	        
        	// close the socket
		close(socket);
	}
	free(pathclone);
	clean_search_results(tags_and_urls, links_in_code);
	
	// free tags_and_urls
	string_llist_free(tags_and_urls);

	return links_in_code;
}

/*
 * Takes array consisting of <a> tags and urls(in alternating order)
 * Picks out which urls are not google's garbage, and pushes the associated url to destination.
 */
void clean_search_results(string_llist *tags_and_urls, string_llist *destination)
{
	char pattern[] = "((^(.(?!http))*$)|(<a onclick|<a class|google.com|facebook|youtube))";
	parser *jargonParser = init_parser(pattern);
    
	int vector[jargonParser->vectorsize];
	string_node *node = tags_and_urls->front;
	while(node)
	{
		// find matches for re with text, optimized w/ study
		pcre_exec(jargonParser->re, NULL,//p->study,
                		node->string, strlen(node->string), 0, 0,
                		vector, jargonParser->vectorsize);
       		
        	// if match not found, add to destination list (which comes after the <a> tag
		if (vector[0] < 0)
		{
			//printf("%s\n", node->next->string);
			string_llist_push_back(destination, node->next->string);
		}

		// increment by 2 to get to the next <a> tag
		node = node->next->next;
	}
}

/*
 * Updates the "num=desiredResults" and "start=desiredStart"
 * portion of the search request. Appends update to *path.
 */
void incrementResultsRequest(char *path, char *clone, int start, int numResults)
{
    strcpy(path, clone);
    
    char startString[4];
    sprintf(startString, "%d", start);
    strcat(path, "&start=");
    strcat(path, startString);
    
    char numResultsString[4];
    sprintf(numResultsString, "%d", numResults);
    strcat(path, "&num=");
    strcat(path, numResultsString);
    
    // for testing
    // fputs(path, stdout);
}

/*
 * Prompts user for a search query and sets
 * path to "/search?q=query1+query2+..."
 */
void getUserSearchQuery(char *path)
{
    char templine[(PATH_LENGTH/2)];
    char *token;
    const char delims[2] = {' ', '\n'};//"\n";
   
    fputs("enter a line of text\n", stdout);
    
     
    if (fgetline(templine, (int)sizeof(templine)-1) > 0) //read a line
    {
        token = strtok(templine, delims);
        
        
        strcpy(path, "search?q=");
        
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
        strcat(path, "&safe=active");
    }
    else
        report_error("getUserSearchQuery failed");
}
