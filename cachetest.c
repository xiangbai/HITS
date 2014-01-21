#include "utils/searchcache.h"
#include "utils/string_linked_list.h"
#include "urlinfo.h"
#include <stdio.h>
#include <stdlib.h>

char path[] = "searches";

void get(char *search);
void set(url_llist *urls, char *searchstring);

int main(void)
{
	// get
	get("test search");
	


	// make url list
	url_llist allurls;	
	url_llist_init(&allurls);
	
	// llist
	urlinfo *url1 = makeURL("google.com", NULL);
	urlinfo *url2 = makeURL("test.com", NULL);
	urlinfo *url3 = makeURL("yahoo.com", NULL);
	urlinfo *url4 = makeURL("dogs.com", NULL);
	urlinfo *url5 = makeURL("pizza.com", NULL);
	urlinfo *url6 = makeURL("pizza_party.com", NULL);
	
	url_llist_push_back(&allurls, url1);
	url_llist_push_back(&allurls, url2);
	url_llist_push_back(&allurls, url3);
	url_llist_push_back(&allurls, url4);
	url_llist_push_back(&allurls, url5);
	url_llist_push_back(&allurls, url6);
	
	llist_push_back(&url1->outlinks, url2);
	llist_push_back(&url1->outlinks, url3);
	llist_push_back(&url1->outlinks, url4);
	llist_push_back(&url1->outlinks, url6);

	llist_push_back(&url3->outlinks, url1);
	llist_push_back(&url3->outlinks, url5);
	
	set(&allurls, "test search");
	
	// remove
	//rem("i3");
		
	return 0;
}

void get(char *search)
{
	// get
	url_llist *urls = getcache(path, search);

	if (urls)
	{
		// print urls
		url_node *current_url = urls->front;
		while (current_url)
		{
			printf("url: %s\n", url_tostring(current_url->url));
			llist *list = &current_url->url->outlinks;
			lnode *current_link = list->front;
			while(current_link)
			{
				printf("  link: %s\n", url_tostring((urlinfo *)current_link->data));
				current_link = current_link->next;
			}
	
			current_url = current_url->next;
		}
	}
}

void set(url_llist *urls, char *searchstring)
{
	setcache(path, searchstring, urls);

/*
	//set
	string_llist newurls;
	string_llist_init(&newurls);
	string_llist_push_back(&newurls, "test1");
	string_llist_push_back(&newurls, "test2");
	string_llist_push_back(&newurls, "test3");
	string_llist_push_back(&newurls, "test4");
	string_llist_push_back(&newurls, "NEW!!!!");
	setcache(path, mastername, search, &newurls);
*/
} 
