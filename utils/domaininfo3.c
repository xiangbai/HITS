#include "domaininfo3.h"
#include "binarytree.h"
#include "parser.h"
#include "../urlinfo.h"
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
int compare_domain(domaininfo3 *a, domaininfo3 *b)
{
	return strcmp(a->name, b->name);
}

int compare_domain_name(char *a, char *b)
{
	// get domains without subdomains
	char *domain_a = get_name_sans_subdomains(a);
	char *domain_b = get_name_sans_subdomains(b);

	int compare_val;

	// get comparison
	if (!domain_b)
		compare_val = -1;
	else if (!domain_a)
		compare_val = 1;
	else
		compare_val = strcmp(domain_a, domain_b);
	
	// free strings
	if (domain_a)
		free(domain_a);
	if (domain_b)
		free(domain_b);
	
	// return value
	return compare_val;
}

/**
 * Compare function for domaininfo and domainlink
 * Used to compare domaininfos to links held in domaininfo.outlinks
 */
/*
int compare_domain_to_link(domaininfo2 *domain, urlinfo *link)
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
*/
/**
 * Compare function for domainlinks
 * Used for domainlink.pages llist
 */
int compare_domainlinks(domainlink* linka, domainlink* linkb)
{
	//return urlcompare(linka->target_domain, linkb->target_domain);
	return compare_domain(linka->target_domain, linkb->target_domain);
}

int equals_domainlink(domainlink *a, domainlink *b)
{
	if (compare_domainlinks(a, b))
		return 0;
	else return 1;
}

domaininfo3 *domaininfo_init(char *domainname)
{
	domaininfo3 *domain = (domaininfo3*)malloc(sizeof(domaininfo3));
	domain->name = get_name_sans_subdomains(domainname);
	btree_init(&domain->outlinks, (void*)&compare_domainlinks);
	return domain;
}

void domaininfo_puturl(domaininfo3 *fromdomain, domaininfo3 *todomain)
{
	// construct dummy domainlink for url and see if it exists
	domainlink* dummy_domainlink = (domainlink*)malloc(sizeof(domainlink));
	dummy_domainlink->target_domain = todomain;
	domainlink* domainlink_found = (domainlink*)btree_find(&fromdomain->outlinks, (void*)dummy_domainlink);
	
	if (domainlink_found)
	{
		// add new url to pages linked list
		free(dummy_domainlink);
		domainlink_found->num_links++;
	}
	else
	{
		// no appropriate domainlink found; add it
		dummy_domainlink->num_links = 1;
		btree_insert(&fromdomain->outlinks, dummy_domainlink);
	}
}
/*
urlinfo* domaininfo_findurl(domaininfo *domain, urlinfo *url)
{
	domainlink *dummy_domainlink = (domainlink*)malloc(sizeof(domainlink));
	dummy_domainlink->target_url = url;
	
	domainlink* domainlink_found = btree_find(&domain->outlinks, dummy_domainlink);
	if (domainlink_found)
		return domainlink_found->target_url;//(urlinfo*)llist_find(&domain_found->pages, url);
	
	return NULL;
}
*/
domainlink* domaininfo_find_domainlink(domaininfo3 *fromdomain, domaininfo3 *todomain)
{
	domainlink *dummy_domainlink = (domainlink*)malloc(sizeof(domainlink));
	dummy_domainlink->target_domain = todomain;
	
	domainlink* domainlink_found = btree_find(&fromdomain->outlinks, dummy_domainlink);
	free(dummy_domainlink);
	if (domainlink_found)
		return domainlink_found;//(urlinfo*)llist_find(&domain_found->pages, url);
	
	return NULL;
}

int domaininfo_numlinks_to_domain(domaininfo3 *fromdomain, domaininfo3 *todomain)
{
	domainlink* domainlink_found = domaininfo_find_domainlink(fromdomain, todomain);
	//domainlink* domain_found = (domainlink*)btree_find(&domain->outlinks, (void*)link);
	if (domainlink_found)
		return domainlink_found->num_links;
	else
		return 0;
}

void freedomain(domaininfo3 *domain)
{
	// iterate through domain->outlinks and remove internal data
/*	treenode *node = domain->outlinks.root;
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
*/
	// free the binary tree last
	btree_free(&domain->outlinks, 0);
	free(domain->name);
}

void print_domains_to_file(FILE *fp, btree *domains)
{
	/*
	domaininfo **array = (domaininfo **)btree_toarray(domains);
	int i;
	
	for (i = 0; i < domains->numElems; i++)
	{
		domainlink **
	}
	 */
}
