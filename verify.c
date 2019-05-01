#include "headerFuncs.h"

void verify(float min, float max){
  int i, bool;
  float min_max[2] = {min, max};
  float * temp = (float *) malloc(2 * num_ranks * sizeof(float));
  MPI_Allgather(min_max, 2, MPI_FLOAT, temp, 2, MPI_FLOAT, MPI_COMM_WORLD);
 
  bool = 1;
  
  for (i = 1; i < num_ranks-1; i+=2){
    if (temp[i] > temp[i+1]){
      if (my_rank == 0)
	printf("Last element on node %d is >= first element on node %d\n", i/2, i/2 + 1);
      bool = 0;
    }
  }

 
}
