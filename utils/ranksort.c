#include <stdio.h>
#include <stdlib.h>
#include "ranksort.h"

void rec_quick_sort(urlinfo **urlArray, int left, int right);
double median_of_3(urlinfo **urlArray, int left, int right);
void swap(urlinfo **urlArray, int index1, int index2);
int partition_it(urlinfo **urlArray, int left, int right, double pivot);
void insertion_sort(urlinfo **urlArray, int left, int right);

/*
 * Public method for quicksort on array of urlinfo pointers
 */
void rank_sort(urlinfo **urlArray, int urlSize)
{
    rec_quick_sort(urlArray, 0, urlSize-1);
}

/*
 * Private helper method for rank_sort, the recursive call
 */
void rec_quick_sort(urlinfo **urlArray, int left, int right)
{
    int size = right-left+1;
    if (size < 10)
        insertion_sort(urlArray, left, right);
    else
    {
        double median = median_of_3(urlArray, left, right);
        int partition = partition_it(urlArray, left, right, median);
        rec_quick_sort(urlArray, left, partition-1);
        rec_quick_sort(urlArray, partition-1, right);
    }
}

/*
 * Private helper method for rank_sort, returns the pivot 
 * value for partitioning by taking the median of the first,
 * middle, and last elements of a sub-array.
 */
double median_of_3(urlinfo **urlArray, int left, int right)
{
    int center = (left+right)/2;
    
    if (urlArray[left]->authScore > urlArray[center]->authScore)
        swap(urlArray, left, center);
    
    if (urlArray[left]->authScore > urlArray[right]->authScore)
        swap(urlArray, left, center);
    
    if (urlArray[center]->authScore > urlArray[right]->authScore)
        swap(urlArray, center, right);
    
    swap(urlArray, center, right-1);
    return urlArray[right-1]->authScore;
}

/*
 * Swaps the indices
 */
void swap(urlinfo **urlArray, int index1, int index2)
{
    urlinfo *temp = urlArray[index1];
    urlArray[index1] = urlArray[index2];
    urlArray[index2] = temp;
}

/* Partitions the portion of the array so that all elements to the right of
 * the pivot are greater than the pivot and all elements to the left of the
 * pivot are less than the pivot. Returns the index of the pivot.
 */
 int partition_it(urlinfo **urlArray, int left, int right, double pivot)
{
    int leftPtr = left;
    int rightPtr = right - 1;
    while(1)
    {
        while(urlArray[++leftPtr]->authScore < pivot); //nop, find bigger
        while(urlArray[--rightPtr]->authScore > pivot); //nop. find smaller
        
        if (leftPtr >= rightPtr)
            break;
        else
            swap(urlArray, leftPtr, rightPtr);
    }
    swap(urlArray, leftPtr, right-1);
    return leftPtr;
}

/*
 * Insertion sort to handle small sub-arrays.
 */
void insertion_sort(urlinfo **urlArray, int left, int right)
{
    int in, out;
    
    for (out = left +1; out <= right; out++)
    {
        urlinfo *temp = urlArray[out];
        in = out;
        
        while (in > left && urlArray[in-1]->authScore >= temp->authScore)
        {
            urlArray[in] = urlArray[in-1];
            --in;
        }
        urlArray[in] = temp;
    }
}
