#include "domaininfo.h"
#include <stdlib.h>
#include <string.h>

int compare_domain_name(domaininfo *a, domaininfo *b)
{
	return strcmp(a->name, b->name);
}

domaininfo *domaininfo_init(char *domainname)
{
	domaininfo *domain = (domaininfo *)malloc(sizeof(domaininfo));
	domain->name = (char *)malloc(sizeof(char) * (strlen(domainname) + 1));
	strcpy(domain->name, domainname);
	domain->numpages = 0;
	llist_init(&domain->pages, &compare_domain_name);
	return domain;
}

void domaininfo_pushurl(domaininfo *domain, urlinfo *url)
{
	llist_push_back(&domain->pages, url);
	domain->numpages++;
}

urlinfo *domaininfo_popurl(domaininfo *domain)
{
	urlinfo *output = llist_pop_front(domain);
	if (output)
		domain->numpages--;
	return output;
}

void freedomain(domaininfo *domain)
{
	// empty linked list
	while (domain->numpages)
		llist_pop_back(&domain->pages);
	free(domain);
}
