/*
 * =======================================================================
 *
 *       Filename:  testdomain.c
 *
 * =======================================================================
 */

#include "utils/domaininfo.h"
#include "urlinfo.h"
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	int i;
	
	// initialize domain regex
	domain_regex_init();
	
	domaininfo* domaina = domaininfo_init("google.com");

	/* test domains */
	domaininfo* domainb = domaininfo_init("test.google.com");
	domaininfo* domainc = domaininfo_init("google2.com");
	domaininfo* domaind = domaininfo_init("agoogle.net");
	domaininfo *domains[] = {domainb, domainc, domaind};	
	
	for (i = 0; i < 3; i++)
	{
		int cmp = compare_domain_name(domaina, domains[i]);
		printf("%s vs %s: %d\n", domaina->name, domains[i]->name, cmp);
	}
	puts("*********************************");
	
	/* test urls */
	urlinfo* a = makeURLfromlink("test.com/a.html", NULL);
	urlinfo* b = makeURLfromlink("test.com/b.html", NULL);
	urlinfo* c = makeURLfromlink("test.com/c.html", NULL);

	urlinfo* urls[] = {a, b, c};
	
	for (i = 0; i < 3; i++)
	{
		// make associated domain (may be discarded)
		domaininfo *newdomain = domaininfo_init(urls[i]->host);

		int currentnum = domaininfo_numlinks_to_domain(domaina, newdomain);
		
		printf("current num links: %d\n", currentnum);
		if (currentnum < 2)
		{
			puts("adding url");
			domaininfo_puturl(domaina, urls[i]);
		}
		else
		{
			puts("reached max; not adding new link");
			freedomain(newdomain);
		}
	}
	puts("freeing");
	
	freedomain(domaina);

	return 0;
}
