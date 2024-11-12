#include "vector.h"

void swap(int *a, int *b){
  int temp = *a;
  *a = *b;
  *b = temp;
}


int partition(int *a, int l, int r){
  int pivot = a[(l+r)/2];
  int i = l;
  int j = r;
  while(i <= j){
    while(a[i] < pivot)
      ++i;
    while(a[j] > pivot)
      --j;
    if(i >= j)
      break;
    swap(&a[i++], &a[j--]);
  }
  return j;
}

void quick_sort(int *arr, int left, int right){
    if (left >= right) return;
    int q = partition(arr, left, right);
    quick_sort(arr, left, q);
    quick_sort(arr, q+1, right);
}