#include "headerFuncs.h"

struct data_struct *  AllToAllIsend(void * sends, int  *total_recv_counts, void * rankc){

  int* send_counts;
  int* send_displs;
  
  MPI_Request send_req[num_ranks];
  MPI_Status status[num_ranks], stat;
  int i;
  struct data_struct*  send_array  = (struct data_struct *) sends;
  int * rank_counts = (int *) rankc;

  send_counts = (int *) calloc(num_ranks, sizeof(int));
  send_displs = (int *) calloc(num_ranks, sizeof(int));
  
  
  send_counts[0] = rank_counts[num_ranks*my_rank];
  *total_recv_counts =  rank_counts[my_rank];
  for (i=1; i < num_ranks; i++){
    send_counts[i] = rank_counts[num_ranks*my_rank + i];
    send_displs[i] = send_displs[i-1] + send_counts[i-1];
    (*total_recv_counts) += rank_counts[i*num_ranks + my_rank];
  }

  struct data_struct *recv_array = (struct data_struct *) malloc((*total_recv_counts) * sizeof(struct data_struct));

  for(i=0; i < num_ranks; i++){
    MPI_Isend(&send_array[send_displs[i]], send_counts[i], array_type, i, i, MPI_COMM_WORLD, &send_req[i]);
  }
  int count, flag;
  int displs = 0;
  for (i=0; i < num_ranks; i++){
      flag = 0;
      while(!flag)
	MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status[i]);
      if (flag){
	MPI_Get_count(&status[i], array_type, &count);
	MPI_Recv(&recv_array[displs], count, array_type, status[i].MPI_SOURCE, status[i].MPI_TAG, MPI_COMM_WORLD, &stat);
	displs += count;
      }
  }
  MPI_Waitall(num_ranks-1, &send_req[0], &status[0]);
 
  
  return recv_array;
}
