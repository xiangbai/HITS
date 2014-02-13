#include "hits.h"
#include "urlinfo.h"
#include "utils/llist.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
int main(int argc, char *argv[])
{
	urlinfo *a = makeURLfromlink("www.a.com", NULL);
	urlinfo *b = makeURLfromlink("www.b.com", NULL);
	urlinfo *c = makeURLfromlink("www.c.com", NULL);

	llist_push_back(&a->outlinks, b);
	llist_push_back(&a->outlinks, c);
	llist_push_back(&b->outlinks, c);

	int iterations = (argc > 1)? atoi(argv[1]): 10;
	printf("iterations: %d\n", iterations);
	char names[] = {'a','b','c'};
	urlinfo *urls[] = {a, b, c};
	link_inlinks(urls, 3);
	compute_hub_and_auth(urls, 3, iterations);

	int i;
	for (i = 0; i < 3; i++)
		printf("%c:\tauth: %lf\n\thub: %lf\n", names[i], urls[i]->authScore, urls[i]->hubScore);
	
	return 0;
}
*/

void link_inlinks(urlinfo *urls[], int size)
{
	int i;
	// iterate through urls
	for (i = 0; i < size; i++)
	{
		// iterate through url's outlinks
		lnode *current_node = urls[i]->outlinks.front;
		while (current_node)
		{
			// get urlinfo from node
			urlinfo *url = (urlinfo*)current_node->data;
			
			// for each outlink, add url to its outlink
			llist_push_back(&url->inlinks, urls[i]);
			
			// increment node
			current_node = current_node->next;
		}
	}
}

void compute_hub_and_auth(urlinfo *urls[], int size, int iterations)
{

	/* 
	 * Algorithm from Kleinberg: 
	 * 	"Authoritative Sources in a Hyperlinked Environment"
	 * 	pg. 9 (modified)
	 * 
	 * 	x0 = 1
	 *	y0 = 1
	 * 	for i = 1, 2, ..., k
	 * 		apply I		(auth score = sum of inlink hub scores)
	 * 		apply O		(hub score = sum of outlink auth scores)
	 * 		normalize xi
	 * 		normalize yi
	 */ 

	// declare iterators
	int i, k;

	// initialize all hub/auth scores to 1
	for (i = 0; i < size; i++)
	{
		urls[i]->hubScore = 1;
		urls[i]->authScore = 1;
	}

	// repeat for many iterations
	for (k = 0; k < iterations; k++)
	{
		double auth_norm = 0;
		double hub_norm = 0;
		
		// I Operaton
		for (i = 0; i < size; i++)
		{
			// Computer auth scores from inlinks
			urls[i]->authScore = 0;
			lnode *current_node = urls[i]->inlinks.front;
			while (current_node)
			{
				// increment auth score by inlink's auth score
				urlinfo *current = (urlinfo*)current_node->data;
				urls[i]->authScore += current->hubScore;
				current_node = current_node->next;
			}
			// update auth normalizer
			auth_norm += urls[i]->authScore * urls[i]->authScore;
		}
		
		// O Operation
		for (i = 0; i < size; i++)
		{
			// compute hub score from outlinks
			urls[i]->hubScore = 0;
			lnode *current_node = urls[i]->outlinks.front;
			while (current_node)
			{
				// increment hub score by outlink's auth score
				urlinfo *current = (urlinfo*)current_node->data;
				urls[i]->hubScore += current->authScore;
				current_node = current_node->next;
			}
			// update hub normalizer
			hub_norm += urls[i]->hubScore * urls[i]->hubScore;
		}
		
		// normalization = sqrt(sum of squares)
		auth_norm = sqrt(auth_norm);
		hub_norm = sqrt(hub_norm);
	
		// normalize scores
		for (i = 0; i < size; i++)
		{
			//printf("AUTH:\t%lf\t/\t%lf\n", urls[i]->authScore, auth_norm);
			// divide scores by norm
			urls[i]->authScore /= auth_norm;
			urls[i]->hubScore /= hub_norm;
		}
	}
}
