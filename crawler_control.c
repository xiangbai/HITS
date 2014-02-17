#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "hits.h"
#include "utils/domaininfo.h"
#include "utils/string_linked_list.h"
#include "utils/parser.h"
#include "utils/general_utils.h"
#include "utils/ranksort.h"
#include "utils/socket_utils.h"
#include "utils/status_code_util.h"
#include "utils/url_linked_list.h"
#include "urlinfo.h"
#include "utils/llist.h"
#include "utils/binarytree.h"
#include "utils/searchcache.h"

#define BUFFER_SIZE 1024
#define PORT_80 "80"

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
void getUserSearchQuery(char *path, char *save_query);
void incrementResultsRequest(char *path, char *clone, int start, int numResults);
string_llist *get_potential_root_set(char *request, char *port_string, parser *regexparser, urlinfo *searchURL, char *save_query);
void link_outlinks(llist *urltable, btree *all_links, btree *redirects);
void validate_url_string_list(urlinfo search_engine, string_llist *links_in_search, url_llist *redir_stack, btree *all_links, btree *redir_tree, llist *urls_w_strings_list, char *request, char *port_string, parser *regexparser);
int validate_url_and_populate(urlinfo *cur_url, url_llist *redir_stack, btree *all_links, btree *redir_tree, llist *urls_w_strings_list, char *request, char *port_string, parser *regexparser);
void validate_outlinks_get_backlinks(urlinfo *search_engine, btree *all_links, url_llist *redir_stack, btree *redir_tree, llist *urltable, char *request, char *port_string, parser *regexparser, string_llist *destination);
void back_link_request(char *request, urlinfo *engine, urlinfo *url, int num_links);
void get_back_links(urlinfo *search_engine, urlinfo *current_url, char *port_string, char *request, parser *regexparser, btree *all_links, string_llist *destination);


/* main routine for testing our crawler's funcitonality */
int main()
{
	char pattern[] = "<a [^>]*?href *= *[\'\"]([^\"\'>]+)[\'\"].*?>";
	parser *regexparser;
	char request[BUFFER_SIZE + 100];
	char port_string[3];
	strcpy(port_string, PORT_80);
    char search_string[100];    //holds the user search query for autonaming save files
    
	// initialize parser
	regexparser = init_parser(pattern);
    
    // Declare structures for link processing
	llist urltable;             // llist of url_w_string_list
	btree linksfound;           // for quick finding so we don't add redunant urls to linkstocheck
	btree redirects;            // holds string_redirects
	url_llist redir_stack;      // holds redirected urls
	string_llist backlinks;     // holds backlinks of urls as strings
	
	// Initialize structures
    llist_init(&urltable, (void *)&equals_url_w_string_links);
	btree_init(&linksfound, (void *)&urlcompare);       //all_links
	btree_init(&redirects, (void *)&compare_redirects);
    url_llist_init(&redir_stack);
    string_llist_init(&backlinks);
	
    // Initialize search engine
	urlinfo search_engine;
    search_engine.host = "www.google.com";
	search_engine.path = (char *) malloc(100);
	search_engine.filename = "";
	search_engine.searchdepth = 0;
    search_engine.searchdepth = 0;
    
    
    /***************************** Start alogorithm ********************************/
    
    //Get root set and put potential links in links_in_search
    string_llist *links_in_search;
	links_in_search = get_potential_root_set(request, port_string, regexparser,
                                             &search_engine, search_string);
    
    //Validate and populate the links in links_in_search, which is the potential root set
    validate_url_string_list(search_engine, links_in_search, &redir_stack, &linksfound,
                             &redirects, &urltable, request, port_string, regexparser);
    
	/*
	 TEMPORARILY DISABLE THE GETTING OF BACKLINKS
	 
	 //Validate and populate the outlinks of the root set and get potential backlinks from root set
	 validate_outlinks_get_backlinks(&search_engine, &linksfound, &redir_stack, &redirects,
	 &urltable, request,port_string, regexparser, &backlinks);
	 
	 //Validate backlinks and populate
	 validate_url_string_list(search_engine, &backlinks, &redir_stack, &linksfound, &redirects,
	 &urltable, request, port_string, regexparser);
	 */
    
	//Link outlinks of valid urls to valid urls
    link_outlinks(&urltable, &linksfound, &redirects);
	
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
    setcache(path, search_string, &super_set_list);
	
	/* DEMO: this should be moved out of crawler_control eventually */
	
	// run hits
	int num_links = linksfound.numElems;
	int arbitrary_num_iterations = 10;
	
	// run HITS
	puts("linking inlinks");
	link_inlinks(super_set_array, num_links);
	puts("running hits");
	compute_hub_and_auth(super_set_array, num_links, arbitrary_num_iterations);
	
	// sort
	puts("sorting..");
	rank_sort(super_set_array, num_links);
	
	// display
	printf("----------------------\n The results are in\n----------------------\n");
	printf("score\turl\n");
	for (i = num_links - 1; i >= 0; i--)
	{
		char *url_name = url_tostring(super_set_array[i]);
		printf("%lf\t%s\n", super_set_array[i]->authScore, url_name);
		free(url_name);
	}
	/* END DEMO */
	
    /***** Free Structures *****/
    //TODO
	
	return 0;
}

/*
 * Goes through all of the potential string outlinks in the urltable and searches for them
 * in our data structures. If found the url is pushed to the outlinks of the corresponding url in the urltable.
 */
void link_outlinks(llist *urltable, btree *all_links, btree *redirects)
{
	puts("Linking Outlinks!");
	char string_link[BUFFER_SIZE];
	
	// iterate through each urlinfo
	lnode *current_url_node = urltable->front;
	while (current_url_node)
	{
		url_w_string_links *current_url = (url_w_string_links *)current_url_node->data;
		
		// iterate through each url string in each urlinfo
		while(current_url->outlinks.size)
		{
			string_llist_pop_front(&current_url->outlinks, string_link);
			
			// construct dummy urlinfo and see if it is in all links
			
			urlinfo *desired_url = (urlinfo *)makeURLfromlink(string_link, current_url->url);
			urlinfo *found_url = (urlinfo *)btree_find(all_links, desired_url);
			
			// if found, link url to it
			if (found_url)
				llist_push_back(&current_url->url->outlinks, found_url);
			else
			{
				
				// construct dummy string_redirect and find it in redirects
				string_redirect *desired_redirect = (string_redirect  *)redirect_init(string_link, NULL);
				string_redirect *found_redirect = (string_redirect  *)btree_find(redirects, desired_redirect);
				
				redirect_free(desired_redirect);
                
				if (found_redirect)
					llist_push_back(&current_url->url->outlinks, found_redirect->valid_url);
			}
			freeURL(desired_url);
		}
		current_url_node = current_url_node->next;
	}
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
            strcat(save_query, token);
            token = strtok(NULL, delims);
        }
        
        while (token != NULL)
        {
            strcat(path, "+");
            strcat(save_query, "_");
            strcat(path, token);
            token = strtok(NULL, delims);
        }
        strcat(path, "&safe=active");
    }
    else
        report_error("getUserSearchQuery failed");
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
	urlinfo *temp = (urlinfo*)url_llist_pop_front(redir_stack);
	struct string_redirect *good_bad;
	while (redir_stack->size)
	{
		good_bad = redirect_init(url_tostring(temp), valid_url);
		btree_insert(redir_tree, good_bad);
		freeURL(temp);
		temp = (urlinfo*)url_llist_pop_front(redir_stack);
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
int validate_url_and_populate(urlinfo *cur_url, url_llist *redir_stack, btree *all_links, btree *redir_tree, llist *urls_w_strings_list, char *request, char *port_string, parser *regexparser)
{
    /*
     * INITIAL SETUP: Attempt to connect to cur_url and download the html
     */
    
    //generate http GET request
    getRequest(cur_url, request);
    
    // notify user of url
    printf("\nURL: %s %s %s\n", cur_url->host, cur_url->path, cur_url->filename);
    
    // create socket
    int socket = connect_socket(cur_url->host, port_string, stdout);
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
            
            // insert into redir_links btree if necessary and free links
			add_links_to_redirect_tree(redir_tree, redir_stack, cur_url);
            
			free(code);
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
            
            urlinfo *url_from_redirect = makeURLfromredirect(redirect_url_string, cur_url);
            //redirect->searchdepth = newURL->searchdepth;
            
            url_llist_push_front(redir_stack, cur_url); // push cur_url to redirected urls stack
            
            //search for new url from redirect in all_links
            urlinfo *url_from_all_links = (urlinfo*)btree_find(all_links, url_from_redirect);
            
            //CASE 2a: Redirect is an existing url. Delete the duplicate, create
            //all redirect entries and insert into redir_tree
            if (url_from_all_links) //if temp != NULL, then redirect is NOT a new link
            {
                freeURL(url_from_redirect); //duplicate url so we need to free it
                
                //insert into redir_links btree if necessary and free links
	        	add_links_to_redirect_tree(redir_tree, redir_stack, cur_url);
                free(redirect_url_string);
                free(code);
                return 2;
            }//END CASE 2a
            
            //CASE 2b: redirect is a NEW url: push cur_url to redir_stack and recursively call
            //with url_from_redirect as new urlinfo param
            else
            {
                //free unnecessary stuff before recursive call so we don't have excess memory build-up
				free(redirect_url_string);
                
				free(code);
				
                int ret_val = validate_url_and_populate(url_from_redirect, redir_stack, all_links, redir_tree,
                                                        urls_w_strings_list, request,port_string, regexparser);
				
				return ret_val;
            }//END CASE 2b
        } //END CASE 2
        
        //Case 3: NOT success AND NOT redirect so we have a DEAD END. Free structures and exit
        else
        {
            free(code);
			
			//free URLs on redir_stack
			urlinfo *temp_url;
			while (redir_stack->size)
			{
				puts("popping and freeing redir_stack");
            	temp_url = url_llist_pop_front(redir_stack);
				freeURL(temp_url);
			}
            return 0;
        }//END CASE 3
		
		// close socket
    	close(socket);
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
		puts("making link in validate_url_string_list");
		urlinfo *url_from_search = makeURLfromlink(link_in_search, &origin_url);
		
		if (!url_from_search)	// unable to construct url
			continue;
        else if (btree_find(all_links, url_from_search)) //link already in all_links
            freeURL(url_from_search);
        else
        {
            // construct dummy string_redirect and find it in redirects
            string_redirect *desired_redirect = (string_redirect *)redirect_init(link_in_search, NULL);
            string_redirect *found_redirect = (string_redirect *)btree_find(redir_tree, desired_redirect);
            
            redirect_free(desired_redirect); //if we didn't find the redirect, that DOESN'T mean it's a valid url or redirect
            
            if (found_redirect)
                freeURL(url_from_search);
            else
                validate_url_and_populate(url_from_search, redir_stack, all_links, redir_tree,
                                          urls_w_strings_list, request, port_string, regexparser);
        }
	}
}

/*
 * Interleaves validating the outlinks of a root set member and gathering potential backlinks from a root set member
 */
void validate_outlinks_get_backlinks(urlinfo *search_engine, btree *all_links, url_llist *redir_stack, btree *redir_tree, llist *urltable, char *request, char *port_string, parser *regexparser, string_llist *destination)
{
	
    lnode *current_url_node = urltable->front;
    url_w_string_links *current_url = (url_w_string_links *)current_url_node->data;
    //iterate through the root set in the urltable
    while(current_url_node && current_url->url->searchdepth == 1)
    {
		// iterate through each url string in each urlinfo and validate
		lnode *current_outlink_node = (lnode *)current_url->outlinks.front;
		while (current_outlink_node)
        {
            char *string_link = (char *)current_outlink_node->data;
            
            // construct dummy urlinfo and see if it is in all links
			puts("making url in validate_outlinks_get_backlinks");
            urlinfo *desired_url = (urlinfo *)makeURLfromlink(string_link, current_url->url);
            urlinfo *found_url = (urlinfo *)btree_find(all_links, desired_url);
            
            // if NOT found in all_links, check redir_tree
            if (!found_url)
            {
                // construct dummy string_redirect and find it in redirects
                string_redirect *desired_redirect = (string_redirect *)redirect_init(string_link, NULL);
                string_redirect *found_redirect = (string_redirect *)btree_find(redir_tree, desired_redirect);
                
                redirect_free(desired_redirect);
                
                //if NOT found in redir_tree
                if (!found_redirect)
                {
                    validate_url_and_populate(desired_url, redir_stack, all_links, redir_tree, urltable, request, port_string, regexparser);
                }
                else
                    freeURL(desired_url);
            }
            else
                freeURL(desired_url);
            
            current_outlink_node = current_outlink_node->next;
		}
        
        //do a backlink request on the current url
        get_back_links(search_engine, current_url->url, port_string, request, regexparser, all_links, destination);
        
		current_url_node = current_url_node->next;
		current_url = (url_w_string_links *)current_url_node->data;
    }
}

/*
 * Formats the back link request, returns the format in the param *request
 */
void back_link_request(char *request, urlinfo *engine, urlinfo *url, int num_links)
{
    strcpy(request, "GET /search?q=link:");
    strcat(request, url->host);
    strcat(request, "&start=0&num=50");
	
	strcat(request, " HTTP/1.0\n");
    
	// construct headers
    strcat(request, "Host: www.");
    strcat(request, engine->host);
    strcat(request, "\n");
    
	// end it
	strcat(request, "\n");
}

/*
 * Performs a backlink query for *current_url and returns the results as strings in *destination
 */
void get_back_links(urlinfo *engine, urlinfo *current_url, char *port_string, char *request, parser *regexparser, btree *all_links, string_llist *destination)
{
    //connect to engine
    int socket = connect_socket(engine->host, port_string, stdout);
    
    if (socket >= 0)
    {
        puts("\nconnected to:\n");
        
        back_link_request(request, engine, current_url, 50);
        
        printf("sending backlink request\n%s", request);
        send(socket, request, strlen(request), 0);
        
        char *code = loadPage(socket);
        printf("Request returned with code: %d\n", get_status_code(code));
        
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
        char *link_in_code = NULL;
        string_llist_pop_front(&links_in_code, link_in_code);
        while(link_in_code)
        {
            string_llist_push_back(destination, link_in_code);
            string_llist_pop_front(&links_in_code, link_in_code);
        }
    }
    else
    {
        char *error_msg = NULL;
        sprintf(error_msg,"unable to connect to engine: %s for backlink search on site: %s", engine->host, current_url->host);
        report_error(error_msg);
    }
	close(socket);
}

