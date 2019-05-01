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
  }else{
    acgrange->next = NULL;
  }
  

}

void createCommLevel(struct commgroupcollection * cgc, int assigned){
  int allAssigned,i,j;
  int * whosAssigned = (int *)malloc(global_num_ranks*sizeof(int));
  MPI_Allreduce(&assigned, &allAssigned, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD); 
  if (allAssigned < global_num_ranks){
    MPI_Allgather(&assigned, 1, MPI_INT, whosAssigned, 1, MPI_INT, MPI_COMM_WORLD);
    if (assigned == 0){
      cgc->this_num_ranks = allAssigned;
      cgc->ranks = (int *)malloc(allAssigned*sizeof(int));
      j=0;
      for (i=0;i<allAssigned;i++){
	if (whosAssigned[i] == 0){
	  cgc->ranks[j] = i;
	  j++;
	}
      }
    }
    MPI_Group_incl( world_group, cgc->this_num_ranks, cgc->ranks, &cgc->localgroup );
    MPI_Comm_create( dup_comm_world, cgc->localgroup, &cgc->localcomm );
    if (assigned == 0){
      free(cgc->ranks);
      MPI_Comm_free(&cgc->localcomm);
      MPI_Group_free(&cgc->localgroup);
    }
  }else{
    MPI_Group_incl( world_group, cgc->this_num_ranks, cgc->ranks, &cgc->localgroup );
    MPI_Comm_create( dup_comm_world, cgc->localgroup, &cgc->localcomm );
  }
}

void deleteComms(){
  int i = 1;
  while (tempCollection->next != NULL && i<maxLevel) {
    tempCollection = tempCollection->next;
    //MPI_Comm_free(&tempCollection->prev->localcomm);
    //MPI_Group_free(&tempCollection->prev->localgroup);
    free(tempCollection->prev);
    i++;
  }
  //MPI_Comm_free(&tempCollection->localcomm);
  //MPI_Group_free(&tempCollection->localgroup);
  free(tempCollection);
}

void printLTree(struct node *anode, struct node **cnode){
  int caCount = 1,i=0, num = anode->num_below;
  char fname[20];
  sprintf(fname,"/home/gst2d/Final/nodes%03u.txt", my_global_rank);
  FILE *myfile = fopen(fname, "w");
  cnode[0] = anode;
  while (caCount > 0){
    anode = cnode[0];
    if (anode->num_below > 1){
      for (i=0;i<caCount;i++)
	cnode[i] = cnode[i+1];
      caCount++;
      if (caCount > num-1){
	printf("rank: %d CA COUNT TOO BIG", my_global_rank);
	return;
      }
      cnode[caCount-2] = anode->left;
      cnode[caCount-1] = anode->right;
      //fprintf(myfile,"caCount: %03d;\nMax: (%15f,%15f,%15f);\nMin: (%15f,%15f,%15f);\nmaxRadius: %15f;\nnum_below: %15u\n========\n",
      //	  caCount,
      //	  anode->max[0],anode->max[1],anode->max[2],
      //	  anode->min[0],anode->min[1],anode->min[2],
      //	  anode->maxRadius,
      //	  anode->num_below);
       
    }else{
      for (i=0;i<caCount;i++)
	cnode[i] = cnode[i+1];
      caCount--;
      //fprintf(myfile,": %03d;\npointId: %:Ld;center: (%15f,%15f,%15f);\nnum_below: %15u\n========\n",
      //	      caCount,
      //	      anode->center->num,
      //	      anode->center->xyz[0],anode->center->xyz[1],anode->center->xyz[2],
      // 	      anode->num_below);
    }

  }
  fclose(myfile);
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
  timePrint = 1;
  char timeName[80] = "/home/gst2d/Final/time.txt";
  FILE *timeFile;
  int datapoints = atoi(argv[1]);
  int num;
  int i, j, k, l, colIndex = 0, mymid, timeIndex = 1, timeStops = 11 ;
  int beginflag=0,readflag=0, lhflag=0, gtreeflag=0, readtflag=0, getsizeflag=0;
  int ltreeflag=0,sendsizeflag=0,assigntargetflag=0, localcountflag=0,sumlocalcountflag=0;
  int endflag=0;
  maxminflag=largestdimflag=globalsortflag=splitranksflag=0;
  getallcountflag=alltoallflag=0;
  getbucketsflag=getcountsflag=inAdjustLflag=afterAdjustLflag=Bcastflag=0;
  double startTime[timeStops], endTime[timeStops], avgTime[timeStops], dummyTime[timeStops];
  struct node headNode, *localHead, *buildLocalHead, *tnode;
  struct Gnode Gtree, *currNode;
  MPI_Status mystat;
  
  if (beginflag == 1)
    printf("Begin gid%03d\n", my_global_rank);
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  global_num_ranks = num_ranks;
  my_global_rank = my_rank;
  if ((int)datapoints/num_ranks < global_num_ranks){
    printf("TOO FEW DATAPOINTS\n");
    MPI_Finalize();
    return 0;
  }
  
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
  //if (timePrint == 1){
  //  startTime[timeIndex] = timestamp();    
  //  
  //}
  //myCommCollection= (struct commgroupcollection *)malloc(sizeof( struct commgroupcollection));
  //MPI_Comm_group( dup_comm_world, &myCommCollection->localgroup );
  //MPI_Comm_create( dup_comm_world, myCommCollection->localgroup, &MPI_LOCAL_COMM );
  //myCommCollection->this_num_ranks = global_num_ranks;
  //myCommCollection->ranks = (int *)malloc(myCommCollection->this_num_ranks*sizeof(int));
  //for (i=0;i<myCommCollection->this_num_ranks;i++)
  //  myCommCollection->ranks[i] = i;
  //
  //myCommCollection->next= (struct commgroupcollection *)malloc(sizeof( struct commgroupcollection));
  //myCommCollection->next->prev = myCommCollection;
  //numRanges = 1;
  //mymid = (int)global_num_ranks/2;
  //if (my_global_rank < mymid){
  //  createCommCollections(0, mymid-1, myCommCollection->next);
  //}else{
  //  createCommCollections(mymid, global_num_ranks-1, myCommCollection->next);
  //}
  //
  //tempCollection = myCommCollection->next;
  //
  //MPI_Allreduce(&numRanges, &maxLevel, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

  //for (i=1;i<maxLevel;i++){
  //  if (i > numRanges){
  //    createCommLevel(tempCollection, 0);
  //  }else{
  //    createCommLevel(tempCollection, 1);      
  //    tempCollection = tempCollection->next;
  //  }
  //  MPI_Barrier(MPI_LOCAL_COMM);
  //}

  //tempCollection = myCommCollection;
  //if (timePrint == 1){
  //  endTime[timeIndex++] = timestamp();
  //  dummyTime[timeIndex-1] = endTime[timeIndex-1] - startTime[timeIndex-1];    
  //  
  //  MPI_Reduce( &dummyTime[timeIndex-1], &avgTime[timeIndex-1], 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD ); 
  //  if (my_global_rank == 0){
  //    //timeFile = fopen(timeName, "a");
  //    printf("%f", avgTime[timeIndex-1]);      
  //    //fclose(timeFile);
  //  }
  //}
  
  //=============================== 
  //READ
  //=============================== 
  if (timePrint == 1){
    startTime[timeIndex] = timestamp();    
  }

  struct data_struct* array  = (struct data_struct *) malloc(num * sizeof(struct data_struct));
  create_array_datatype();
  readFromFileAllRead(datapoints, array );
  
  //printFile(num, array);
  
  if (timePrint == 1){
    endTime[timeIndex++] = timestamp();
    dummyTime[timeIndex-1] = endTime[timeIndex-1] - startTime[timeIndex-1];
    //printf("startTime: %f endTime: %f diffTime: %f\n", startTime[timeIndex-1],endTime[timeIndex-1], dummyTime[timeIndex-1]);    
    MPI_Reduce( &dummyTime[timeIndex-1], &avgTime[timeIndex-1], 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD ); 
    if (my_global_rank == 0){
      //timefile = fopen(timeName, "a");
      printf(";%f", avgTime[timeIndex-1]);
      //printf("READ: %f\n", avgTime[timeIndex-1]);
      //fclose(timeFile);
    }
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
    dummyTime[timeIndex-1] = endTime[timeIndex-1] - startTime[timeIndex-1];
    MPI_Reduce( &dummyTime[timeIndex-1], &avgTime[timeIndex-1], 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD ); 
    if (my_global_rank == 0){
      //timefile = fopen(timeName, "a");
      printf(";%f", avgTime[timeIndex-1]);
      //printf("GET LOCAL HEAD %f\n", avgTime[timeIndex-1]);
      //fclose(timeFile);
    }
  }
  if (lhflag == 1)
    printf("GETLOCALHEAD gid%03d\n", my_global_rank);
  

  //deleteLocalHeadBuild(&headNode);
  

  MPI_Barrier(MPI_COMM_WORLD);
  //MPI_Comm_free(&tempCollection->localcomm);
  //MPI_Group_free(&tempCollection->localgroup);
  //deleteComms();

  //printf("myrank: %d; num: %d\n", my_global_rank, num);
  MPI_Allreduce(&num,&k,1,MPI_INT,MPI_MIN,MPI_COMM_WORLD);
  if ( k < 4){
    printf("A RANK HAS < 4\n");
    MPI_Finalize();
    return 0;
  }
  
  //=============================== 
  //BUILD GLOBAL TREE ON RANK ZERO
  //=============================== 
  if (timePrint == 1){
    startTime[timeIndex] = timestamp();    
  }
  globalTreeMaster(&Gtree, localHead);
  if (timePrint == 1){
    endTime[timeIndex++] = timestamp();
    dummyTime[timeIndex-1] = endTime[timeIndex-1] - startTime[timeIndex-1];
    MPI_Reduce( &dummyTime[timeIndex-1], &avgTime[timeIndex-1], 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD ); 
    if (my_global_rank == 0){
      printf(";%f", avgTime[timeIndex-1]);
      //printf("BUILDGLOBALTREE %f\n", avgTime[timeIndex-1]);
    }
  }
  if (gtreeflag == 1)
    printf("BUILDGLOBALTREE gid%03d\n", my_global_rank);
  
  //========================
  //
  //                                           TARGETS SECTION
  //
  //========================
  
  char fname[80] = "/home/gst2d/localstorage/public/coms7900-data/binary/bdatafile00501.bin";
  
  struct data_struct* targetArray  = (struct data_struct *) malloc(targetSize * sizeof(struct data_struct )); 
  int * sendSize = (int *)calloc(global_num_ranks,sizeof(int));
  int mySendSize = 0, maxSendSize;
  int totalSendSize = 0;
  struct data_struct* sendArray;//  =
  //struct data_struct* allSendArrays[global_num_ranks];
  //struct data_struct ** allSendArrays = (struct data_struct **) malloc(global_num_ranks * sizeof(struct data_struct *)); 
  if (timePrint == 1){
      startTime[timeIndex] = timestamp();    
    }
  
  if (my_global_rank == 0){
    
    readFromFile(fname, targetSize, targetArray );
    
    if (readtflag == 1)
      printf("READTARGETS gid%03d\n", my_global_rank);
  }
  if (timePrint == 1){
    endTime[timeIndex++] = timestamp();
    dummyTime[timeIndex-1] = endTime[timeIndex-1] - startTime[timeIndex-1];
    
    if (my_global_rank == 0){
      avgTime[timeIndex-1] = dummyTime[timeIndex-1];
      //timefile = fopen(timeName, "a");
      printf(";%f", avgTime[timeIndex-1]);
      //printf("READTARGETS %f\n", avgTime[timeIndex-1]);
      //fclose(timeFile);
    }
  }
  
  //========================
  //   GET NUM OF TARGETS FOR EACH RANK
  //========================
  if (timePrint == 1){
    startTime[timeIndex] = timestamp();    
  }
  //if (my_global_rank == 0){
  //  getSendSize1(&Gtree, 0.1, targetArray, targetSize, sendSize);
  //  maxSendSize = sendSize[0];
  //  for (i=0;i<global_num_ranks;i++){
  //    totalSendSize += sendSize[i];
  //    if (maxSendSize < sendSize[i])
  //	maxSendSize = sendSize[i];
  //    //printf("sendsize[%d]: %d\n", i, sendSize[i]);
  //  }
  //  
  //}
  
  //printf("my_rank: %d; num: %d\n", my_global_rank, num);
  
  if (timePrint == 1){
    endTime[timeIndex++] = timestamp();
    dummyTime[timeIndex-1] = endTime[timeIndex-1] - startTime[timeIndex-1];
    
    if (my_global_rank == 0){
      avgTime[timeIndex-1] = dummyTime[timeIndex-1];
      //timefile = fopen(timeName, "a");
      printf(";%f", avgTime[timeIndex-1]);
      //printf("GETNUMTARGETS %f\n", avgTime[timeIndex-1]);
      //fclose(timeFile);
    }
  }
  if (getsizeflag == 1)
    printf("GETSIZE gid%03d\n", my_global_rank);
  MPI_Barrier(MPI_COMM_WORLD);
  
  //========================
  //   LOCAL TREE BUILD 
  //========================
  if (timePrint == 1){
    startTime[timeIndex] = timestamp();    
  }
  //reduceLocalHead()
  buildTree(array, num, localHead, -1);
  if (timePrint == 1){
    endTime[timeIndex++] = timestamp();
    dummyTime[timeIndex-1] = endTime[timeIndex-1] - startTime[timeIndex-1];
    MPI_Reduce( &dummyTime[timeIndex-1], &avgTime[timeIndex-1], 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD ); 
    if (my_global_rank == 0){
      
      //timefile = fopen(timeName, "a");
      printf(";%f", avgTime[timeIndex-1]);
      //printf("LOCALTREEBUILD %f\n", avgTime[timeIndex-1]);
      //fclose(timeFile);
    }
  }
  if (ltreeflag == 1)
    printf("LOCALTREEBUILD gid%03d\n", my_global_rank);
  
  
  //========================
  //   RANK 0 SENDS NUM OF TARGETS TO OTHER RANKS
  //========================
  if (timePrint == 1){
    startTime[timeIndex] = timestamp();    
  }
  //MPI_Scatter(sendSize, 1, MPI_INT, &mySendSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if (timePrint == 1){
    endTime[timeIndex++] = timestamp();
    dummyTime[timeIndex-1] = endTime[timeIndex-1] - startTime[timeIndex-1];
    MPI_Reduce( &dummyTime[timeIndex-1], &avgTime[timeIndex-1], 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD ); 
    if (my_global_rank == 0){
      //timefile = fopen(timeName, "a");
      printf(";%f", avgTime[timeIndex-1]);
      //printf("SENDSIZE %f\n", avgTime[timeIndex-1]);
      //fclose(timeFile);
    }
  }



  if (sendsizeflag == 1)
    printf("SENDSIZE gid%03d\n", my_global_rank);
  MPI_Barrier(MPI_COMM_WORLD);
  
  //========================
  //   RANK 0 ASSIGNS TARGETS TO OTHER RANKS
  //========================
  if (timePrint == 1){
    startTime[timeIndex] = timestamp();    
  }
  if (my_global_rank == 0){
    //for (i = 0;i<global_num_ranks;i++){
    //  if (sendSize[i] > 0)
    //	allSendArrays[i] = (struct data_struct *) malloc(sendSize[i] * sizeof(struct data_struct)); 
    //}
    
    sendArray = (struct data_struct *) malloc(targetSize * sizeof(struct data_struct)); 
    for (i = 1;i<global_num_ranks;i++){
      //if (sendSize[i] > 0){
	//getSendArray(&Gtree, 0.1, targetArray, targetSize, allSendArrays[i], sendSize[i], i);
	//MPI_Send(allSendArrays[i], sendSize[i], array_type, i, 0, MPI_COMM_WORLD);
	//sendArray = (struct data_struct *) malloc(sendSize[i] * sizeof(struct data_struct)); 
	getSendArray(&Gtree, 0.1, targetArray, targetSize, sendArray, &sendSize[i], i);
	MPI_Send(sendArray, sendSize[i], array_type, i, 0, MPI_COMM_WORLD);
	//MPI_Recv(&j, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &mystat);
	//free(sendArray);
	// }
    }
    if (mySendSize > 0){
      //sendArray = (struct data_struct *) malloc(mySendSize * sizeof(struct data_struct));
      
      getSendArray(&Gtree, 0.1, targetArray, targetSize, sendArray, &sendSize[0], 0);
      //printFile(mySendSize, sendArray);
    }
    mySendSize = sendSize[0];    
    free(sendArray);
    
  }else{
    //if (mySendSize > 0){
    MPI_Probe(0, 0, MPI_COMM_WORLD, &stat);
    //d = stat.MPI_SOURCE;
    MPI_Get_count(&stat,array_type,&mySendSize);
    sendArray = (struct data_struct *) malloc(mySendSize * sizeof(struct data_struct)); 
    //printFile(mySendSize, sendArray);
    MPI_Recv(sendArray, mySendSize, array_type, 0, 0, MPI_COMM_WORLD, &mystat);    
    //i = 1;
    //MPI_Send(&i,1,MPI_INT,0,0,MPI_COMM_WORLD);
    //}
  }
  
  
  if (timePrint == 1){
    endTime[timeIndex++] = timestamp();
    dummyTime[timeIndex-1] = endTime[timeIndex-1] - startTime[timeIndex-1];
    //printf("myRank: %f\n", dummyTime[timeIndex-1]);
    MPI_Reduce( &dummyTime[timeIndex-1], &avgTime[timeIndex-1], 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD ); 
    if (my_global_rank == 0){
      //timefile = fopen(timeName, "a");
      printf(";%f", avgTime[timeIndex-1]);
      //printf("ASSIGN TARGETS %f\n", avgTime[timeIndex-1]);
      //fclose(timeFile);
    }
  }
  
  if (assigntargetflag == 1)
    printf("ASSIGNTARGET gid%03d\n", my_global_rank);
  
  
  MPI_Barrier(MPI_COMM_WORLD);
  
  float radius[3] = {0.01, 0.05, 0.1};
  int localCount = 0, totalCount, tgi = 0, sendi = 0, radi = 0;
  long int *radiCounts = (long int *) malloc(4*mySendSize*sizeof(long int));
  long int *allRadiCounts;
  int *tsendSize, *counters;
  struct node ** childArray = (struct node **)malloc( localHead->num_below*sizeof(struct node *));
  counters = (int *)calloc(3,sizeof(int));
  
  //if (my_global_rank == 0)
  //  printf("radiCountsSize %d\tmySendSize %d\n\n\n\n", 4*mySendSize, mySendSize);
  
  //========================
  //   EACH RANK COUNT LOCALLY
  //========================
  //MPI_Allreduce(&mySendSize, &maxSendSize, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
  //char lfilename[80];
  //sprintf(lfilename,"/home/gst2d/Final/local%03u.txt", my_global_rank);
  //FILE *lfile = fopen(lfilename, "w");
  
  if (timePrint == 1){
    startTime[timeIndex] = timestamp();    
  }
  //printf("rank: %d, SENDSIZE: %d\n", my_global_rank, mySendSize);
  //fprintf(lfile,"SENDSIZE: %d\n", mySendSize);
  
  for (sendi =0; sendi<mySendSize; sendi++){
    radi = sendi*4;
    radiCounts[radi++] = sendArray[sendi].num;
    //printf("rank: %d, sendi: %d; targetId: %Ld; \n", my_global_rank,sendi, sendArray[sendi].num);
    //fprintf(lfile, "sendi: %d; targetId: %Ld; ", sendi, sendArray[sendi].num);
    //if (sendi < mySendSize){
    localSearch(localHead, sendArray[sendi], childArray, &radiCounts[radi]);	
    
    
    //for (i=0;i<3;i++){
    //  //fprintf(lfile,"%f ", radius[i]);
    //  radiCounts[radi++] =counters[i];// localSearch(localHead, radius[i], sendArray[sendi], childArray);	
      
    //}
    //}
    //printf("rank: %d FIN%d\n", my_global_rank, sendi);
    //fprintf(lfile, "\n");
    //MPI_Barrier(MPI_COMM_WORLD);
    //for (i=0;i<num;i++){
    //  if (sendArray[sendi].num == array[i].num){
    //	if (radiCounts[radi-3] == 0)
    //	  printf("myrank: %d; targetId: %Ld\n", my_global_rank, sendArray[sendi].num);
    //	break;
    //  }
    //}
    
  }
  free(childArray);
  free(counters);
  //fclose(lfile);

  if (timePrint == 1){
    endTime[timeIndex++] = timestamp();
    dummyTime[timeIndex-1] = endTime[timeIndex-1] - startTime[timeIndex-1];
    MPI_Reduce( &dummyTime[timeIndex-1], &avgTime[timeIndex-1], 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD ); 
    if (my_global_rank == 0){
      //timefile = fopen(timeName, "a");
      printf(";%f", avgTime[timeIndex-1]);
      //printf("LOCALCOUNT %f\n", avgTime[timeIndex-1]);
      //fclose(timeFile);
    }
  }
  
  if (localcountflag == 1)
    printf("LOCALCOUNT gid%03d\n", my_global_rank);
  
  MPI_Barrier(MPI_COMM_WORLD);
  
  if (timePrint == 1){
    startTime[timeIndex] = timestamp();    
  }

  //  printf("HELLO GUYS BEFORE \n");

  if (my_global_rank == 0){
    //MPI_Status stat;
    int d,gc, nonZeroRanks = 0;
    allRadiCounts = (long int *) calloc(3*targetSize,sizeof(long int));
    tsendSize = (int *)malloc(global_num_ranks*sizeof(int));
    tsendSize[0] = sendSize[0]*4;
    //========================
    //   GET SEND TOTAL SEND SIZE
    //========================
    for (i=1;i<global_num_ranks;i++){
      tsendSize[i] = sendSize[i]*4;
      if (tsendSize[i] > 0)
	nonZeroRanks++;
    }
    

    //========================
    //   SUM COUNTS FOR RANK 0
    //========================
    //printf("HELLO GUYS BEFORE\n");
    for (radi=0;radi<tsendSize[my_global_rank];radi+=4){
      k = (int)(radiCounts[radi]-targetArray[0].num)*3;
      //      printf("k %d\t radi %d\t tSendSize %d\tallRadicountsSize %d\tradiCounts %Lu\ttargetArray %Lu\n", k, radi, tsendSize[my_global_rank], 3*targetSize, radiCounts[radi], targetArray[0].num);
      for (j=1; j<=3;j++){
	allRadiCounts[k+j-1] += radiCounts[radi+j];	
      }
    }
    
    
      //========================
    //  SUM COUNTS FOR OTHER RANKS
    //========================
    i = 0;
    while (i<nonZeroRanks){ 

      MPI_Probe(MPI_ANY_SOURCE, 123, MPI_COMM_WORLD, &stat);
      d = stat.MPI_SOURCE;
      MPI_Get_count(&stat,li_type,&gc);
      
      radiCounts = (long int *)malloc(tsendSize[d]*sizeof(long int));      
      MPI_Recv(radiCounts,tsendSize[d] , li_type, d, 123, MPI_COMM_WORLD, &mystat);
      //printf("Num %d to %d\n", i, d);
      i++;
      for (radi=0;radi<tsendSize[d];radi+=4){      	
      	k = (int)(radiCounts[radi]-targetArray[0].num)*3;
      	for (j=1; j<=3;j++){
      	  allRadiCounts[k+j-1] += radiCounts[radi+j];      	  
      	}      	
      }    
      free(radiCounts);
    }
    
  }else{
    mySendSize *=4;
    if (mySendSize > 0)
      MPI_Send(radiCounts, mySendSize, li_type, 0, 123, MPI_COMM_WORLD);
    
  }
  MPI_Barrier(MPI_COMM_WORLD);
  
  
  if (timePrint == 1){
    endTime[timeIndex++] = timestamp();
    dummyTime[timeIndex-1] = endTime[timeIndex-1] - startTime[timeIndex-1];
    MPI_Reduce( &dummyTime[timeIndex-1], &avgTime[timeIndex-1], 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD ); 
    if (my_global_rank == 0){
      //timefile = fopen(timeName, "a");
      printf(";%f", avgTime[timeIndex-1]);
      //printf("GLOBALCOUNT %f\n", avgTime[timeIndex-1]);
      //fclose(timeFile);
    }
  }
  if (sumlocalcountflag == 1)
    printf("SUMCOUNTS gid%03d\n", my_global_rank);
  if (timePrint == 1){
    endTime[0] = timestamp();
    dummyTime[0] = endTime[0] - startTime[0];
    MPI_Reduce( &dummyTime[0], &avgTime[0], 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD ); 
    if (my_global_rank == 0){
      //timeFile = fopen(timeName, "a");
      printf(";%f\n", avgTime[0]);
      //printf("TOTALTIME %f\n", avgTime[0]);
      //fclose(timeFile);
    }
    
  }
  
  if (my_global_rank == -1){
    
    
    ////================
    ////
    //// PRINT ALL TARGETS
    ////
    ////================
    
    printf("%20s\t%10s\t%10s\t%10s\n", "TargetID", "0.01", "0.05","0.1");
    for (tgi =0; tgi<targetSize; tgi++){
      //printf("%20Lu\t%0.15f\t%0.15f\t%0.15f",targetArray[tgi].num, targetArray[tgi].xyz[0], targetArray[tgi].xyz[1], targetArray[tgi].xyz[2]);
      printf("%20Lu",targetArray[tgi].num);
      k = tgi*3;
      for (i=0;i<3;i++){
	printf("\t%10Lu",allRadiCounts[ k+i]);
      }
      printf("\n");      
    }
  
  }

  
   
  MPI_Finalize();
  return 0;
  
}
