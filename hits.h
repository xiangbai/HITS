/**********************************************************************************************
 * Implementation of HITS algorithm by Jon Kleinberg (plus  helper function)
 * 	Based on pseudocode from pg. 9 of "Authoritative Sources in a Hyperlinked Environment"
 *
 * This could be updated to use Eigenvectors for increased performance/accuracy
 *********************************************************************************************/

#include "urlinfo.h"

/*
 * Compute hub and authority scores for each url.
 * Assumes inlinks and outlinks of each url only point to other urls in the array.
 * Currently does algorithm using multiple iterations of a loop, rather than computing eigenvector
 *
 * *urls[] - Array of urlinfos
 * size - size of the array
 * iterations - number of iterations (10 was more than enough with a small sample size)
 */ 
void compute_hub_and_auth(urlinfo *urls[], int size, int iterations);

/* 
 * Add inlinks to urlinfos based on their outlinks.
 * Assumes inlinks of urls point to other urlinfos in urls.
 * 
 * *urls[] - Array of urlinfos
 * size	- size of the array
 */ 
void link_inlinks(urlinfo *urls[], int size);
