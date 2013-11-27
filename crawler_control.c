#include <stdlib.h>


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
