#include "headerFuncs.h"

struct data_struct * globalSort(void * varray, int *num, int colIndex, int *globalNum){
  struct data_struct* array  = (struct data_struct *) varray;
  int i;
  do_sort(array, *num, colIndex);
  
  // BALANCE
  int* allCounts = (int *) malloc(num_ranks*num_ranks*sizeof(int));
  getallCount(*num, colIndex, array, allCounts); 
  if (getallcountflag == 1)
      printf("GETALLCOUNT gid%03d\n", my_global_rank);
  //if (readflag == 1)
  //  printf("GETALLCOUNT gid%03d\n", my_global_rank); 
  int total_recv_counts;  
  struct data_struct *recv_array = AllToAllSend(array, &total_recv_counts, allCounts);
  if (alltoallflag == 1)
      printf("ALLTOALL gid%03d\n", my_global_rank);
  //printf("ALLTOALLSEND gid%03d\n", my_global_rank); 
  *num =total_recv_counts;
  do_sort(recv_array, *num, colIndex);
  *globalNum = 0;
  for (i=0; i<num_ranks*num_ranks;i++)
    *globalNum+=allCounts[i];
  free(allCounts);
  return recv_array;
}
