#include "../urlinfo.h"

void rank_sort(urlinfo **urlArray, int urlSize);
void rec_quick_sort(urlinfo **urlArray, int left, int right);
double median_of_3(urlinfo **urlArray, int left, int right);
void swap(urlinfo **urlArray, int index1, int index2);
int partition_it(urlinfo **urlArray, int left, int right, double pivot);
void insertion_sort(urlinfo **urlArray, int left, int right);