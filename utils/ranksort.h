#ifndef RANK_SORT_H
#define RANK_SORT_H

#include "../urlinfo.h"

/***************************************************************
 * ranksort.h
 *
 * Sorts an array of pointers to urlinfo. Urlinfos are sorted
 * by their authority score. After rank_sort() is called 
 * urlArray[0] is min value and urlArray[urlSize-1] is max value.
 ***************************************************************/
void rank_sort(urlinfo **urlArray, int urlSize);
void rank_sort_hub(urlinfo **urlArray, int urlSize);

//Hidden functions in ranksort.c
/*
void rec_quick_sort(urlinfo **urlArray, int left, int right);
double median_of_3(urlinfo **urlArray, int left, int right);
void swap(urlinfo **urlArray, int index1, int index2);
int partition_it(urlinfo **urlArray, int left, int right, double pivot);
void insertion_sort(urlinfo **urlArray, int left, int right);
*/

#endif
