/*
 * =======================================================================
 *
 *       Filename:  testdomain.c
 *
 * =======================================================================
 */

#include "utils/domaininfo3.h"
#include "urlinfo.h"
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	int i;
	
	// initialize domain regex
	domain_regex_init();
	
	domaininfo3* domaina = domaininfo_init("stackexchange.com");

	/* test domains */
	domaininfo3* domainb = domaininfo_init("math.stackexchange.com");
	domaininfo3* domainc = domaininfo_init("google.stackexchange.com");
	domaininfo3* domaind = domaininfo_init("stacks.stackexchange.com");
	domaininfo3 *domains[] = {domainb, domainc, domaind};	
	
	for (i = 0; i < 3; i++)
	{
		int cmp = compare_domain(domaina, domains[i]);
		printf("%s vs %s: %d\n", domaina->name, domains[i]->name, cmp);
	}
	puts("*********************************");
	
	/* test urls */
	urlinfo* a = makeURLfromlink("stackexchange.com/a.html", NULL);
	urlinfo* b = makeURLfromlink("test.stackexchange.com/b.html", NULL);
	urlinfo* c = makeURLfromlink("food.stackexchange.com/c.html", NULL);
	urlinfo* d = makeURLfromlink("dog.stackexchange.com/c.html", NULL);
	urlinfo* e = makeURLfromlink("www.w.stackexchange.com/c.html", NULL);
	urlinfo* f = makeURLfromlink("ask.stackexchange.com/c.html", NULL);
	urlinfo* g = makeURLfromlink("null.stackexchange.com/c.html", NULL);
	urlinfo* h = makeURLfromlink("last.stackexchange.com/c.html", NULL);

	urlinfo* urls[] = {a, b, c, d, e, f, g, h};
	
	for (i = 0; i < 8; i++)
	{
		// make associated domain (may be discarded)
		domaininfo3 *newdomain = domaininfo_init(urls[i]->host);

		int currentnum = domaininfo_numlinks_to_domain(domaina, newdomain);
		
		printf("current num links: %d\n", currentnum);
		if (currentnum < 2)
		{
			puts("adding url");
			domaininfo3 *dummy = domaininfo_init(urls[i]->host);
			domaininfo_puturl(domaina, dummy);
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
