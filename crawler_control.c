#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "utils/string_linked_list.h"
#include "utils/parser.h"
#include "utils/general_utils.h"
#include "utils/socket_utils.h"
#include "url_queue.h"
#include "urlinfo.h"

#define BUFFER_SIZE 1024

char *loadPage(int socket);

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

void getRequest(urlinfo *url, char *request);
int get_links(char *code, parser *p, string_llist *list);
void getRequest(urlinfo *url, char *request);
void getUserSearchQuery(char *path);
void incrementResultsRequest(char *path, char *clone, int start, int numResults);
string_llist *get_base_graph(char *request, char *port_string, parser *regexparser);

/* main routine for testing our crawler's funcitonality */
int main(void)
{
    char pattern[] = "<a [^>]*?href *?= *?[\'\"]([^\">]+)[\'\"].*?>";
    parser *regexparser;
    int socket;
    int port = 80;
    char request[PATH_LENGTH + 100]; //max path size + space for get request
    char *pathclone;
    struct url_queue linkstocheck;
    struct url_queue allURLs;
    struct urlinfo *newURL;
    struct urlinfo seedURL;
    
    char port_string[3];
    sprintf(port_string, "%d", port);
    
    // initialize parser
    regexparser = init_parser(pattern);
    
    // initialize queues
    linkstocheck.size = 0;
    allURLs.size = 0;
    
    //search engine seed
    seedURL.host = "www.google.com";
    getUserSearchQuery(seedURL.path);
    
    // to hold search query minus the "start=num" part for easy reload and update
    pathclone = malloc(strlen(seedURL.path)+1);
    strcpy(pathclone, seedURL.path);
    
    // create linked list to hold hyperlinks from code
    string_llist *links_in_code = malloc(sizeof(string_llist));
    string_llist_init(links_in_code);
    
    int resultsPerPage = 100;
    int i;
    for(i = 0; i < 1000; i += resultsPerPage)
    {
        incrementResultsRequest(seedURL.path, pathclone, i, resultsPerPage);
        
        getRequest(&seedURL, request);
        
        socket = connect_socket(seedURL.host, port_string, stdout);
        if (socket >= 0)
        {
            // send http requrest
            fputs("sending http request: ", stdout);
            fputs(request, stdout);
            
            //wait for some time so google don't block us!
            sleep( (rand() % 7) + 2);
            send(socket, request, strlen(request), 0);
            
            // get code
            char *code = loadPage(socket);
            
            //following line for testing
            fputs(code, stdout);
            
            get_links(code, regexparser, links_in_code);
            
            //following lines for testing purposes
            //clean_search_results(links_in_code);
            //printf("print cleaned results form search %d to %d, ",i, i+resultsPerPage-1);
            //string_llist_printforward(links_in_code);
        }
        else
            report_error("socket_connect() failed");
        
        // close the socket
        close(socket);
    }
    clean_search_results(links_in_code);
    fputs("print all cleaned", stdout);
    string_llist_printforward(links_in_code);
    print_queue(&allURLs);
    
    return 0;
}

/*
 * Builds the starting graph of urls from repeatedly searching google.
 * Asks the user for a search query and then performs a goole search
 * with results 0-99 shown. The valid urls are scraped from this page 
 * and the search is performed with the same query but with results 100-199
 * shown. This is continued trhough 1000 search results. Currently parsing
 * approximately 800-1040 links when all is said in done.  
 * Bad links are removed: adds, dropdown menus, etc.
 */
string_llist *get_base_graph(char *request, char *port_string, parser *regexparser)
{
    struct urlinfo seedURL;
    char *pathclone;
    
    //search engine seed
    seedURL.host = "www.google.com";
    getUserSearchQuery(seedURL.path);
    
    // to hold search query minus the "start=num" part for easy reload and update
    pathclone = malloc(strlen(seedURL.path)+1);
    strcpy(pathclone, seedURL.path);
    
    // create linked list to hold hyperlinks from code
    string_llist *links_in_code = malloc(sizeof(string_llist));
    string_llist_init(links_in_code);
    
    int socket;
    
    int resultsPerPage = 100;
    int i;
    for(i = 0; i < 1000; i += resultsPerPage)
    {
        incrementResultsRequest(seedURL.path, pathclone, i, resultsPerPage);
        
        getRequest(&seedURL, request);
        
        socket = connect_socket(seedURL.host, port_string, stdout);
        if (socket >= 0)
        {
            // send http requrest
            fputs("sending http request: ", stdout);
            fputs(request, stdout);
            
            //optional: wait for some time so google don't block us!
            //sleep( (rand() % 45) + 7);
            send(socket, request, strlen(request), 0);
            
            // get code
            char *code = loadPage(socket);
            
            //following line for testing
            //fputs(code, stdout);
            
            get_links(code, regexparser, links_in_code);
            
            //following lines for testing purposes
            //clean_search_results(links_in_code);
            //printf("print cleaned results form search %d to %d, ",i, i+resultsPerPage-1);
            //string_llist_printforward(links_in_code);
        }
        else
            report_error("socket_connect() failed");
        
        // close the socket
        close(socket);
    }
    //test output internally
    //clean_search_results(links_in_code);
    //fputs("print all cleaned", stdout);
   // string_llist_printforward(links_in_code);
    return links_in_code;
}

/*
 * Generates a standard get request from the path using the url->path and puts in *request.
 * Random chooses a user agent which helps query search engines without getting blocked.
 */
void getRequest(urlinfo *url, char *request)
{
    strcpy(request, "GET ");
    strcat(request, url->path);
    strcat(request, " HTTP/1.0\n");
    strcat(request, "Host: ");
    strcat(request, url->host);
    strcat(request, "\nUser Agent: ");
    strcat(request, userAgents[rand() % 9]);
    strcat(request, "\n\n");
}

/*
 * Updates the "num=desiredResults" and "start=desiredStart" portion of the search request.
 * Appends update to *path.
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
    
    fputs(path, stdout);
}

/*
 * Prompts user for a search query and sets *path to "/search?q=query1+query2+..."
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
        strcat(path, "&safe=active");
    }
    else
        report_error("getUserSearchQuery failed");
}

/*
 * Parses valid hyperlinks from char *code using parser *p, and puts in *list
 */
int get_links(char *code, parser *p, string_llist *list)
{
    fputs("parsing links..\n\n", stdout);
    // get list of links using regex
    int codelen = strlen(code);
    parse_all(p, code, codelen, list);
    
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
