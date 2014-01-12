#include "utils/ranksort.h"
#include "urlinfo.h"
#include <stdlib.h>
#include <stdio.h>

/***************************************************************
 * ranksorttest.c
 *
 * main method for testing functionallity of ranksort.c
 ***************************************************************/

int main()
{
    size_t numURLS = 10;
    urlinfo **urlArray = calloc(numURLS, sizeof(urlinfo *));
    int j = 0;
    for (j=0; j < numURLS; j++)
    {
        urlArray[j] = malloc(sizeof(urlinfo));
        char *temp = malloc(sizeof(2));
        
        sprintf(temp, "%c",'A' + j);
        urlArray[j]->filename = temp;
        urlArray[j]->authScore = (double)(rand() % 20 + 1);
    }
    
    rank_sort(urlArray, (int)numURLS);
    
    int k;
    for (k = (int)numURLS-1; k >= 0; k--)
		printf("url: %s, rank: %f\n", urlArray[k]->filename, urlArray[k]->authScore);
	free(urlArray);
}