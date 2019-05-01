#include "headerFuncs.h"

int compare_datastruct(const void* s1, const void* s2, int index){
  struct data_struct  *p1 = (struct data_struct *) s1;
  struct data_struct  *p2 = (struct data_struct *) s2;
  return p1->xyz[index] > p2->xyz[index];
}

int compare_x(const void* s1, const void* s2){
  return compare_datastruct(s1, s2, 0);
}

int compare_y(const void* s1, const void* s2){
  return compare_datastruct(s1, s2, 1);
}

int compare_z(const void* s1, const void* s2){
  return compare_datastruct(s1, s2, 2);
}

int compare_longfloat(const void* s1, const void* s2){
  float *p1 = (float *) s1;
  float *p2 = (float *) s2;
  return *p1 > *p2;
}

void do_sort(struct data_struct *array, int num, int colIndex){
  
  if (colIndex == 0)
    qsort(array, num, sizeof(struct data_struct), compare_x);
  else if (colIndex == 1)
    qsort(array, num, sizeof(struct data_struct), compare_y);
  else if (colIndex == 2)
    qsort(array, num, sizeof(struct data_struct), compare_z);
  else{
    printf("colIndex is between 0 and 2\n");
    exit(0);
  }
}
