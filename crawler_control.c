#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "hits.h"
#include "utils/domaininfo.h"
#include "utils/string_linked_list.h"
#include "utils/parser.h"
#include "utils/progress_bar.h"
#include "utils/general_utils.h"
#include "utils/ranksort.h"
#include "utils/socket_utils.h"
#include "utils/status_code_util.h"
#include "utils/url_linked_list.h"
#include "urlinfo.h"
#include "utils/llist.h"
#include "utils/binarytree.h"
#include "utils/searchcache.h"

#define BUFFER_SIZE 4096
#define PORT_80 "80"

#define ROOT_GRAPH_SIZE			200
#define MAX_BACKLINKS			50

#define MAX_DOMAIN_TO_DOMAIN	4

//#define LOG_INTRINSIC_VALUE
#define LOG_LINKS
#define LOG_ROOT_SET
#define LOG_GOOGLE_REQUESTS
#define LOG_HITS_RESULTS
#define LOG_REDIRECTS
#define LOG_MAKES
#define LOG_DOMAINS


//userAgents randomly selected for http requests to avoid getting blocked by google
char *userAgents[9] =
{	"Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.57 Safari/537.36",
	"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9) AppleWebKit/537.71 (KHTML, like Gecko) Version/7.0 Safari/537.71",
	"Mozilla/5.0 (Windows NT 6.1; WOW64; rv:25.0) Gecko/20100101 Firefox/25.0",
	"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.57 Safari/537.36",
	"Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.101 Safari/537.36",
	"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.101 Safari/537.36",
	"Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.57 Safari/537.36",
	"Mozilla/5.0 (Macintosh; Intel Mac OS X 10.9; rv:25.0) Gecko/20100101 Firefox/25.0",
	"Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.57 Safari/537.36"};


string_redirect *find_redirect(char* url);
char *loadPage(int socket);
void clean_search_results(string_llist *tags_and_urls, string_llist *destination);
void getRequest(urlinfo *url, char *request);
int get_links(char *code, parser *p, string_llist *list, int *substrings, int num_substrings);
void formatSearchRequest(urlinfo *url, char *request);
void getUserSearchQuery(char *path, char *save_query);
void incrementResultsRequest(char *path, char *clone, int start, int numResults);
string_llist *get_potential_root_set(char *request, char *port_string, parser *regexparser, urlinfo *searchURL, char *save_query);
void link_outlinks(llist *urltable, btree *all_links);
void validate_url_string_list(urlinfo search_engine, string_llist *links_in_search, url_llist *redir_stack, btree *all_links, btree *redir_tree, llist *urls_w_strings_list, char *request, char *port_string, parser *regexparser);
int validate_url_and_populate(urlinfo *cur_url, url_llist *redir_stack, btree *all_links, llist *urls_w_strings_list, char *request, char *port_string, parser *regexparser);
void validate_outlinks_get_backlinks(urlinfo *search_engine, btree *all_links, url_llist *redir_stack, btree *redir_tree, llist *urltable, char *request, char *port_string, parser *regexparser, string_llist *destination);
void back_link_request(char *request, urlinfo *engine, urlinfo *url, int num_links);
void get_back_links(urlinfo *search_engine, urlinfo *current_url, char *port_string, char *request, parser *regexparser, string_llist *destination);
void get_back_links2(urlinfo *engine, urlinfo *current_url, char *port_string, char *request, parser *regexparser, string_llist *destination);
void clean_outlinks(url_w_string_links *current_url, int add_urls);
void print_url_table(llist *urltable, char *file_tag);
void save_root_graph(llist *urltable, char *search_string);
int get_outlinks_and_populate(urlinfo *cur_url, btree *all_links, llist *urls_w_strings_list, char *request, char *port_string, parser *regexparser);

#ifdef LOG_INTRINSIC_VALUE
FILE *intrinsic_file;
#endif
#ifdef LOG_REDIRECTS
FILE *redirect_file;
#endif
#ifdef LOG_ROOT_SET
FILE *root_set_file;
#endif
#ifdef LOG_LINKS
FILE *links_file;
#endif
#ifdef LOG_GOOGLE_REQUESTS
FILE *google_req_file;
int potential_backlinks_found = 0;
#endif
#ifdef LOG_MAKES
FILE *success_log;
FILE *fail_log;
#endif
#ifdef LOG_DOMAINS
FILE *domain_log_file;
#endif
#ifdef LOG_MAKES
	FILE *success_log;
	FILE *fail_log;
#endif

//!is_intrinsic requires the global parser intrin_parser be initialized in main before being called
int is_intrinsic(urlinfo *old_page, urlinfo *new_page);
//char intrin_pattern[] = "(\\w+)\\.\\w+$";
//parser *intrin_parser;

// globally available data structures
btree domains;		// hold domaininfos, allowing us to cap links from one domain to another
btree redirects;	// holds string_redirects
llist urltable;             // llist of url_w_string_list
btree linksfound;           // for quick finding so we don't add redunant urls to linkstocheck
url_llist redir_stack;      // holds redirected urls

parser *regexparser;

//global time objects
time_t timer1;
time_t timer2;

/* main routine for testing our crawler's funcitonality */
int main()
{
	timer1 = time(&timer1);
	timer2 = time(&timer2);
	
	
#ifdef LOG_INTRINSIC_VALUE
	intrinsic_file = fopen("intrins.txt", "w");
	printf("%p\n", intrinsic_file);
#endif
	
#ifdef LOG_GOOGLE_REQUESTS
	google_req_file = fopen("google_requests.txt", "w");
	printf("%p\n", google_req_file);
#endif
	
#ifdef LOG_REDIRECTS
	redirect_file = fopen("redirectlog.txt", "w");
#endif

#ifdef LOG_MAKES
	success_log = fopen("success_log.txt", "w");
	fail_log = fopen("fail_log.txt", "w");
#endif
	
#ifdef LOG_DOMAINS
	domain_log_file = fopen("domain_log.txt", "w");
#endif

	
	char pattern[] = "<a [^>]*?href *= *[\'\"]([^\"\'>]+)[\'\"].*?>";
	char request[BUFFER_SIZE + 100];
	char port_string[3];
	strcpy(port_string, PORT_80);
	char search_string[100] = "";    //holds the user search query for autonaming save files
	// initialize domain regex (required before domaininfo may be used)
	domain_regex_init();
	
	// initialize parsers
	regexparser = init_parser(pattern);
	//intrin_parser = init_parser(intrin_pattern);
	
	// Declare structures for link processing
	string_llist backlinks;     // holds backlinks of urls as strings
	
	// Initialize structures
	llist_init(&urltable, (void *)&equals_url_w_string_links);
	btree_init(&linksfound, (void *)&urlcompare);       //all_links
	btree_init(&redirects, (void *)&compare_redirects);
	btree_init(&domains, (int (*)(void*, void*))&compare_domain);
	url_llist_init(&redir_stack);
	string_llist_init(&backlinks);
	
	// Initialize search engine
	urlinfo search_engine;
	search_engine.host = "www.google.com";
	search_engine.path = (char *) malloc(100);
	search_engine.filename = "";
	search_engine.searchdepth = 0;
	search_engine.searchdepth = 0;
	
	//**************************** Start alogorithm ********************************
	
	
#ifdef LOG_GOOGLE_REQUESTS
	fprintf(google_req_file, "Tracing google search request...\n");
	fprintf(google_req_file, "--------------------------------------");
#endif
	
	//Get root set and put potential links in links_in_search
	string_llist *links_in_search;
	links_in_search = get_potential_root_set(request, port_string, regexparser,
											 &search_engine, search_string);
	
#ifdef LOG_GOOGLE_REQUESTS
	fprintf(google_req_file, "\n\n%zd potential urls found from search requests", links_in_search->size);
#endif
	
	//Validate and populate the links in links_in_search, which is the potential root set
	validate_url_string_list(search_engine, links_in_search,
							 &redir_stack, &linksfound, &redirects,
							 &urltable, request, port_string, regexparser);
	
	save_root_graph(&urltable, search_string);
	
	// get last node in urltable
	lnode *last_in_root = urltable.back;
	
#ifdef LOG_LINKS
	char fname[BUFFER_SIZE] = "links_";
	strcat(fname, search_string);
	links_file = fopen(fname, "w");
#endif
	
	int end_of_root_inedx = urltable.size;
	
	char pre_filename[100];
	strcpy(pre_filename, "pre_");
	strcat(pre_filename, search_string);
	print_url_table(&urltable, pre_filename);
	
	//Validate and populate the outlinks of the root set and get potential backlinks from root set
#ifdef LOG_GOOGLE_REQUESTS
	fprintf(google_req_file, "\n\n\nTracing google backlink requests...\n");
	fprintf(google_req_file, "--------------------------------------");
#endif
	validate_outlinks_get_backlinks(&search_engine, &linksfound, &redir_stack, &redirects,
									&urltable, request, port_string, regexparser, &backlinks);
	
#ifdef LOG_GOOGLE_REQUESTS
	fprintf(google_req_file, "\n\nPOTENTIAL BACKLINKS FOUND = %d", potential_backlinks_found);
#endif
	
	/*add the backlinks to the urltable
	validate_url_string_list(search_engine, &backlinks,
							 &redir_stack, &linksfound, &redirects,
							 &urltable, request, port_string, regexparser);
	*/

	// clean the outlink strings in the newly added entries to urltable
	int total_out_in = urltable.size - end_of_root_inedx;
	int counter = 0;
	lnode *current_node = last_in_root->next;
	while (current_node)
	{
		counter++;
		printf("\nCleaning backlink/outlink %d of %d total b/o links in table", counter, total_out_in);
		url_w_string_links *current_url = current_node->data;
		clean_outlinks(current_url, 0);
		current_node = current_node->next;
	}
	
	char post_filename[100];
	strcpy(post_filename, "post_");
	strcat(post_filename, search_string);
	print_url_table(&urltable, post_filename);
	
	//Link outlinks of valid urls to valid urls
	link_outlinks(&urltable, &linksfound);
	
	puts("linked!");
	
	/***** Export graph *****/
	
	char path[] = "searches"; //folder for export
	
	// Convert: void btree -> urlinfo ** -> url_llist
	urlinfo **super_set_array = (urlinfo**)btree_toarray(&linksfound);
	url_llist super_set_list;
	url_llist_init(&super_set_list);
	
	int i = 0;
	for (i = 0; i < linksfound.numElems; i++)
		url_llist_push_back(&super_set_list, super_set_array[i]);
	
	
	// Save Data
	char meta_data[25];
	sprintf(meta_data, "_RG=%d_MBL=%d_MD2D=%d", ROOT_GRAPH_SIZE, MAX_BACKLINKS, MAX_DOMAIN_TO_DOMAIN);
	strcat(search_string, meta_data);
	setcache(path, search_string, &super_set_list);
	
	
	/* DEMO: this should be moved out of crawler_control eventually */
	
	// run hits
	int num_links = (int)linksfound.numElems;
	int arbitrary_num_iterations = 20;
	
	// run HITS
	puts("linking inlinks");
	link_inlinks(super_set_array, num_links);
	puts("running hits");
	compute_hub_and_auth(super_set_array, num_links, arbitrary_num_iterations);
	
	setcache2(path, search_string, &super_set_list);
	
	// sort
	puts("sorting..");
	
	// rank by hub
	rank_sort_hub(super_set_array, num_links);

#ifdef LOG_HITS_RESULTS
	// sort by hub score
	char results_hub_filename[BUFFER_SIZE];
	strcpy(results_hub_filename, "searches/");
	strcat(results_hub_filename, search_string);
	strcat(results_hub_filename, "_results_hub");
	FILE *hits_results_hub_file = fopen(results_hub_filename, "w");
	fprintf(hits_results_hub_file, "ROOT GRAPH SIZE = %d\n", ROOT_GRAPH_SIZE);
	fprintf(hits_results_hub_file, "MAX BACKLINKS = %d\n", MAX_BACKLINKS);
	fprintf(hits_results_hub_file, "MAX DOMAIN-TO-DOMAIN = %d\n", MAX_DOMAIN_TO_DOMAIN);
	fprintf(hits_results_hub_file, "----------------------\n");
	
	for (i = num_links - 1; i >= 0; i--)
	{
		char *url_name = url_tostring(super_set_array[i]);
		fprintf(hits_results_hub_file, "%lf\t%s\n",
				super_set_array[i]->hubScore, url_name);
		free(url_name);
	}
#endif

	// rank by authority
	rank_sort(super_set_array, num_links);

#ifdef LOG_HITS_RESULTS
	// sort by authority score
	char results_filename[BUFFER_SIZE];
	strcpy(results_filename, "searches/");
	strcat(results_filename, search_string);
	strcat(results_filename, "_results");
	FILE *hits_results_file = fopen(results_filename, "w");
	fprintf(hits_results_file, "ROOT GRAPH SIZE = %d\n", ROOT_GRAPH_SIZE);
	fprintf(hits_results_file, "MAX BACKLINKS = %d\n", MAX_BACKLINKS);
	fprintf(hits_results_file, "MAX DOMAIN-TO-DOMAIN = %d\n", MAX_DOMAIN_TO_DOMAIN);
	fprintf(hits_results_file, "----------------------\n");
#endif
	
	// display
	printf("----------------------\n The results are in\n----------------------\n");
	printf("auth\t\thub\t\turl\n");
	for (i = num_links - 1; i >= 0; i--)
	{
		char *url_name = url_tostring(super_set_array[i]);
#ifdef LOG_HITS_RESULTS
		fprintf(hits_results_file, "%lf\t%s\n",
				super_set_array[i]->authScore, url_name);
#endif
		printf("%lf\t%lf\t%s\n", super_set_array[i]->authScore,
			   super_set_array[i]->hubScore, url_name);
		free(url_name);
	}
	
	/* END DEMO */
	
	/***** Free Structures *****/
	//TODO
	
#ifdef LOG_HITS_RESULTS
	fclose(hits_results_file);
#endif
#ifdef LOG_GOOGLE_REQUESTS
	fclose(google_req_file);
#endif
#ifdef LOG_INTRINSIC_VALUE
	fclose(intrinsic_file);
#endif
#ifdef LOG_LINKS
	fclose(links_file);
#endif
#ifdef LOG_MAKES
	fclose(success_log);
	fclose(fail_log);
#endif
	return 0;
}

void save_root_graph(llist *urltable, char* search_string)
{
	char filename[100] = "root_sets/";
	strcat(filename, search_string);
	
	FILE *file;
	if (!(file = fopen(filename, "w")))
		printf("ERROR: %s\n", strerror(errno));
	lnode *current_node = urltable->front;
	while (current_node)
	{
		url_w_string_links *current_url = current_node->data;
		char *url = url_tostring(current_url->url);
		fprintf(file, "%s\n", url);
		free(url);
		current_node = current_node->next;
	}
	fclose(file);
}

/*
 * Goes through all of the potential string outlinks in the urltable and searches for them
 * in our data structures. If found the url is pushed to the outlinks of the corresponding url in the urltable
 */
void link_outlinks(llist *urltable, btree *all_links)
{
#ifdef LOG_LINKS
	fprintf(links_file, "----------------------------\n");
	fprintf(links_file, "--------- LINKING ----------\n");
	fprintf(links_file, "----------------------------\n");
#endif
	
	puts("LINKING");
	char string_link[BUFFER_SIZE];
	int urlindex = -1;
	
	// iterate through each urlinfo
	lnode *current_url_node = urltable->front;
	
	while (current_url_node)
	{
		urlindex++;
		url_w_string_links *current_url = (url_w_string_links *)current_url_node->data;
		
		// iterate through each url string in each urlinfo, converting string_links to urlinfos
		//while(current_url->outlinks.size)
		while(current_url->outlinks.front)
		{
			string_llist_pop_front(&current_url->outlinks, string_link);
			// construct dummy urlinfo and see if it is in all links
			// We don't check for NULL because we've already made this URL before
			urlinfo *desired_url = (urlinfo *)makeURLfromlink(string_link, current_url->url);
			
			// attempt to find the outlink
			urlinfo *outlink = (urlinfo *)btree_find(all_links, desired_url);
			
			printf(" -> %s\n", url_tostring(desired_url));
			
			// free dummy variable
			freeURL(desired_url);
			free(desired_url);
			desired_url = NULL;
			
			// if outlink wasn't found, check redirects
			if (!outlink)
			{
				// find if url is in redirect tree
				string_redirect *found_redirect = find_redirect(string_link);

				
				if (found_redirect)
					outlink = found_redirect->valid_url;
			}
			
			// link current_url->url to outlink
			if (outlink)
			{
				urlinfo *duplicate = llist_find(&current_url->url->outlinks, outlink);
				if (duplicate == NULL)
				{
					llist_push_back(&current_url->url->outlinks, outlink);
					
#ifdef LOG_INTRINSIC_VALUE
					// print the linked urls
					fprintf(intrinsic_file, "%d: ", is_intrinsic(current_url->url, outlink));
					fprintf(intrinsic_file, "linking: %s -> %s\n",
							url_tostring(current_url->url), url_tostring(outlink));
#endif
					
#ifdef LOG_LINKS
					char *temp1 = url_tostring(current_url->url);
					char *temp2 = url_tostring(outlink);
					fprintf(links_file, "%s --> %s\n", temp1,temp2);
					free(temp1);
					free(temp2);
#endif
				}
			}
		}
		current_url_node = current_url_node->next;
	}
}

string_redirect *find_redirect(char* url)
{
	// construct dummy redirect
	string_redirect *desired_redirect = redirect_init(url, NULL);
	
	// attempt to find existing redirect with the same url
	string_redirect *found = (string_redirect*)btree_find(&redirects, desired_redirect);
	
	// free the dummy redirect
	redirect_free(desired_redirect);
	free(desired_redirect);
	
	// return the found url
	return found;
}

/*
 * Formats the standard GET request (i.e., not for backlinks or search queries)
 */
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

/*
 * Parses html links from code, returns strings in *list
 */
int get_links(char *code, parser *p, string_llist *list, int *substrings, int num_substrings)
{
	int codelen = (int)strlen(code);
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
		bytes_received = (int)read(socket, buffer, BUFFER_SIZE - 1);
		buffer[BUFFER_SIZE - 1] = '\0';
		string_llist_push_back(&list, buffer);
	} while (bytes_received > 0);
	
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
string_llist *get_potential_root_set(char *request, char *port_string,
									 parser *regexparser, urlinfo *searchURL, char *save_query)
{
	char *pathclone;
	
	getUserSearchQuery(searchURL->path, save_query);
	
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
	
	int resultsPerPage = 50;
	int i;
	for(i = 1; i < ROOT_GRAPH_SIZE; i += resultsPerPage)
	{
		// get number pages to request
		int pages_to_request;
		if (i + resultsPerPage <= ROOT_GRAPH_SIZE)
			pages_to_request = resultsPerPage;
		else
			pages_to_request = ROOT_GRAPH_SIZE - i;
		
		incrementResultsRequest(searchURL->path, pathclone, i, pages_to_request);
		
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
#ifdef LOG_GOOGLE_REQUESTS
			fprintf(google_req_file, "\n\nsearch request returned with status code %d", get_status_code(code));
#endif
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
				  node->string, (int)strlen(node->string), 0, 0,
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
void getUserSearchQuery(char *path, char *save_query)
{
	char templine[BUFFER_SIZE];
	char *token;
	//const char delims[2] = " \n";//"\n";
	
	fputs("enter a line of text\n", stdout);
	
	
	if (fgets(templine, BUFFER_SIZE, stdin)) //read a line
	{
		printf("you entered = %s\n", templine);
		token = strtok(templine, " ");
		printf("token = %s\n", token);
		
		strcpy(path, "search?q=");
		
		if (token != NULL)
		{
			strcat(path, token);
			strcat(save_query, token);
			token = strtok(NULL, " ");
			printf("token = %s\n", token);
		}
		
		while (token != NULL)
		{
			strcat(path, "+");
			strcat(save_query, "_");
			strcat(path, token);
			strcat(save_query, token);
			token = strtok(NULL, " ");
			printf("token = %s\n", token);
		}
		
		path[strlen(path)-1] = '\0';
		strcat(path, "&safe=active");
	}
	else
		report_error("getUserSearchQuery failed");
	
	save_query[strlen(save_query)-1] = '\0';
	
	printf("save_query = %s\n", save_query);
	printf("path = %s\n", path);
}

/*
 * Construct string_redirects from a url_llist of urls that redirect to a valid url
 * These are then pushed to the btree holding redirects
 *
 * *redir_tree - Tree to insert redirects into
 * *redir_stack - Linked list holding urls that are redirected
 * *valid_url - url that everything in redir_stack is redirected to
 */
void add_links_to_redirect_tree(btree *redir_tree, url_llist *redir_stack, urlinfo *valid_url)
{
	urlinfo *temp = url_llist_pop_front(redir_stack);
	struct string_redirect *good_bad;
	while (redir_stack->size)
	{
		good_bad = redirect_init(url_tostring(temp), valid_url);
		btree_insert(redir_tree, good_bad);
		freeURL(temp);
		free(temp);
		temp = url_llist_pop_front(redir_stack);
	}
}

/*
 * This function uses: request, port_string, and regexparser to attemp to
 * connect to cur_url and download the links as strings. If a redirect occurs
 * the function calls itself recursively and pushes old urls onto the redir_stack.
 * Upon success this function inserts a pointer to cur_url into all_links,
 * and urls_w_strings_list. It also pushes any bad_url_string/good_url pairs to
 * the redir_links btree.
 *
 * *cur_url - pointer to the url to connect to
 * *empty - stack (really a llist) for holding old urls from redirect
 * *all_links - binary tree of all valid urls
 * *redir_links - binary tree of all bad_url_string/good_url redirect pairs
 * *urls_w_strings_list - linked list of valid_url/string_llist_of_potential_outlinks_from_valid_url
 * *request - char * for formatting GET request
 * *port_string - proper port as string for socket connect
 * *regexparser - regex pattern parser for parsing url strings from GET request
 *
 * PRECONDITION - cur_url passed in has NOT been found in all_links tree or redirect tree
 * POSTCONDITION
 * Return Values: -1 = connect_socket failure(actually, program will exit via report_error)
 *                 0 = Link not found. Nothing added to structures
 *                 1 = Valid new link. Inserted into all_links. redir_tree entries added if redirects occurred
 *                 2 = Redirect to existing link. No new links added but NEW redir_tree entries added
 */
int validate_url_and_populate(urlinfo *cur_url, url_llist *redir_stack, btree *all_links, llist *urls_w_strings_list, char *request, char *port_string, parser *regexparser)
{
	/*
	 * INITIAL SETUP: Attempt to connect to cur_url and download the html
	 */
	
	//generate http GET request
	getRequest(cur_url, request);
	
	// notify user of url
	printf("\nURL: %s/%s%s\n", cur_url->host, cur_url->path, cur_url->filename);
	
	// create socket
	int socket = connect_socket(cur_url->host, port_string, stdout);
	if (socket >= 0)
	{
		// send http requrest (MSG__NOSIGNAL prevents exiting on SIGPIPE error)
#if defined(SO_NOSIGPIPE)
		send(socket, request, strlen(request), 0);
#elif defined(MSG_NOSIGNAL)
		send(socket, request, strlen(request), MSG_NOSIGNAL);
#else
		report_error("This program requires systems to define MSG_NOSIGNAL or SO_NOSIGPIPE");
#endif
		
		// get code
		char *code = loadPage(socket);
		
		// test if the page loaded properly
		int statuscode = get_status_code(code);
		printf("status code: %d\n", statuscode);
		
		//CASE 1: We have a successful GET request. URL is valid. Insert into all_links and
		//insert any required redirect entries into the redirect tree
		if (statuscode >= 200 && statuscode < 300) //success, URL is valid - populate structures!
		{
			// create linked list to hold hyperlinks from code
			string_llist *links_in_code = malloc(sizeof(string_llist));
			string_llist_init(links_in_code);
			
			// puts url strings into links_in_code
			int substrings[] = {1};
			get_links(code, regexparser, links_in_code, substrings, 1);
			
			//insert into url_w_string_links linked list
			struct url_w_string_links *url_and_strings = malloc(sizeof(struct url_w_string_links));
			url_and_strings->url = cur_url;
			url_and_strings->outlinks = *links_in_code;
			llist_push_back(urls_w_strings_list, url_and_strings);
			
			//insert into all_links btree
			btree_insert(all_links, cur_url);
			printf("populating %s\n", url_tostring(cur_url));
			// insert into redir_links btree if necessary and free links
			add_links_to_redirect_tree(&redirects, redir_stack, cur_url);
			
			free(code);
			close(socket);
			return 1;
		}//END CASE 1
		
		//CASE 2: We've been redirected. Create url_from_redirect. If url_from_redirect
		//already in all_links. Simply create redirect entries and insert into redirect tree.
		//Otherwise, push cur_url (the url that triggered the redirect) onto the redirect_stack,
		//And call this function recursively with url_from_redirect as the urlinfo param.
		//
		else if ((statuscode >= 300 && statuscode < 400) && (cur_url->redirectdepth < MAX_REDIRECT_DEPTH))
		{
			// set up new url from redirect info
			char *redirect_url_string = get_300_location(code);
			printf("REDIRECT TO %s\n", redirect_url_string);
			
			urlinfo *url_from_redirect = NULL;
			
			if (redirect_url_string)
				url_from_redirect = makeURLfromredirect(redirect_url_string, cur_url);
			//redirect->searchdepth = newURL->searchdepth;
			
			url_llist_push_front(redir_stack, cur_url); // push cur_url to redirected urls stack
			
			urlinfo *url_from_all_links = NULL;
			if (url_from_redirect) //search for new url from redirect in all_links
				url_from_all_links = (urlinfo*)btree_find(all_links, url_from_redirect);
			
#ifdef LOG_REDIRECTS
			fprintf(redirect_file, "%s\n", redirect_url_string);
#endif
			
			//CASE 2a: Redirect is an existing url. Delete the duplicate, create
			//all redirect entries and insert into redir_tree
			if (url_from_all_links) //if temp != NULL, then redirect is NOT a new link
			{
				freeURL(url_from_redirect); //duplicate url so we need to free it
				free(url_from_redirect);
#ifdef LOG_REDIRECTS
				char *r = url_tostring(url_from_all_links);
				fprintf(redirect_file, " IN ALLLINKS %s\n", r);
				free(r);
#endif
				
				//insert into redir_links btree if necessary and free links
				add_links_to_redirect_tree(&redirects, redir_stack, url_from_all_links);//cur_url);
				free(redirect_url_string);
				free(code);
				close(socket);
				return 2;
			}//END CASE 2a
			
			//CASE 2b: redirect is a NEW url: push cur_url to redir_stack and recursively call
			//with url_from_redirect as new urlinfo param
			else if (url_from_redirect)
			{
				//free unnecessary stuff before recursive call so we don't have excess memory build-up
				free(redirect_url_string);
				
#ifdef LOG_REDIRECTS
				char *r = url_tostring(url_from_redirect);
				fprintf(redirect_file, " NEW %s\n", r);
				free(r);
#endif
				
				free(code);
				close(socket);
				
				int ret_val = validate_url_and_populate(url_from_redirect, redir_stack, all_links,
														urls_w_strings_list, request,port_string, regexparser);
				return ret_val;
			}//END CASE 2b
			
			//CASE 2c: makeURLfromredirect returned NULL so dead END. Free structures and exit
			else
			{
#ifdef LOG_REDIRECTS
				fprintf(redirect_file, " FAIL\n\n");
#endif
				
				free(code);
				close(socket);
				urlinfo *temp_url;
				while (redir_stack->size)
				{
					puts("popping and freeing redir_stack");
					temp_url = url_llist_pop_front(redir_stack);
					freeURL(temp_url);
					free(temp_url);
				}
				return 0;
			}//END CASE 2c
		} //END CASE 2
		
		//Case 3: NOT success AND NOT redirect so we have a DEAD END. Free structures and exit
		else
		{
#ifdef LOG_REDIRECTS
			fprintf(redirect_file, "CasE3 fail\n\n");
#endif
			free(code);
			
			//free URLs on redir_stack
			urlinfo *temp_url;
			while (redir_stack->size)
			{
				puts("popping and freeing redir_stack");
				temp_url = url_llist_pop_front(redir_stack);
				freeURL(temp_url);
				free(temp_url);
			}
			close(socket);
			return 0;
		}//END CASE 3
		
	}//end (socket>=0)
	else
		report_error("socket_connect() failed");
	
	return -1;
}

/*
 * Goes through the root set (links_in_search), turns each string into a urlinfo.
 * If the urlinfo is not a duplicate it passes it to validate_url_and_populate.
 */
void validate_url_string_list(urlinfo origin_url, string_llist *links_in_search, url_llist *redir_stack, btree *all_links, btree *redir_tree, llist *urls_w_strings_list, char *request, char *port_string, parser *regexparser)
{
	char link_in_search[BUFFER_SIZE];
	while (links_in_search->size)
	{
		string_llist_pop_front(links_in_search, link_in_search);
		urlinfo *url_from_search = makeURLfromlink(link_in_search, &origin_url);
		
		if (url_from_search)
		{
#ifdef LOG_MAKES
			fprintf(success_log, "makeURLfromLink(%s, %s)\n", link_in_search, url_tostring(&origin_url));
#endif
		}
		
		if (!url_from_search)// unable to construct url
		{
#ifdef LOG_MAKES
			fprintf(fail_log, "makeURLfromLink(%s, %s)\n", link_in_search, url_tostring(&origin_url));
#endif
			continue;
		}
		else if (btree_find(all_links, url_from_search)) //link already in all_links
		{
			freeURL(url_from_search);
			free(url_from_search);
		}
		else
		{
			// check if url is in redirect tree
			string_redirect *found_redirect = find_redirect(link_in_search);
			
			if (found_redirect)
			{
				freeURL(url_from_search);
				free(url_from_search);
			}
			else
			{
				/*
				 validate_url_and_populate(url_from_search, redir_stack, all_links,
				 urls_w_strings_list, request, port_string, regexparser);
				 */
				get_outlinks_and_populate(url_from_search, all_links,
										  urls_w_strings_list, request,port_string, regexparser);
			}
		}
	}
}

/*
 * Remove any outlinks that meet any of the following criteria:
 * 		intrinsic
 * 		possible mass endorsement
 * 		invalid url
 * For all urls that are kept, validate_url_and_populate is called
 */
void clean_outlinks(url_w_string_links *current_url, int add_urls)
{
	// iterate through each url string in each urlinfo and validate
	int num_outlinks_to_check = current_url->outlinks.size;
	char new_string_link[BUFFER_SIZE];
	char request[BUFFER_SIZE + 100];
	
#ifdef LOG_INTRINSIC_VALUE
	fprintf(intrinsic_file, "Links from: %s\n", url_tostring(current_url->url));
#endif
	
	while (num_outlinks_to_check > 0)
	{
		string_llist_pop_front(&current_url->outlinks, new_string_link);
		
		// construct dummy urlinfo and see if it is in all links
		urlinfo *desired_url = makeURLfromlink(new_string_link, current_url->url);
		
		if (desired_url)
		{
#ifdef LOG_MAKES
			fprintf(success_log, "makeURLfromLink(%s, %s)\n", new_string_link, url_tostring(current_url->url));
#endif
			
			int intrin_val = is_intrinsic(current_url->url, desired_url);
			int dont_free_url = 0;
			int okay_to_link = 0;
			urlinfo *found_url = (urlinfo *)btree_find(&linksfound, desired_url);
			
			if (!intrin_val) //if NOT intrinsic
			{
				/*
				 * See if domain of current url may still link to domain of new url
				 */
				// make dummy domaininfos for searching purposes
				domaininfo *desired_domain = domaininfo_init(current_url->url->host);
				
				if (desired_domain->name == NULL)
				{
					freedomain(desired_domain);
					freeURL(desired_url);
					free(desired_url);
					num_outlinks_to_check--;
					continue;
				}
				
				domaininfo *fromdomain = btree_find(&domains, desired_domain);
				
				if (fromdomain == NULL)	// if desired domain not in domains, it is the new fromdomain
				{
					fromdomain = desired_domain;		// set fromdomain for use later in function
					btree_insert(&domains, fromdomain);	// insert new domain into tree
				}
				else
				{	// desired_domain is a duplicate; free it
					freedomain(desired_domain);
					free(desired_domain);
				}
				
				// dummy domain for searching
				domaininfo *todomain = domaininfo_init(desired_url->host);
				
				if (todomain->name == NULL)
				{
					freedomain(todomain);
					freeURL(desired_url);
					free(desired_url);
					num_outlinks_to_check--;
				}
				
				int num_domain_links = domaininfo_numlinks_to_domain(fromdomain, todomain);
				
				// free dummy to-domain
				freedomain(todomain);
				free(todomain);
				
				// if there is room for more links to new domain, add the link to all_links and domain.outlinks
				if (num_domain_links < MAX_DOMAIN_TO_DOMAIN)
				{
					printf("\nclean_outlinks: num_links %d < md2d %d\n", num_domain_links, MAX_DOMAIN_TO_DOMAIN);
					if (found_url) // if NOT found in all_links, check redir_tree
					{
						okay_to_link = 1;
					}
					else
					{
						// check if url is in redirect tree
						string_redirect *found_redirect = find_redirect(new_string_link);
						
						//if NOT found in redir_tree
						if (found_redirect)
						{
							found_url = found_redirect->valid_url;
							okay_to_link = 1;
						}
						else if (add_urls)
						{
							char port[3] = PORT_80;
							found_url = desired_url;
							/*
							 int populate_val = validate_url_and_populate(found_url, &redir_stack,
							 &linksfound, &urltable, request, port, regexparser);
							 
							 printf("pop: %d\n",populate_val);
							 */
							int populate_val = get_outlinks_and_populate(found_url,
																		 &linksfound, &urltable, request, port, regexparser);
							if (populate_val < 2)
								dont_free_url = 1;
							
							if (populate_val > 0)
							{
								okay_to_link = 1;
							}
						}
					}
					
					if (okay_to_link)
					{
#ifdef LOG_INTRINSIC_VALUE
						fprintf(intrinsic_file, "adding %s\n", url_tostring(found_url));
#endif
						string_llist_push_back(&current_url->outlinks, new_string_link);
						domaininfo_puturl(fromdomain, found_url);
						
#ifdef LOG_LINKS
						char *temp1 = url_tostring(current_url->url);
						char *temp2 = url_tostring(found_url);
						fprintf(links_file, "%s --> %s\n", temp1,temp2);
						free(temp1);
						free(temp2);
#endif
					}
				}
				else
				{
					//a temporary output trace
					printf("\n!!!clean_outlinks: num_links %d !< md2d %d\n", num_domain_links, MAX_DOMAIN_TO_DOMAIN);
				}
			}

			if (!dont_free_url)
			{
				freeURL(desired_url);
			}
		}
		else
		{
#ifdef LOG_MAKES
			fprintf(fail_log, "makeURLfromLink(%s, %s)\n", new_string_link, url_tostring(current_url->url));
#endif
		}
		//free(new_string_link);
		num_outlinks_to_check--;
		desired_url = NULL;
	}
}

/*
 * Interleaves validating the outlinks of a root set member and gathering potential backlinks from a root set member
 */
void validate_outlinks_get_backlinks(urlinfo *search_engine, btree *all_links, url_llist *redir_stack, btree *redir_tree, llist *urltable, char *request, char *port_string, parser *regexparser, string_llist *destination)
{
	//char new_string_link[BUFFER_SIZE];
	lnode *current_url_node = urltable->front;
	url_w_string_links *current_url = (url_w_string_links *)current_url_node->data;
	
	// set progress bar for easier monitoring of progress
	long root_size = all_links->numElems;
	progress_bar progbar;
	progress_init(&progbar, root_size, 40);
	
	//iterate through the root set in the urltable
	while(current_url_node && current_url->url->searchdepth == 1)
	{
		printf("Validating outlinks/getting inlinks: %s\n", progbar.display);
		progress_setval(&progbar, progbar.value + 1);
		
		clean_outlinks(current_url, 1);
		
		//check elapsed time since last backlink req.
		timer2 = time(&timer2);
		double diff = difftime(timer2, timer1);
		if (MAX_BACKLINKS)
		{
			//if (diff < 10)
			//{
				int random = rand() % 30 + 20;
				//printf("\nElapsed time since last b-link req. = %lf \n Sleeping for %lf seconds\n", diff, 10-diff);
				sleep(random);
				//sleep(10-diff);
			//}
			//else
			//{
			//	printf("\nElapsed time since last b-link req. = %lf \n Not sleeping!\n", diff);
			//}
		}
		
		//do a backlink request on the current url
		//get_back_links(search_engine, current_url->url, port_string, request, regexparser, destination);
		get_back_links2(search_engine, current_url->url, port_string, request, regexparser, destination);
		
		timer1 = time(&timer1);
		
		// go to next node and get its url_w_string_links
		current_url_node = current_url_node->next;
		if (current_url_node)
			current_url = (url_w_string_links *)current_url_node->data;
	} // end urlinfo loop
}

/*
 * Formats the back link request, returns the format in the param *request
 */
void back_link_request(char *request, urlinfo *engine, urlinfo *url, int num_links)
{
	char params[20];
	sprintf(params, "&start=0&num=%d", num_links);
	
	strcpy(request, "GET /search?q=link:");
	strcat(request, url->host);
	strcat(request, params);
	//strcat(request, "&start=0&num=50");
	
	strcat(request, " HTTP/1.0\n");
	
	// construct headers
	strcat(request, "Host: ");
	strcat(request, engine->host);
	strcat(request, "\n");
	
	// end it
	strcat(request, "\n");
}

/*
 * Performs a backlink query for *current_url and returns the results as strings in *destination
 */
void get_back_links(urlinfo *engine, urlinfo *current_url, char *port_string, char *request, parser *regexparser, string_llist *destination)
{
	//connect to engine
	int socket = connect_socket(engine->host, port_string, stdout);
	int num_backlink_strings = potential_backlinks_found;
	
	if (socket >= 0)
	{
		puts("\nconnected to:\n");
		
		back_link_request(request, engine, current_url, MAX_BACKLINKS);
		
		printf("sending backlink request\n%s", request);
		
#ifdef LOG_GOOGLE_REQUESTS
		fprintf(google_req_file, "\n\nRequest backlinks for : %s", url_tostring(current_url));
#endif
		send(socket, request, strlen(request), 0);
		
		char *code = loadPage(socket);
		printf("\nBacklink request returned with code: %d\n", get_status_code(code));
		
#ifdef LOG_GOOGLE_REQUESTS
		fprintf(google_req_file, "\nBacklink request returned with code: %d", get_status_code(code));
#endif
		
		// create linked list to hold hyperlinks from code
		string_llist links_in_code;
		string_llist_init(&links_in_code);
		
		// linked list to hold <a> tags and associated urls
		string_llist *tags_and_urls = malloc(sizeof(string_llist));
		string_llist_init(tags_and_urls);
		
		
		//get links from code and put in tags_and_urls
		int substrings[] = {0,1};
		get_links(code, regexparser, tags_and_urls, substrings, 2);
		free(code);
		
		//clean tags_and_urls and put in links_in_code
		clean_search_results(tags_and_urls, &links_in_code);
		free(tags_and_urls);
		
		// push the found strings to destination
		char link_in_code[BUFFER_SIZE];
		string_llist_pop_front(&links_in_code, link_in_code);
		
		while(links_in_code.front)
		{
#ifdef LOG_GOOGLE_REQUESTS
			fprintf(google_req_file, "\n\t%s", link_in_code);
			potential_backlinks_found++;
#endif
			string_llist_push_back(destination, link_in_code);
			string_llist_pop_front(&links_in_code, link_in_code);
		}
		
		num_backlink_strings = potential_backlinks_found - num_backlink_strings;
#ifdef LOG_GOOGLE_REQUESTS
		fprintf(google_req_file, "\n%d backlink strings scraped\n", num_backlink_strings);
#endif
	}
	else
	{
		char error_msg[BUFFER_SIZE];
		sprintf(error_msg,"unable to connect to engine: %s for backlink search on site: %s", engine->host, current_url->host);
		report_error(error_msg);
	}
	close(socket);
}

/*
 * Performs a backlink query for *current_url, creates url from strings scraped and links
 * pushes them to current_url's inlinks. Also add's urls to url_table to get it's outlinks
 * as well as all_urls
 */
void get_back_links2(urlinfo *engine, urlinfo *current_url, char *port_string, char *request, parser *regexparser, string_llist *destination)
{
	//connect to engine
	int socket = connect_socket(engine->host, port_string, stdout);
	int num_backlink_strings = potential_backlinks_found;
	
	if (socket >= 0)
	{
		puts("\nconnected to:\n");
		
		back_link_request(request, engine, current_url, MAX_BACKLINKS);
		
		printf("sending backlink request\n%s", request);
		
#ifdef LOG_GOOGLE_REQUESTS
		fprintf(google_req_file, "\n\nRequest backlinks for : %s", url_tostring(current_url));
#endif
		send(socket, request, strlen(request), 0);
		
		char *code = loadPage(socket);
		printf("\nBacklink request returned with code: %d\n", get_status_code(code));
		
#ifdef LOG_GOOGLE_REQUESTS
		fprintf(google_req_file, "\nBacklink request returned with code: %d", get_status_code(code));
#endif
		
		// create linked list to hold hyperlinks from code
		string_llist links_in_code;
		string_llist_init(&links_in_code);
		
		// linked list to hold <a> tags and associated urls
		string_llist *tags_and_urls = malloc(sizeof(string_llist));
		string_llist_init(tags_and_urls);
		
		
		//get links from code and put in tags_and_urls
		int substrings[] = {0,1};
		get_links(code, regexparser, tags_and_urls, substrings, 2);
		free(code);
		
		//clean tags_and_urls and put in links_in_code
		clean_search_results(tags_and_urls, &links_in_code);
		free(tags_and_urls);
		
		// push the found strings to destination
		char link_in_code[BUFFER_SIZE];
		string_llist_pop_front(&links_in_code, link_in_code);
		
		while(links_in_code.front)
		{
#ifdef LOG_GOOGLE_REQUESTS
			fprintf(google_req_file, "\n\t%s", link_in_code);
			potential_backlinks_found++;
#endif
			urlinfo *backlink = makeURLfromlink(link_in_code, current_url);
			
			if (backlink)
			{
				//check domains
				domaininfo *backlink_domain = domaininfo_init(backlink->host);
				domaininfo *desired_backlink_domain = btree_find(&domains, backlink_domain);
				
				if (!desired_backlink_domain)
				{
					desired_backlink_domain = backlink_domain;
					btree_insert(&domains, desired_backlink_domain);
				}
				else
				{
					freedomain(backlink_domain);
					free(backlink_domain);
				}
				
				domaininfo *current_url_domain = domaininfo_init(current_url->host);
				int num_links = domaininfo_numlinks_to_domain(desired_backlink_domain, current_url_domain);
				
				if (num_links < MAX_DOMAIN_TO_DOMAIN)
				{
					printf("\nnum links = %d, MD2D = %d, -> adding inlink\n", num_links, MAX_DOMAIN_TO_DOMAIN);
					domaininfo_puturl(desired_backlink_domain, current_url);
					
					urlinfo *desired_backlink = btree_find(&linksfound, backlink);
					if (desired_backlink)
					{
						// force link between current_url and it's inlink
						llist_push_back(&current_url->inlinks, desired_backlink);
						llist_push_back(&desired_backlink->outlinks, current_url);
						freeURL(backlink);
						backlink = NULL;
					}
					else
					{
						// force link between current_url and it's inlink
						llist_push_back(&current_url->inlinks, backlink);
						llist_push_back(&backlink->outlinks, current_url);
						
						// push to url_table for getting outlinks of the backlink
						url_w_string_links *table_entry = url_w_links_init(backlink);
						llist_push_back(&urltable, table_entry);
						
						// insert into linksfound
						btree_insert(&linksfound, backlink);
					}
				}
				else
				{
					printf("\nbacklink not added because exceeds D-2-D MAX of %d", MAX_DOMAIN_TO_DOMAIN);
					freeURL(backlink);
					backlink = NULL;
				}
			}
			
			//string_llist_push_back(destination, link_in_code);
			string_llist_pop_front(&links_in_code, link_in_code);
		}
		
		num_backlink_strings = potential_backlinks_found - num_backlink_strings;
#ifdef LOG_GOOGLE_REQUESTS
		fprintf(google_req_file, "\n%d backlink strings scraped\n", num_backlink_strings);
#endif
	}
	else
	{
		char error_msg[BUFFER_SIZE];
		sprintf(error_msg,"unable to connect to engine: %s for backlink search on site: %s", engine->host, current_url->host);
		report_error(error_msg);
	}
	close(socket);
}

/* Checks if the new_page is intrinsic (has the same domain) as the old_page
 * @Warning: The global variable intrin_parser must be intialized before this function is called
 */
int is_intrinsic(urlinfo *old_page, urlinfo *new_page)
{
	int compareval = compare_domain_name(old_page->host, new_page->host);
	int output;
	if (compareval)
		output = 0;
	else
		output = 1;
	
#ifdef LOG_INTRINSIC_VALUE
	fprintf(intrinsic_file, "intrinsic compare: %s, %s: %d\n", old_page->host, new_page->host, output);
#endif
	
	return output;
}

void print_url_table(llist *urltable, char *file_tag)
{
	FILE *fp;
	char file_name[BUFFER_SIZE];
	strcpy(file_name, "results/url_table_");
	strcat(file_name, file_tag);
	strcat(file_name, ".txt");
	fp = fopen(file_name, "w");
	
	lnode *current_url_node = urltable->front;
	
	fprintf(fp, "TOTAL URLS = %d\n", urltable->size);
	
	while(current_url_node)
	{
		url_w_string_links *current_url = (url_w_string_links *)current_url_node->data;
		fprintf(fp, "\nURL: %s", url_tostring(current_url->url));
		fprintf(fp,"\n\tOutlinks:");
		string_node *temp_outlink = current_url->outlinks.front;
		
		while(temp_outlink)
		{
			fprintf(fp, "\n\t%s", temp_outlink->string);
			temp_outlink = temp_outlink->next;
		}
		fprintf(fp, "\n");
		current_url_node = current_url_node->next;
		
	}
	
	fclose(fp);
	
}

int get_outlinks_and_populate(urlinfo *cur_url, btree *all_links, llist *urls_w_strings_list, char *request, char *port_string, parser *regexparser)
{
	/*
	 * INITIAL SETUP: Attempt to connect to cur_url and download the html
	 */
	
	//make entry for table
	//struct url_w_string_links *url_and_strings = malloc(sizeof(struct url_w_string_links));
	//url_and_strings->url = cur_url;
	struct url_w_string_links *url_and_strings = url_w_links_init(cur_url);
	
	//generate http GET request
	getRequest(cur_url, request);
	
	// notify user of url
	printf("\nURL: %s/%s%s\n", cur_url->host, cur_url->path, cur_url->filename);
	
	// create socket
	int socket = connect_socket(cur_url->host, port_string, stdout);
	if (socket >= 0)
	{
		// send http requrest (MSG__NOSIGNAL prevents exiting on SIGPIPE error)
#if defined(SO_NOSIGPIPE)
		send(socket, request, strlen(request), 0);
#elif defined(MSG_NOSIGNAL)
		send(socket, request, strlen(request), MSG_NOSIGNAL);
#else
		report_error("This program requires systems to define MSG_NOSIGNAL or SO_NOSIGPIPE");
#endif
		
		// get code
		char *code = loadPage(socket);
		
		// test if the page loaded properly
		int statuscode = get_status_code(code);
		printf("status code: %d\n", statuscode);
		
		if (statuscode >= 200 && statuscode < 300) //success - get outlinks
		{
			// create linked list to hold hyperlinks from code
			string_llist *links_in_code = malloc(sizeof(string_llist));
			string_llist_init(links_in_code);
			
			// puts url strings into links_in_code
			int substrings[] = {1};
			get_links(code, regexparser, links_in_code, substrings, 1);
			
			//add outlinks to url_table entry
			url_and_strings->outlinks = *links_in_code;
		}
		
		free(code);
		close(socket);
	}
	else
		puts("couldn't connect socket");
	
	//insert into url_table
	llist_push_back(urls_w_strings_list, url_and_strings);
	
	//insert into all_links btree
	btree_insert(all_links, cur_url);
	
	return 1; //to keep things happy, returns 1 no matter what right now
}
