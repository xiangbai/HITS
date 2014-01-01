#ifndef DOMAININFO_H
#define DOMAININFO_H

#include "llist.h"
#include "stdlib.h"

/***************************************
 * domaininfo.h
 * Holds info associated with each host
 *  and methods for using it
 **************************************/

typedef struct domaininfo
{
	char *name;
	size_t numpages;
	llist pages;
} domaininfo;

domaininfo *domaininfo_init(char *domain);

void domaininfo_pushurl(domaininfo *domain, urlinfo *url);

urlinfo *domaininfo_popurl(domaininfo *domain);

/*
 * Returns 1 if provided domains have the same name.
 * Returns 0 otherwise
 */
int compare_domain_name(domaininfo *a, domaininfo *b);

void freedomain(domaininfo *domain);

#endif
