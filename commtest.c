#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "mpi.h"
MPI_Comm MPI_LOCAL_COMM, MPI_TEMP_COMM, dup_comm_world;
MPI_Group world_group, local_group;
int num_ranks, global_num_ranks;
int my_rank, my_global_rank, numRanges, maxLevel;
struct commgrouprange * myCommGroupRanges;
struct commworlds * myComms;
struct commgroupcollection * myCommCollection, * tempCollection;
struct commgrouprange{
  int this_num_ranks;
  int ranges[1][3];// = {0,0,0};
  struct commgrouprange * prev;
  struct commgrouprange * next;

};

struct commgroupcollection{
  int this_num_ranks;
  int *ranks;// = {0,0,0};
  struct commgroupcollection * prev;
  struct commgroupcollection * next;

};

struct commworlds{
  int this_num_ranks;
  MPI_Group localgroup;
  MPI_Comm localcomm;
  struct commworlds * prev;
  struct commworlds * next;

};

struct data_struct{
  long int num;
  float xyz[3];
};


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
  for(i=0; i < num_ranks; i++){ // Send and receive for each rank                                                                                                                              
    if (my_rank ==  i){  // Send sequentially for each rank                                                                                                                                    
      for (j=0; j < num_ranks; j++){
        if (j != my_rank){    // Send to others except me                                                                                                                                      
          if (send_counts[i] > 0) // Send only if send_counts  has something                                                                                                                   
            MPI_Send(send_array, send_counts[i], array_type,  myCommCollection->ranks[j], j, MPI_COMM_WORLD);
        }else{
          for(k =0; k < send_counts[i]; k++){
            recv_array[send_displs[i] + k] = send_array[k];
          }
        }
      }
    }else{
      if (send_counts[i] > 0)
        MPI_Recv(recv_array[send_displs[i]], send_counts[i], array_type,  myCommCollection->ranks[i], my_rank, MPI_COMM_WORLD, &stat);
    }

  }
  free(send_array);
  return recv_array;
}


//void splitRanks(){
//  MPI_Barrier(MPI_LOCAL_COMM);
//  MPI_Comm_free(&MPI_LOCAL_COMM);
//  if (num_ranks != global_num_ranks){
//    MPI_Group_free( &local_group );
//  }
//  MPI_Group_range_incl( world_group, 1, myCommGroupRanges->ranges, &local_group );
//  MPI_Comm_create( dup_comm_world, local_group, &MPI_LOCAL_COMM );
//  myCommGroupRanges = myCommGroupRanges->next;
//  free(myCommGroupRanges->prev);
//  
//  if (my_rank < num_ranks/2){
//    MPI_Comm_size(MPI_LOCAL_COMM, &num_ranks);
//    //printf("HELLO\n");
//    MPI_Comm_rank(MPI_LOCAL_COMM, &my_rank);
//  }else{
//    MPI_Comm_size(MPI_LOCAL_COMM, &num_ranks);
//      //printf("HELLO\n");
//    MPI_Comm_rank(MPI_LOCAL_COMM, &my_rank);
//  }
//
//}


//void splitRanks(){
//  if (global_num_ranks != num_ranks){
//    MPI_Comm_free(&MPI_TEMP_COMM);
//  }
//  
//  if (my_rank < num_ranks/2){
//    MPI_Comm_split(MPI_LOCAL_COMM,
//		   0,
//		   my_rank,
//		   &MPI_TEMP_COMM);
//    MPI_Comm_free(&MPI_LOCAL_COMM);
//    //MPI_LOCAL_COMM = MPI_TEMP_COMM;
//    //NEW
//    MPI_Group_free( &world_group);
//    MPI_Comm_group( MPI_TEMP_COMM, &world_group );
//    MPI_Comm_create( MPI_TEMP_COMM, world_group, &MPI_LOCAL_COMM );
//    //NEW
//    MPI_Comm_size(MPI_LOCAL_COMM, &num_ranks);
//    //printf("HELLO\n");
//    MPI_Comm_rank(MPI_LOCAL_COMM, &my_rank);
//    //myFunc();
//    
//  }else{
//    MPI_Comm_split(MPI_LOCAL_COMM,
//		   1,
//		   my_rank,
//		   &MPI_TEMP_COMM);
//    MPI_Comm_free(&MPI_LOCAL_COMM); 
//    //NEW
//    MPI_Group_free( &world_group);
//    MPI_Comm_group( MPI_TEMP_COMM, &world_group );
//    MPI_Comm_create( MPI_TEMP_COMM, world_group, &MPI_LOCAL_COMM );
//    //NEW
//    MPI_Comm_size(MPI_LOCAL_COMM, &num_ranks);
//    //printf("HELLO\n");
//    MPI_Comm_rank(MPI_LOCAL_COMM, &my_rank);
//    //myFunc();
//    
//  }
//}

void splitRanks(){
  int j;
  
  if (my_rank < num_ranks/2){
    MPI_Comm_split(MPI_LOCAL_COMM,
    		     0,
    		     my_rank,
    		     &MPI_TEMP_COMM);
    MPI_Comm_free(&MPI_LOCAL_COMM);
    //MPI_LOCAL_COMM = MPI_TEMP_COMM;
    j = MPI_Comm_dup(MPI_TEMP_COMM, &MPI_LOCAL_COMM);
    MPI_Comm_free(&MPI_TEMP_COMM);
    MPI_Comm_size(MPI_LOCAL_COMM, &num_ranks);
    //printf("HELLO\n");
    MPI_Comm_rank(MPI_LOCAL_COMM, &my_rank);
    
  }else{
    MPI_Comm_split(MPI_LOCAL_COMM,
    		     1,
    		     my_rank,
    		     &MPI_TEMP_COMM);
    MPI_Comm_free(&MPI_LOCAL_COMM);
    //MPI_LOCAL_COMM = MPI_TEMP_COMM;
    j = MPI_Comm_dup(MPI_TEMP_COMM, &MPI_LOCAL_COMM);
    MPI_Comm_free(&MPI_TEMP_COMM);
    MPI_Comm_size(MPI_LOCAL_COMM, &num_ranks);
    //printf("HELLO\n");
    MPI_Comm_rank(MPI_LOCAL_COMM, &my_rank);
    //myFunc();
    
  }
}


void createCommGroups(int mymin, int mymax, struct commgrouprange *acgrange){
  int i, mymid;
  acgrange->this_num_ranks = mymax-mymin+1;
  //printf("num_ranks: %d, my_global_rank: %d, my_rank: %d\n", acgrange->this_num_ranks, my_global_rank, my_rank);
  acgrange->ranges[0][0] = mymin;
  acgrange->ranges[0][1] = mymax;
  acgrange->ranges[0][2] = 1;
  acgrange->next = (struct commgrouprange *)malloc(sizeof( struct commgrouprange));
  acgrange->next->prev = acgrange;
  if (acgrange->this_num_ranks > 1){    
    numRanges += 1;
    
    mymid = mymin + (int)acgrange->this_num_ranks/2;
    if (my_global_rank < mymid){
      createCommGroups(mymin, mymid-1, acgrange->next);
      
    }else{
      createCommGroups(mymid, mymax, acgrange->next);
    }
  }
  

}

void createCommCollections(int mymin, int mymax, struct commgroupcollection *acgrange){
  int i, mymid, aval = mymin;
  acgrange->this_num_ranks = mymax-mymin+1;
  //printf("num_ranks: %d, my_global_rank: %d, my_rank: %d\n", acgrange->this_num_ranks, my_global_rank, my_rank);
  acgrange->ranks = (int *)malloc(acgrange->this_num_ranks*sizeof(int));
  for (i=0;i<acgrange->this_num_ranks;i++){//al=mymin;aval<=mymax;aval++){
    acgrange->ranks[i] = aval;
    aval++;
  }
  if (acgrange->this_num_ranks > 1){    
    numRanges += 1;
    acgrange->next = (struct commgroupcollection *)malloc(sizeof( struct commgroupcollection));
    acgrange->next->prev = acgrange;
    mymid = mymin + (int)acgrange->this_num_ranks/2;
    if (my_global_rank < mymid){
      createCommCollections(mymin, mymid-1, acgrange->next);
      
    }else{
      createCommCollections(mymid, mymax, acgrange->next);
    }
  }
  

}

void createCommWorlds(int mymin, int mymax, struct commworlds *acgrange){
  int i, mymid;
  int ranges[1][3];// = {0,0,0};
  
  acgrange->this_num_ranks = mymax-mymin+1;
  char fname[80] = "/home/gst2d/COMS7900/commoutput.txt";
  FILE * afile=fopen(fname,"a");
  //printf("num_ranks: %d, my_global_rank: %d, my_rank: %d\n", acgrange->this_num_ranks, my_global_rank, my_rank);
  fclose(afile);
  ranges[0][0] = mymin;
  ranges[0][1] = mymax;
  ranges[0][2] = 1;
  
  if (acgrange->this_num_ranks > 1){    
    MPI_Group_range_incl( world_group, 1, ranges, &acgrange->localgroup );
    MPI_Comm_create( dup_comm_world, acgrange->localgroup, &acgrange->localcomm );
    MPI_Comm_size(acgrange->localcomm, &num_ranks);
    MPI_Comm_rank(acgrange->localcomm, &my_rank);
    numRanges += 1;
    acgrange->next = (struct commworlds *)malloc(sizeof( struct commworlds));
    acgrange->next->prev = acgrange;
    mymid = mymin + (int)acgrange->this_num_ranks/2;
    if (my_global_rank < mymid){
      createCommWorlds(mymin, mymid-1, acgrange->next);
      
    }else{
      createCommWorlds(mymid, mymax, acgrange->next);
    }
  }
  

}


//void createCommLevel(struct commgroupcollection * cgc, int assigned){
//  int allAssigned,i,j;
//  int * whosAssigned = (int *)malloc(global_num_ranks*sizeof(int));
//  MPI_Allreduce(&assigned, &allAssigned, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD); 
//  if (allAssigned < global_num_ranks){
//    MPI_Allgather(&assigned, 1, MPI_INT, whosAssigned, 1, MPI_INT, MPI_COMM_WORLD);
//    if (assigned == 0){
//      cgc->this_num_ranks = allAssigned;
//      cgc->ranks = (int *)malloc(allAssigned*sizeof(int));
//      j=0;
//      for (i=0;i<allAssigned;i++){
//	if (whosAssigned[i] == 0){
//	  cgc->ranks[j] = i;
//	  j++;
//	}
//      }
//    }
//    MPI_Group_incl( world_group, cgc->this_num_ranks, cgc->ranks, &cgc->localgroup );
//    MPI_Comm_create( dup_comm_world, cgc->localgroup, &cgc->localcomm );
//    if (assigned == 0){
//      //cgc->next = (struct commgroupcollection *)malloc(sizeof( struct commgroupcollection));
//      free(cgc->ranks);
//      MPI_Comm_free(&cgc->localcomm);
//      MPI_Group_free(&cgc->localgroup);
//    }
//  }else{
//    MPI_Group_incl( world_group, cgc->this_num_ranks, cgc->ranks, &cgc->localgroup );
//    MPI_Comm_create( dup_comm_world, cgc->localgroup, &cgc->localcomm );
//  }
//}

int main(int argc, char* argv[]) {
  int i, j, k, l, mymid, my_sum;
  MPI_Status mystat;
  
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  global_num_ranks = num_ranks;
  my_global_rank = my_rank;
  //j = MPI_Comm_dup(MPI_COMM_WORLD, &MPI_LOCAL_COMM);
  
  j = MPI_Comm_dup(MPI_COMM_WORLD, &dup_comm_world); //&MPI_LOCAL_COMM);
  MPI_Comm_group( dup_comm_world, &world_group );
  MPI_Comm_create( dup_comm_world, world_group, &MPI_LOCAL_COMM );
    
  MPI_Barrier(MPI_LOCAL_COMM);
  //myCommGroupRanges = (struct commgrouprange *)malloc(sizeof( struct commgrouprange));
  //numRanges = 1;
  //mymid = (int)global_num_ranks/2;
  //if (my_global_rank < mymid){
  //  createCommGroups(0, mymid-1, myCommGroupRanges);
  //}else{
  //  createCommGroups(mymid, global_num_ranks-1, myCommGroupRanges);
  //}
  //myComms = (struct commworlds *)malloc(sizeof( struct commworlds));
  //numRanges = 1;
  //mymid = (int)global_num_ranks/2;
  //if (my_global_rank < mymid){
  //  createCommWorlds(0, mymid-1, myComms);
  //}else{
  //  createCommWorlds(mymid, global_num_ranks-1, myComms);
  //}
  //char fname[80] = "/home/gst2d/COMS7900/commoutput.txt";
  //FILE * afile=fopen(fname,"a");
  //printf("my_global_rank: %d", my_global_rank);
  //fclose(afile);
  myCommCollection= (struct commgroupcollection *)malloc(sizeof( struct commgroupcollection));
  myCommCollection->this_num_ranks = global_num_ranks;
  myCommCollection->ranks = (int *)malloc(myCommCollection->this_num_ranks*sizeof(int));
  for (i=0;i<myCommCollection->this_num_ranks;i++)
    myCommCollection->ranks[i] = i;
  
  myCommCollection->next= (struct commgroupcollection *)malloc(sizeof( struct commgroupcollection));
  myCommCollection->next->prev = myCommCollection;
  numRanges = 1;
  mymid = (int)global_num_ranks/2;
  if (my_global_rank < mymid){
    createCommCollections(0, mymid-1, myCommCollection->next);
  }else{
    createCommCollections(mymid, global_num_ranks-1, myCommCollection->next);
  }
  //tempCollection = myCommCollection->next;
  //my_sum = 4;
  //MPI_Allreduce(&numRanges, &maxLevel, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

  //printf("num_ranks: %d, my_global_rank: %d, my_rank: %d, my_sum: %d\n", num_ranks, my_global_rank, my_rank, my_sum);
  //splitRanks();
  //printf("num_ranks: %d, my_global_rank: %d, my_rank: %d, my_sum: %d\n", num_ranks, my_global_rank, my_rank, my_sum);
  //splitRanks();
  //printf("num_ranks: %d, my_global_rank: %d, my_rank: %d, my_sum: %d\n", num_ranks, my_global_rank, my_rank, my_sum);
  //numRanges = 2;
  //for (i=1;i<maxLevel;i++){
  //  if (i > numRanges){
  //    createCommLevel(tempCollection, 0);
  //    //assigned = 0
  //  }else{
  //    createCommLevel(tempCollection, 1); 
  //    tempCollection = tempCollection->next;
  //  }
  //  
  //  
  //  MPI_Barrier(MPI_LOCAL_COMM);
  //  //MPI_Allreduce(&num_ranks, &my_sum, 1, MPI_INT, MPI_SUM, MPI_LOCAL_COMM);  
  //  
  //}

  
  //for (i=0;i<numRanges;i++){
  //  MPI_Comm_size(MPI_LOCAL_COMM, &num_ranks);
  //  MPI_Comm_rank(MPI_LOCAL_COMM, &my_rank);
  //  MPI_Allreduce(&num_ranks, &my_sum, 1, MPI_INT, MPI_SUM, MPI_LOCAL_COMM);  
  //  printf("i: %d; num_ranks: %d, my_global_rank: %d, my_rank: %d, my_sum: %d\n", i, num_ranks, my_global_rank, my_rank, my_sum);
  //  //printf("i: %d; this_num_ranks: %d\n", i, myCommCollection->this_num_ranks);
  //  myCommCollection=myCommCollection->next;
  //}
  
  struct data_struct * sends
  
  printf("AFTER LOOP\n");
  MPI_Finalize();
  return 0;
  
}

