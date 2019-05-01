#include "headerFuncs.h"

void getallCount(int num, const int colIndex, void* varray, void *vallCounts){
  struct data_struct  *array = (struct data_struct *) varray;
  int* allCounts = (int *)vallCounts;
  int balanced;
  float* nodeDivL = (float *) malloc((num_ranks+2)*sizeof(float));
  int* totalCount = (int *) malloc((num_ranks)*sizeof(int));
  float* LDivinfo = (float *) malloc(((num_ranks+2)*num_ranks)*sizeof(float));
  float* LDiv = (float *) malloc(((num_ranks)*num_ranks)*sizeof(float));
  float* L = (float *) malloc((num_ranks+1)*sizeof(float));
  float totalMax; // = LDivinfo[num_ranks];
  MPI_Status status;
  int total=0,K;
  int i, j=0, k=0, D;
  //============================
  //
  // Get nodeDivL (the "percentiles")
  // the min of N evenly split groups on each node
  //
  //============================
  D = (int)(num/num_ranks);
  j = 0;
  nodeDivL[num_ranks+1] = num;
  nodeDivL[num_ranks] = array[num-1].xyz[colIndex]; //array[0].x;
  for (i=0;i<num_ranks;i++)
    nodeDivL[i] = array[i*D].xyz[colIndex];
  
  
  //============================
  //
  // Get LDivinfo 
  // (all nodeDivLs + each nodes max + each nodes size)
  //
  //============================

  MPI_Allgather(nodeDivL, num_ranks+2,ld_type, LDivinfo,num_ranks+2,ld_type, MPI_LOCAL_COMM); 
  //AllgatherLD(nodeDivL, LDivinfo, num_ranks+2);
  
  //============================
  //
  // Get totalMax 
  // The max value across all nodes
  //
  //============================

       
  totalMax = LDivinfo[num_ranks];
  for (i=0;i<num_ranks;i++){
    if (totalMax < LDivinfo[i*(num_ranks+2) + num_ranks])
      totalMax = LDivinfo[i*(num_ranks+2) + num_ranks];

  }
  
  //============================
  //
  // Get LDiv 
  // (all nodeDivLs)
  //
  //============================

  k=0;
  for (i=0; ((num_ranks+2)*num_ranks); i++){
    if (i%(num_ranks+2) == 0){
      for (j=i;j<i+num_ranks; j++){
	LDiv[k] = LDivinfo[j];
	k++;
	if (k >= num_ranks*num_ranks)
	  break;
      }
    }
    if (k >= num_ranks*num_ranks)
      break;
  }

  //============================
  //
  // Get L 
  // (lower Bound of each + totalMax)
  //
  //============================

  qsort(LDiv, (num_ranks)*num_ranks, sizeof(float), compare_longfloat);

  for (i=0;i<num_ranks;i++){
    L[i] = LDiv[i + i*num_ranks];
  }
  L[num_ranks] = totalMax;


  //============================
  //
  // Get Counts
  // check if total count balanced
  //
  //============================
  if (getbucketsflag == 1)
    printf("GETBUCKETS gid%03d\n", my_global_rank);
  getCounts(num, colIndex, array, L, totalCount, allCounts);
  if (getcountsflag == 1)
    printf("GETCOUNTS gid%03d\n", my_global_rank);
  checkBalance(&balanced, totalCount);

  //============================
  //
  // Adjust L if not balanced
  //
  //============================
  
  adjustL(num, colIndex, array, L, allCounts, totalCount, &balanced);
  free(nodeDivL);
  free(totalCount);
  free(LDivinfo);
  free(LDiv);
  free(L);
  
}

//============================
//
// If not balanced, get Vars
//
//
// sDiffi int [num_ranks] totalCount[i]-K
// rangeLi float [num_ranks] L[i+1] - L[i-1] index 0 is blank
// totalRange float L[num_ranks] - L[0]
// smallest float min[array[i+1] - array[i]] > 0
// smallestDiff int min(sum(sDiffi), smallestDiff) for each iteration of adjustment
// smallestDiffL float [num_ranks] L at smallestDiff
// K = total/N
// balance => abs(Si-K) < .10*K
//
// adjustCount int [num_ranks] number of adjustments for each limit index 0 is blank
// prevsDiffi = sDiffi
// percentRange float [num_ranks] the percent of rangeLi for each node
//============================
void adjustL(int num,  const int colIndex, void* varray, void *vL, void *vallCounts, void *vtotalCount, void *vbalanced){
  struct data_struct  *array = (struct data_struct *) varray;
  float* L = (float *) vL;
  int* allCounts = (int *)vallCounts; 
  int* totalCount = (int *) vtotalCount; 
  int* balanced = (int *) vbalanced;
  if (*balanced != 0){
    return;
  }
  if (inAdjustLflag == 1)
    printf("INADJUSTL gid%03d\n", my_global_rank);
  int sDiff[num_ranks];
  int prevsDiff[num_ranks];
  float rangeL[num_ranks];
  float myrange;
  float totalRange = L[num_ranks] - L[0];
  float smallest;
  float mysmallest;
  float * allSmallest = (float *) malloc(num_ranks*sizeof(float));
  int smallestDiff=0;
  int smallestDiffCheck=0;
  float smallestDiffL[num_ranks];
  int smallestDiffBool=0;
  int adjustCount[num_ranks];
  float percentRange[num_ranks];

  MPI_Status status;
  int total=0,K;
  int i, j=0, k=0, D, startIndex = 1, totalIterations, zeroIterations, keepGoing = 0, myflags[3] ;
  //============================
  //
  // Get Required variables
  //
  //============================
  for (i=0; i<num_ranks*num_ranks;i++)
    total+=allCounts[i];
  K = total/num_ranks;
  
  for (i=0; i<num_ranks; i++){
    sDiff[i] = totalCount[i]-K;
    smallestDiff += sDiff[i];
    smallestDiffL[i] = L[i];
    adjustCount[i] = 0;
    percentRange[i] = 0.1;
    if (i > 0 )
      rangeL[i] = L[i+1] - L[i-1];
  }
  mysmallest = 0;
  //============================
  //
  // Get smallest diff between vals
  //
  //============================
  
  mysmallest = array[num-1].xyz[colIndex] - array[0].xyz[colIndex];
  for (i=0;i<num-1; i++){
    smallest = array[i+1].xyz[colIndex] - array[i].xyz[colIndex];
    if (mysmallest > smallest && smallest > 0)
      mysmallest = smallest;
  }
  smallest = mysmallest;
  MPI_Allgather(&mysmallest, 1, ld_type, allSmallest, 1, ld_type, MPI_LOCAL_COMM);
  //AllgatherLD(&mysmallest, allSmallest, 1);
  smallest = allSmallest[0];
  for (i=1;i<num_ranks;i++){
    if (allSmallest[i] < smallest)
      smallest = allSmallest[i];
  }
  //============================
  //
  // Adjust Each L until "Balanced"
  //
  //============================
  
  totalIterations = 0;
  startIndex = 1;
  while (*balanced == 0){
    
    *balanced = 1;
    i = startIndex;
    //for (i=startIndex; i<num_ranks;i++){ //check each Li for i = 1-> N-2
    while (i < num_ranks && *balanced == 1){
      startIndex = i;
      keepGoing = 1;
      if (my_rank == 0){
	if (fabsl(sDiff[i-1]) > 0.10*K){
	  
	  *balanced = 0;
	  myrange = rangeL[i];
	  if (myrange < smallest){
	    j = i+1; //startIndex;
	    while (j<num_ranks && myrange < smallest){
	      myrange += rangeL[j];
	      j++;
	    }
	    j = i-1; //startIndex;
	    while (j>0 && myrange < smallest){
	      myrange += rangeL[j];
	      j--;
	    }
	  }  
	    
	  // Fix Range
	  if (adjustCount[i] != 0){ // NOT first iteration
	    if (myrange*percentRange[i] > smallest){// if stepsize not to small
	      
	      if (fabsl(prevsDiff[i-1]) < fabsl(sDiff[i-1])) //previous step to big
		percentRange[i] /=2;
	      else if (prevsDiff[i-1]*sDiff[i-1] < 0) // stepped over target
		percentRange[i] /=2;
	    }else{                                 // stepsize too small
	      if (totalCount[i-1] > 0){ //skip node if count != 0
		startIndex = i+1;
		zeroIterations = 0;
		keepGoing = 0;
		//break; // Go to next node immediately (skip adjust)
	      }else{
		zeroIterations+=1;
		if (zeroIterations >2){ // if total count == 0 for more than 2 iterations then stop
		  startIndex = i+1;
		  keepGoing = 0;
		  //break; // Go to next node immediately (skip adjust)
		}
	      }
	    }
	  }
	  // Make Adjustments
	  if (keepGoing == 1){
	    adjustCount[i] += 1;
	    if (sDiff[i-1] > 0)
	      L[i] = L[i] - myrange*percentRange[i];
	    else
	      L[i] = L[i] + myrange*percentRange[i];
	    
	    // Fix if Li is out of range 
	    for (j=0;j<num_ranks-1;j++){
	      if (L[j] > L[j+1]){
		startIndex = j;
		L[j+1] = L[j];
		for (k=startIndex; k<num_ranks; k++){//(redo => instantiate count from new start index)
		  adjustCount[k] = 0;
		  percentRange[k] = 0.1;
		}
	      }
	    } 
	    for (j=num_ranks;j>1;j--){
	      if (L[j] < L[j-1]){
		startIndex = j-1; //NOTE: startIndex >= 1
		L[j-1] = L[j];
		for (k=startIndex; k<num_ranks; k++){//(redo => instantiate count from new start index)
		  adjustCount[k] = 0;
		  percentRange[k] = 0.1;
		}
	      }
	    }	    
	    
	  }
	}
      }
      //SEND ALL VARIABLES TO OTHER RANKS
      myflags[0] = *balanced;
      myflags[1] = keepGoing;
      myflags[2] = i;
      //my_Bcast_int(balanced, 1, 0);
      //my_Bcast_int(&keepGoing, 1, 0);
      //my_Bcast_int(&i, 1, 0);
      //my_Bcast_int(myflags, 3, 0);
      MPI_Bcast(myflags, 3, MPI_INT, 0, MPI_LOCAL_COMM);
      *balanced = myflags[0];
      keepGoing = myflags[1];
      i = myflags[2];
      //if (my_global_rank <= 1){
      //	printf("balanced: %d; keepGoing: %d; i: %d; gid%03d\n", *balanced, keepGoing, i, my_global_rank);
      //}
      if (*balanced == 0 &&  keepGoing == 0){
	//my_Bcast_ld(L, num_ranks, 0);
	MPI_Bcast(L, num_ranks, ld_type, 0, MPI_LOCAL_COMM);
	getCounts(num, colIndex, array, L, totalCount, allCounts);
	//my_Bcast_int(&startIndex, 1, 0);
	MPI_Bcast(&startIndex, 1, MPI_INT, 0, MPI_LOCAL_COMM);
      }
      

      //SET ALL DIFF VARS
      if (my_rank == 0 && *balanced == 0){
	  // Set new Vars
	  smallestDiffCheck = 0;
	  for (j=0; j<num_ranks; j++){
	    prevsDiff[j] = sDiff[j]; // Set prefsDiff before sDiff changes
	    sDiff[j] = totalCount[j]-K;
	    smallestDiffCheck += sDiff[j];
	    if (j > 0)// j < num_ranks-1 )
	      rangeL[j] = L[j+1] - L[j-1];
	  }
	  if (smallestDiffCheck < smallestDiff){
	    smallestDiff = smallestDiffCheck;
	    for (j=0; j<num_ranks; j++){
	      smallestDiffL[j] = L[j];
	    }
	  }
	  
      }
      
      i++;
    }// break out of here  
    totalIterations++;
    if (totalIterations > num_ranks*100)
      break;
    
  }
  
  // Use smallestDiffL if it's better than L
  if (my_rank == 0){
    smallestDiffBool = 1;
    smallestDiffCheck = 0;
    for (j=0; j<num_ranks; j++){
      smallestDiffCheck += sDiff[j];
    }
    if (smallestDiff < smallestDiffCheck){
      smallestDiffBool = 0;
      for (j=0; j<num_ranks; j++){
	L[i] = smallestDiffL[i];
      }
    }
  }
  if (afterAdjustLflag == 1)
    printf("AFTERADJUSTL gid%03d\n", my_global_rank);
  MPI_Bcast(&smallestDiffBool, 1, MPI_INT, 0, MPI_LOCAL_COMM);
  //my_Bcast_int(&smallestDiffBool, 1, 0);
  if (smallestDiffBool == 0){
    MPI_Bcast(L, num_ranks, ld_type, 0, MPI_LOCAL_COMM);
    //my_Bcast_ld(L, num_ranks, 0);
    getCounts(num, colIndex, array, L, totalCount, allCounts);
  }
  if (Bcastflag == 1)
    printf("BCAST gid%03d\n", my_global_rank);
  free(allSmallest);
}

void getCounts(int num,  const int colIndex, void* varray, void *vL, void *vtotalCount, void *vallCounts){
  struct data_struct  *array = (struct data_struct *) varray;
  float* L = (float *) vL;
  int* totalCount = (int *) vtotalCount;
  int* allCounts = (int *)vallCounts; 
  int* nodeCount = (int *) malloc(num_ranks*sizeof(int));
  float totalMax; // = LDivinfo[num_ranks];
  MPI_Status status;
  int total=0,K;
  int i, j=0, k=0, D;
  //============================
  //
  // Get nodeCount
  // Counts of each division on each node
  //
  //============================
 
  j = 0;
  
  for (i=0;i<num_ranks;i++){
    nodeCount[i] = 0;
    totalCount[i] = 0;
  }

  //============================
  //
  // LINEAR METHOD
  //
  //============================

  k=num_ranks-1;
  for (i = num-1; i>= 0; i--){
    if (array[i].xyz[colIndex] >= L[k])
      nodeCount[k]++;
    else{
      while (array[i].xyz[colIndex] < L[k])
	k--;
      nodeCount[k]++;
    }
  }
  
  //for (i=0;i<num_ranks;i++){
  //  MPI_Barrier(MPI_LOCAL_COMM);
  //  if (my_rank == i){
  //    printf("my_rank: %u; BEFORE BISECTION\n", my_rank);
  //    for (j=0;j<num_ranks;j++)
  //printf("nodeCount[%u]: %u\n", j, nodeCount[j]);
  //  }
  //}

  //============================
  //
  // BISECTION METHOD2
  //
  //============================
  //int midi, maxi = num, mini=0, sumi = 0;
  //k = 0;
  //for (k=1;k<num_ranks;k++){
  //  j = 0;
  //  while (1){
  //    midi = (int)(maxi+mini)/2;
  //    if (L[k] > array[midi].xyz[colIndex]){
  //	if (midi == mini){
  //	  midi = maxi;
  //	  break;
  //	}
  //	mini = midi;
  //    }else if (L[k] > array[midi-1].xyz[colIndex] || midi == mini){
  //	break;
  //    }else{
  //	maxi = midi;
  //    }  
  //  }
  //  maxi = num;
  //  mini = midi;
  //  if (k==1)
  //    nodeCount[k-1] = midi;
  //  else{
  //    nodeCount[k-1] = midi;
  //    for(j = k-2;j>=0;j--)
  //	nodeCount[k-1] -= nodeCount[j];
  //  }
  //  
  //}
  //nodeCount[num_ranks-1] = num;
  //for(j = num_ranks-2;j>=0;j--)
  //  nodeCount[num_ranks-1] -= nodeCount[j];

  //============================
  //
  // Get allCounts
  // share nodeCounts with all nodes
  //
  //============================
  
  MPI_Allgather(nodeCount, num_ranks,MPI_INT, allCounts,num_ranks,MPI_INT, MPI_LOCAL_COMM);
  //AllgatherINT(nodeCount, allCounts, num_ranks);
  //============================
  //
  // Get totalCounts
  // the minimum of each node
  //
  //============================


  for (i=0;i<num_ranks;i++){
    for (j=0;j<num_ranks;j++){
      totalCount[j] +=  allCounts[i*num_ranks + j];
    }
  
  }
  
  
  free(nodeCount);
}

void checkBalance(void *vbalanced, void *vtotalCount){
  int* balanced = (int *) vbalanced;
  int* totalCount = (int *) vtotalCount;
  int total=0,K;
  int i;
  
  //============================
  //
  // Get K and check balance
  // K = total/N
  // balance => abs(Si-K) < .10*K
  //
  //============================


  *balanced = 1;
  total = 0;
  for (i=0;i<num_ranks;i++){
    total += totalCount[i];
  }
  
  K = (int)(total/num_ranks);
  for (i=0;i<num_ranks-1;i++){
    if (labs(totalCount[i]-K) > .10*K){
      *balanced = 0;
      break;
    }
    
  }  
}

void printNodeL(void* vL){
  float  *L = (float *) vL;
  int i;
  for (i=0;i<num_ranks+1;i++)
    printf("%15Lf\t", i,L[i]);
  
  printf("\n");

}

void printCount(void *vallCounts){
  int* allCounts = (int *)vallCounts;
  int* totalCount = (int *) malloc(num_ranks*sizeof(int));
  int i,j,k;

  if (my_rank == 0){
    printf("%15s|", " ");
    for (i=0; i<num_ranks; i++){
      printf("%15u|", i);
      totalCount[i] = 0;
    }
    printf("\n");
    k = 0;
    for (i=0; i<num_ranks; i++){
      printf("%15u|",i);
      for (j=0; j<num_ranks; j++){
	
	printf("%15u|",allCounts[k]);
	totalCount[j] += allCounts[k];
	k++;
      }
      printf("\n");
      
    }
    printf("%15s|", "total");
    for (i=0; i<num_ranks; i++){
      printf("%15u|", totalCount[i]);
    }
    printf("\n================\n\n");
  }
  free(totalCount);
}
