#include "utils/searchcache.h"
#include "utils/string_linked_list.h"
#include <stdio.h>

char path[] = "searches";
char mastername[] = "_MASTER";

void rem(char *search);
void get(char *search);
void set(char *search);

int main(void)
{
	get("i3");
	
	set("i3");
	
	// remove
	//rem("i3");
		
	return 0;
}
void rem(char *search)
{
	removecache(path, mastername, search);
}

void get(char *search)
{
	// get
	string_llist *urls = getcache(path, mastername, search);
	
	if (urls)
	{
		string_llist_printforward(urls);
		string_llist_free(urls);
		free(urls);
	}
}

void set(char *search)
{

	//set
	string_llist newurls;
	string_llist_init(&newurls);
	string_llist_push_back(&newurls, "test1");
	string_llist_push_back(&newurls, "test2");
	string_llist_push_back(&newurls, "test3");
	string_llist_push_back(&newurls, "test4");
	string_llist_push_back(&newurls, "NEW!!!!");
	setcache(path, mastername, search, &newurls);
} 
