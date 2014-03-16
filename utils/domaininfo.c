#include "domaininfo.h"
#include "binarytree.h"
#include "parser.h"
#include <stdlib.h>
#include <string.h>

#define DOMAIN_INDEX 	2
#define NUM_SUBSTRINGS	3

char domain_pattern[] = "(\\w+\\.)*(\\w+\\.\\w+)$";
parser *domain_parser;

void domain_regex_init()
{
	domain_parser = init_parser(domain_pattern);
}

/**
 * Get the domain without subdomains.
 * For example, if a domain of test.google.com is provided
 *	google.com will be returned.
 *
 *	domain_name: domain to parse
 *
 * returns the domain without its subdomains.
 */
char *get_name_sans_subdomains(char *domain_name)
{
	int i;
	// initialize array to store substrings
	char *substrings[NUM_SUBSTRINGS];
	for (i = 0; i < NUM_SUBSTRINGS; i++)
		substrings[i] = NULL;
	
	// parse substrings into array
	substrings_to_array(domain_parser, domain_name, strlen(domain_name), 0, substrings);

	// free all strings except the part of the domain we want
	for (i = 0; i < NUM_SUBSTRINGS; i++)
		if (i != DOMAIN_INDEX)
			free(substrings[i]);
	
	// return domain
	return substrings[DOMAIN_INDEX];
}

/**
 * Compare two domains.
 *
 *	a: First domain
 *	b: Second somain
 *
 * Returns strcmp(a->name, b->name)
 *
 * Note: This is not used internally, but will have functionality in collections of domaininfos.
 */
int compare_domain(domaininfo *a, domaininfo *b)
{
	return strcmp(a->name, b->name);
}

int compare_domain_name(char *a, char *b)
{
	// get domains without subdomains
	char *domain_a = get_name_sans_subdomains(a);
	char *domain_b = get_name_sans_subdomains(b);

	// get comparison
	int compare_val = strcmp(domain_a, domain_b);
	
	// free strings
	free(domain_a);
	free(domain_b);
	
	// return value
	return compare_val;
}

/**
 * Compare function for domaininfo and domainlink
 * Used to compare domaininfos to links held in domaininfo.outlinks
 */
int compare_domain_to_link(domaininfo *domain, domainlink *link)
{
	return strcmp(domain->name, link->domain->name);
	// get domains without subdomains
	char *domain_a = get_name_sans_subdomains(domain->name);
	char *domain_b = get_name_sans_subdomains(link->domain->name);
	
	// get comparison
	int compare_val = strcmp(domain_a, domain_b);
	
	// free strings
	free(domain_a);
	free(domain_b);
	
	// return value
	return compare_val;
}

/**
 * Compare function for domainlinks
 * Used for domainlink.pages llist
 */
int compare_domainlink(domainlink* linka, domainlink* linkb)
{
	return strcmp(linka->domain->name, linkb->domain->name);
}

int equals_domainlink(domainlink *a, domainlink *b)
{
	if (compare_domainlink(a, b))
		return 0;
	else return 1;
}

domaininfo *domaininfo_init(char *domainname)
{
	domaininfo *domain = (domaininfo *)malloc(sizeof(domaininfo));
	domain->name = get_name_sans_subdomains(domainname);
	btree_init(&domain->outlinks, (void*)&compare_domain_to_link);
	return domain;
}

void domaininfo_puturl(domaininfo *domain, urlinfo *url)
{
	domainlink* domain_found = (domainlink*)btree_find(&domain->outlinks, (void*)url);
	if (domain_found)
	{
		// add new url to pages linked list
		llist_push_back(&domain_found->pages, (void*)url);
	}
	else
	{
		// no appropriate domainlink found; create it
		domainlink *newdomain = (domainlink*)malloc(sizeof(domainlink));
		newdomain->domain = domaininfo_init(url->host);		// domain name
		llist_init(&newdomain->pages, (int (*)(void*, void*))&equals_domainlink);	// llist for pages
		
		// add new url to pages linked list
		llist_push_back(&newdomain->pages, (void*)url);
		
		// add newdomain to outlinks
		btree_insert(&domain->outlinks, newdomain);
	}
}

urlinfo* domaininfo_findurl(domaininfo *domain, urlinfo *url)
{
	domainlink* domain_found = (domainlink*)btree_find(&domain->outlinks, (void*)url);
	if (domain_found)
		return (urlinfo*)llist_find(&domain_found->pages, url);
	
	return NULL;
}

int domaininfo_numlinks_to_domain(domaininfo *domain, domaininfo *link)
{
	domainlink* domain_found = (domainlink*)btree_find(&domain->outlinks, (void*)link);
	if (domain_found)
		return domain_found->pages.size;
	else
		return 0;
}

void freedomain(domaininfo *domain)
{
	// iterate through domain->outlinks and remove internal data
	treenode *node = domain->outlinks.root;
	int size = domain->outlinks.numElems;
	
	// get the first node
	if (node)
	{
		// get the left-most node
		while (node->left)
			node = node->left;
	}
	
	// remove node's data and iterate through tree
	size_t i;
	for (i = 0; i < size; i++)
	{
		// free current node's data
		domainlink *link = (domainlink*)node->data;
		llist_free(&link->pages, 0);
		
		// get next node
       	if (node->rightthread)
       	    node = node->right;
       	else
       	{
       	    node = node->right;
       	    while(!node->leftthread)
       	        node = node->left;
       	}
	}

	// free the binary tree last
	btree_free(&domain->outlinks, 0);
}
