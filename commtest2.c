#include "headerFuncs.h"
void createCommCollections(int mymin, int mymax, struct commgroupcollection *acgrange){
  int i, mymid, aval = mymin;
  acgrange->this_num_ranks = mymax-mymin+1;
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

struct node * commTest(void *varray, int num, void *vnode, int colIndex){
  struct data_struct* array  = (struct data_struct *)varray;
  struct node *anode = (struct node *)vnode;
  
  int i,j,k, globalNum;
  if (num_ranks > 1){
    int * allCounts = (int *)malloc(onum_ranks*num_ranks*sizeof(int));\
    
    // BINARY SEARCH FOR MAX AND MIN
    int total_recv_counts;  
    struct data_struct *recv_array = AllToAllSend(array, &total_recv_counts, allCounts);
    *num =total_recv_counts;
    return splitRanks(array, num, anode, colIndex);
    
  }else{
    getMaxMin(array, num, -1, anode->max, anode->min); //arrayMin);
    anode->num_below = num;
    anode->center = array;
    
    return anode;
    
    
  }
}


int main(int argc, char* argv[]) {
  if (argc < 2){
    printf("NEED 1 args: number of data points\n");
    exit(0);
  }
  int targetSize = 20;
  if (argc > 2){
    targetSize = atoi(argv[2]);
  }
  timePrint = 0;
  int datapoints = atoi(argv[1]);
  int num;
  int i, j, k, l, colIndex = 0, mymid, timeIndex = 1, timeStops = 12 ;
  int beginflag=0,readflag=1, lhflag=1, gtreeflag=0, readtflag=0, getsizeflag=0;
  int ltreeflag=0,sendsizeflag=0,assigntargetflag=0, localcountflag=0,sumlocalcountflag=0;
  int endflag=0;
  maxminflag=largestdimflag=globalsortflag=1;
  getbucketsflag=getcountsflag=inAdjustLflag=afterAdjustLflag=1;
  float startTime[timeStops], endTime[timeStops], avgTime[timeStops];
  struct node headNode, *localHead, *buildLocalHead;
  struct Gnode Gtree, *currNode;
  MPI_Status mystat;
  
  if (beginflag == 1)
    printf("Begin gid%03d\n", my_global_rank);
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  global_num_ranks = num_ranks;
  my_global_rank = my_rank;
  if (timePrint == 1){
    //printf("%d,%d,%d",datapoints, 
    startTime[0] = timestamp();
  }
  j = MPI_Comm_dup(MPI_COMM_WORLD, &dup_comm_world); 
  MPI_Comm_group( dup_comm_world, &world_group );
  MPI_Comm_create( dup_comm_world, world_group, &MPI_LOCAL_COMM );
  if (my_rank == num_ranks-1){
    num = (int)datapoints/num_ranks + datapoints%num_ranks;
  }else{

    num = (int)datapoints/num_ranks;
  }
  
  //=============================== 
  // CREATE COMM COLLECTION
  //=============================== 
  if (timePrint == 1){
    startTime[timeIndex] = timestamp();    
  }
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
  
  if (timePrint == 1){
    endTime[timeIndex++] = timestamp();
    
  }

  struct data_struct* array  = (struct data_struct *) malloc(num * sizeof(struct data_struct));
  
  //=============================== 
  //READ
  //=============================== 
  if (timePrint == 1){
    startTime[timeIndex] = timestamp();    
  }
  create_array_datatype();
  readFromFileAllRead(datapoints, array );
  if (timePrint == 1){
    endTime[timeIndex++] = timestamp();
    
  }
  if (readflag == 1)
    printf("READPOINTS gid%03d\n", my_global_rank);
  //=============================== 
  // GET LOCAL HEAD
  //=============================== 
  if (timePrint == 1){
    startTime[timeIndex] = timestamp();    
  }
  localHead = buildTreeGlobal(array, num, &headNode, -1);
  array = localHead->center;
  num = localHead->num_below;
  if (timePrint == 1){
    endTime[timeIndex++] = timestamp();
    
  }
  if (lhflag == 1)
    printf("GETLOCALHEAD gid%03d\n", my_global_rank);

  MPI_Finalize();
  return 0;
  
}
