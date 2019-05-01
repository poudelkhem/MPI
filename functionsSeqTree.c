#include "headerFuncs.h"

//void buildTree(void *varray, int num, void *vnode, int colIndex){
//  struct data_struct* array  = (struct data_struct *)varray;
//  struct node *anode = (struct node *)vnode;
//  int i,j,k;
//  if (num > 1){
//    //Find biggest Dimension
//    // BINARY SEARCH FOR MAX AND MIN
//    getMaxMin(array, num, colIndex, anode->max, anode->min); 
//    getLargestDimension(anode->max, anode->min, &colIndex);
//
//    // Sort Along Dimension
//    do_sort(array, num, colIndex);
//  
//    getNode(num, anode);
//
//    anode->left = (struct node *)malloc(sizeof(struct node));
//    anode->right = (struct node *)malloc(sizeof(struct node));
//    for (i=0;i<3;i++){
//      anode->left->max[i] = anode->max[i];
//      anode->left->min[i] = anode->min[i];
//      anode->right->max[i] = anode->max[i];
//      anode->right->min[i] = anode->min[i];
//    }
//
//    int index = (int)num/2;
//    if (num%2 == 0){
//      buildTree(array, index, anode->left, colIndex);
//      buildTree(&array[index], index, anode->right, colIndex);
//    }else{    
//      buildTree(array, index, anode->left, colIndex);
//      buildTree(&array[index], index+1, anode->right, colIndex);
//    }
//  }else{
//    anode->center = array;
//    anode->num_below = 1;
//    
//    
//  }
//  
//}

void buildTree(void *varray, int num, void *vnode, int colIndex){
  struct data_struct* array  = (struct data_struct *)varray;
  struct node *anode = (struct node *)vnode;
  struct node ** childArray = (struct node **)malloc(num*sizeof(struct node *));
  int start, *starts, *ends;
  starts = ( int *)calloc(num,sizeof(int));
  ends = ( int *)calloc(num,sizeof(int));
  int i,j,k, caSize = 0;
  //char fname[20];
  //sprintf(fname,"/home/gst2d/Final/nodes%03u.txt", my_global_rank);
  //FILE *myfile = fopen(fname, "w");
  getMaxMin(array, num, colIndex, anode->max, anode->min); 
  getLargestDimension(anode->max, anode->min, &colIndex);

  // Sort Along Dimension
  do_sort(array, num, colIndex);
  
  getNode(num, anode);
  //fprintf(myfile,"LRank: %03u;\nMax: (%15f,%15f,%15f);\nMin: (%15f,%15f,%15f);\nmaxRadius: %15f;\nnum_below: %15u\n========\n",
  //	  my_global_rank,
  //	  anode->max[0],anode->max[1],anode->max[2],
  //	  anode->min[0],anode->min[1],anode->min[2],
  //	  anode->maxRadius,
  //	  anode->num_below);
  
  anode->left = (struct node *)malloc(sizeof(struct node));
  anode->right = (struct node *)malloc(sizeof(struct node));
  for (i=0;i<3;i++){
    anode->left->max[i] = anode->max[i];
    anode->left->min[i] = anode->min[i];
    anode->right->max[i] = anode->max[i];
    anode->right->min[i] = anode->min[i];
  }
  childArray[0] = anode->left;
  starts[0] = 0;
  ends[0] = (int)num/2;
  childArray[1] = anode->right;
  starts[1] = (int)num/2;
  if (num%2 == 0){
    ends[1] = (int)num/2;
  }else{
    ends[1] = (int)num/2 + 1;
  }
  caSize = 2;
  while ( caSize > 0){
    anode = childArray[0];
    start = starts[0];
    num = ends[0];
    //if (my_global_rank == 14)
    //  printf("rank: %d; caSize: %d; start: %d, num: %d\n", my_global_rank, caSize, start, num);
    //Find biggest Dimension
    // BINARY SEARCH FOR MAX AND MIN
    if (num > 1){
      getMaxMin(&array[start], num, colIndex, anode->max, anode->min); 
      getLargestDimension(anode->max, anode->min, &colIndex);
    
      // Sort Along Dimension
      do_sort(&array[start], num, colIndex);
  
      getNode(num, anode);
      
      anode->left = (struct node *)malloc(sizeof(struct node));
      anode->right = (struct node *)malloc(sizeof(struct node));
      for (i=0;i<3;i++){
	anode->left->max[i] = anode->max[i];
	anode->left->min[i] = anode->min[i];
	anode->right->max[i] = anode->max[i];
	anode->right->min[i] = anode->min[i];
      }
      for (i=0;i<caSize;i++){
	childArray[i] = childArray[i+1];
	starts[i] = starts[i+1];
	ends[i] = ends[i+1];
      }
      caSize += 1;
      childArray[caSize-2] = anode->left; //, radius, target, sendSize);
      starts[caSize-2] = start;
      ends[caSize-2] = (int)num/2;
      childArray[caSize-1] = anode->right;
      starts[caSize-1] = start + (int)num/2;
      if (num%2 == 0){
	ends[caSize-1] = (int)num/2;
      }else{
	ends[caSize-1] = (int)num/2 + 1;
      }
      //fprintf(myfile,"LRank: %03u;\nMax: (%15f,%15f,%15f);\nMin: (%15f,%15f,%15f);\nmaxRadius: %15f;\nnum_below: %15u\n========\n",
      //	      my_global_rank,
      //	      anode->max[0],anode->max[1],anode->max[2],
      //	      anode->min[0],anode->min[1],anode->min[2],
      //	      anode->maxRadius,
      //	      anode->num_below);
    }else{
      for (i=0;i<caSize;i++){
	childArray[i] = childArray[i+1];
	starts[i] = starts[i+1];
	ends[i] = ends[i+1];
      }
      caSize -= 1;
      anode->center = &array[start];
      anode->num_below = 1;
      //fprintf(myfile,"LRank: %03u;\npointId: %Ld\nCenter: (%15f,%15f,%15f);\nnum_below: %15u\n========\n",
      //	      my_global_rank,
      //	      anode->center->num,
      //	      anode->center->xyz[0],anode->center->xyz[1],anode->center->xyz[2],
      //	      anode->num_below);
    }
    
    
  }
  //fclose(myfile);
  free(childArray);
  free(starts);
  free(ends);  
}


void getNode(int num, void *vnode){
  struct node *anode = (struct node *)vnode;
  int i;
  float radToMax=0, radToMin=0;
  float temp;
  //anode->center = (struct data_struct *)malloc(sizeof(struct data_struct));
  anode->maxRadius = 0;
  for (i=0;i<3;i++){
    temp = (anode->max[i] + anode->min[i])/2;
    radToMax += (temp-anode->max[i])*(temp-anode->max[i]);
    radToMin += (temp-anode->min[i])*(temp-anode->min[i]);
  }
  
  radToMax = sqrt(radToMax);
  radToMin = sqrt(radToMin);
  if (radToMax > radToMin)
    anode->maxRadius = radToMax;
  else
    anode->maxRadius = radToMin;
  
  anode->num_below = num;
  

}

void printNode(void *vnode){
  struct node *anode = (struct node *)vnode;
  printf("Max: (%15Lf,%15Lf,%15Lf);\n Min: (%15Lf,%15Lf,%15Lf);\n Center: (%15Lf,%15Lf,%15Lf);\n maxRadius: %15Lf;\n num_below: %15u\n", 
	 anode->max[0],anode->max[1],anode->max[2],
	 anode->min[0],anode->min[1],anode->min[2],
	 anode->center->xyz[0],anode->center->xyz[1],anode->center->xyz[2],
	 anode->maxRadius,
	 anode->num_below);
}

void getLargestDimension(float *arrayMax, float *arrayMin, int *colIndex){
  float range = arrayMax[0] - arrayMin[0];
  *colIndex = 0;
  if (range < (arrayMax[1] - arrayMin[1])){
    *colIndex = 1;
    range = arrayMax[1] - arrayMin[1];
  }
  if (range < (arrayMax[2] - arrayMin[2])){
    *colIndex = 2;
    range = arrayMax[2] - arrayMin[2];
  }
  
}

void getMaxMin(void* varray, int size,  int colIndex, float *arrayMax, float *arrayMin){
  struct data_struct* array = (struct data_struct *)varray;
  int i,j,k;
  if (colIndex < 0){
    for (i=0;i<3;i++){
      arrayMax[i] = array[0].xyz[i];
      arrayMin[i] = array[0].xyz[i];
    }

    for (i=1;i<size;i++){
      for (j=0;j<3;j++){
	if (arrayMax[j] < array[i].xyz[j])
	  arrayMax[j] = array[i].xyz[j];
	else if (arrayMin[j] > array[i].xyz[j])
	  arrayMin[j] = array[i].xyz[j];
      }
    }
  }else{
    arrayMax[colIndex] = array[0].xyz[colIndex];
    arrayMin[colIndex] = array[0].xyz[colIndex];
    

    for (i=1;i<size;i++){
      
      if (arrayMax[colIndex] < array[i].xyz[colIndex])
	arrayMax[colIndex] = array[i].xyz[colIndex];
      else if (arrayMin[colIndex] > array[i].xyz[colIndex])
	arrayMin[colIndex] = array[i].xyz[colIndex];
      
    }
    
  }
  

}
