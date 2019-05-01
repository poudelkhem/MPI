#include "headerFuncs.h"

struct data_struct *  AllToAllSend(void * sends, int  *total_recv_counts, void * rankc){

  int* send_counts;
  int* recv_counts;
  int* send_displs;
  int* recv_displs;
  int i, j, k;
  struct data_struct*  send_array  = (struct data_struct *) sends;
  int * rank_counts = (int *) rankc;

  send_counts = (int *) calloc(num_ranks, sizeof(int));
  recv_counts = (int *) calloc(num_ranks, sizeof(int));
  recv_displs = (int *) calloc(num_ranks, sizeof(int));
  send_displs = (int *) calloc(num_ranks, sizeof(int));

  send_counts[0] = rank_counts[num_ranks*my_rank];
  *total_recv_counts =  recv_counts[0] = rank_counts[my_rank];
  for (i=1; i < num_ranks; i++){
    send_counts[i] = rank_counts[num_ranks*my_rank + i];
    send_displs[i] = send_displs[i-1] + send_counts[i-1];
    recv_counts[i] = rank_counts[i*num_ranks + my_rank];
    recv_displs[i] = recv_displs[i-1] + recv_counts[i-1];
    (*total_recv_counts) += recv_counts[i];
  }

  struct data_struct *recv_array = (struct data_struct *) malloc((*total_recv_counts) * sizeof(struct data_struct));
  MPI_Alltoallv(send_array, send_counts, send_displs, array_type, recv_array, recv_counts, recv_displs, array_type, MPI_LOCAL_COMM);
  //for(i=0; i < num_ranks; i++){ // Send and receive for each rank
  //  if (my_rank ==  i){  // Send sequentially for each rank
  //    for (j=0; j < num_ranks; j++){
  //      if (j != my_rank){    // Send to others except me
  //        if (send_counts[j] > 0) // Send only if send_counts  has something
  //	    MPI_Send(&send_array[send_displs[j]], send_counts[j], array_type, myCommCollection->ranks[j], j, MPI_COMM_WORLD);
  //      }else{
  //        for(k =0; k < send_counts[i]; k++){
  //          recv_array[send_displs[i] + k] = send_array[k];
  //        }
  //      }
  //    }
  //  }else{
  //    if (recv_counts[i] > 0)
  //      MPI_Recv(&recv_array[recv_displs[i]], recv_counts[i], array_type,  myCommCollection->ranks[i], my_rank, MPI_COMM_WORLD, &stat);
  //  }
  //
  //}
  
  MPI_Barrier(MPI_LOCAL_COMM);
  free(send_array);
  return recv_array;
}



void my_Bcast_ld(void *send, int count, int root)
{
  float *sends=(float*) send;
  if (my_rank== root) 
    {
      int i;
      for ( i = 0; i <num_ranks ; i++) 
	{
	  if (i != my_rank) 
	    {
	      MPI_Send(sends, count, ld_type, myCommCollection->ranks[i], 0, MPI_COMM_WORLD);
	    }
	}
    } 
  else 
    {    
      MPI_Recv(sends, count, ld_type, myCommCollection->ranks[root], 0, MPI_COMM_WORLD, &stat);
    }
}

void my_Bcast_int(void *send, int count, int root)
{
  int *sends=(int*) send;
  if (my_rank== root) 
    {
      int i;
      for ( i = 0; i <num_ranks ; i++) 
	{
	  if (i != my_rank) 
	    {
	      MPI_Send(sends, count, MPI_INT, myCommCollection->ranks[i], 0, MPI_COMM_WORLD);
	    }
	}
    } 
  else 
    {    
      MPI_Recv(sends, count, MPI_INT, myCommCollection->ranks[root], 0, MPI_COMM_WORLD, &stat);
    }
}


void AllgatherLD(void * send, void *recv, int num){

  float * send_array = (float *) send;
  float * recv_array = (float *) recv;

  int i, j, k;
  for (i=0; i < num_ranks; i++){
    if (my_rank != i){
      MPI_Send(send_array, num, ld_type, myCommCollection->ranks[i], i, MPI_COMM_WORLD);
    }else{
      for (j=0; j < num_ranks; j++){
        if (my_rank != j)
          MPI_Recv(&recv_array[j*num], num, ld_type,  myCommCollection->ranks[j], i, MPI_COMM_WORLD, &stat);
        else{
          for(k=0; k < num; k++)
            recv_array[j*num + k] = send_array[k];
        }
      }
    }
  }

}

void AllgatherINT(void * send, void *recv, int num){

  int * send_array = (int *) send;
  int * recv_array = (int *) recv;

  int i, j, k;
  for (i=0; i < num_ranks; i++){
    if (my_rank != i){
      MPI_Send(send_array, num, MPI_INT, myCommCollection->ranks[i], i, MPI_COMM_WORLD);
    }else{
      for (j=0; j < num_ranks; j++){
        if (my_rank != j)
          MPI_Recv(&recv_array[j*num], num, MPI_INT,  myCommCollection->ranks[j], i, MPI_COMM_WORLD, &stat);
        else{
          for(k=0; k < num; k++)
            recv_array[j*num + k] = send_array[k];
        }
      }
    }
  }

}

