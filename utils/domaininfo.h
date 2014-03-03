#ifndef DOMAININFO_H
#define DOMAININFO_H

//#include "llist.h"
#include "binarytree.h"
#include "parser.h"
#include "../urlinfo.h"
#include <stdlib.h>

/************************************************
 * domaininfo.h
 * Holds links from one domain to other domains
 *  and the urls linked to by the initial domain
 * 
 * Sample Structure:
 * 
 * domaininfo	domainlink	   urlinfo
 * ----------	----------	-------------------
 * 						   *-- a.com/index.html
 * 						  /
 *			   *- a.com -*
 * 			  /		 	  \
 * 			 /			   *-- a.com/test.html
 * d.com   -*
 * 			 \			   *-- b.com/index.html
 * 			  \     	  /
 * 			   *- b.com -*
 * 						  \
 * 						   *-- b.com/hello.html
 * 
 ************************************************/

typedef struct domaininfo
{
	char *name;
	//llist pages;
	btree outlinks;	// hold domainlinks
} domaininfo;

/* 
 * Struct to hold links from one domain to another
 * Kleinberg recommends no more than 4-8 links form one domain to another
 * Note: domainlinks are not to hold all pages on a domain.
 *	They are to hold links from a domaininfo to pages on the another given domain
 *	Many domainlinks will reference the same domaininfo
 * 	IE if a.com links to b.com/cs.html and d.com links b.com/e.html:
 *		A domaininfo for a.com holds domainlink for b.com in its binary tree
 *		That domainlink holds a reference to c.html in its llist.
 *		The domaininfo for d.com holds a different domainlink for b.com
 */
typedef struct domainlink
{
	domaininfo *domain;
	llist pages;		// holds pages in domain linked to from domaininfo
} domainlink;

/**
 * Initialize regex for making domaininfo
 * This must be called before initializing domaininfos.
 */
void domain_regex_init();

/**
 * Initialize domaininfo.
 * 	domain: Provided domain.
 * domaininfo will not keep subdomains.
 */
domaininfo *domaininfo_init(char *domain);

/**
 * Add link to url from domaininfo.
 * If domain.outlinks doesn't contain url's domain, it is added
 *	domain:	domain to hold the link
 *	url: url domain will link to
 */
void domaininfo_puturl(domaininfo *domain, urlinfo *url);

/**
 * Get the number links from one domain to another
 *	domain:	domain holding the links
 *	link:	domain linked to by domain
 */
int domaininfo_numlinks_to_domain(domaininfo *domain, domaininfo *link);

/*
 * Compare 2 domains
 * Returns 1 if provided domains have the same name.
 * Returns 0 otherwise
 */
int compare_domain_name(domaininfo *a, domaininfo *b);

/**
 * Free data held by domain
 * This will not free the domain name or associated urlinfos
 */
void freedomain(domaininfo *domain);

#endif
