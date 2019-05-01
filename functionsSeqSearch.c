#include "headerFuncs.h"
//int localSearch(struct node * anode, float radius, struct data_struct target){
//  int i,j,k, count = 0;
//  float targetDir[3], targetPoint[3], targetMagnitude = 0, testRadius=0;//, dist = 0;
//  if (anode->num_below > 1){
//    for (i=0;i<3;i++){
//      targetDir[i] = (target.xyz[i]-anode->center->xyz[i]);
//      targetMagnitude += targetDir[i]*targetDir[i];
//    }
//  
//    targetMagnitude = sqrt(targetMagnitude);
//    for (i=0;i<3;i++){
//      targetDir[i] = targetDir[i]/targetMagnitude;
//      targetDir[i] *= anode->maxRadius;
//      targetPoint[i] = anode->center->xyz[i] + targetDir[i];
//      testRadius += pow(target.xyz[i] - targetPoint[i],2);
//    
//    
//    }
//    testRadius = sqrt(testRadius);
//  
//    if (targetMagnitude < anode->maxRadius || testRadius < radius){ //THIS MEAND NODE MAX RADIUS IS IN RADIUS OF TARGET
//      count += localSearch(anode->left, radius, target);
//      count += localSearch(anode->right,radius, target);
//      return count;
//    }else{
//      return 0;
//    }
//    
//  }else{
//    for (i=0;i<3;i++){
//      targetMagnitude += pow(target.xyz[i]-anode->center->xyz[i],2);
//    }
//    
//    targetMagnitude = sqrt(targetMagnitude);
//    if (targetMagnitude < radius){
//      return 1;
//    }else
//      return 0;
//  }
//}


void localSearch(struct node * anode, struct data_struct target, struct node ** childArray, long int * counters){
  int i,j,k,count=0,caSize = 0, it = 0, start,end, numOfLeaves = anode->num_below;
  float targetDir[3], targetPoint[3], targetMagnitude = 0, testRadius=0, temp, stime,etime,dtime;//, dist = 0;
  float radius = 0.1, tradius, radiiList[3] = {0.01,0.05,0.1};
  
  for (i=0; i<3; i++)
    counters[i] =0;

  //struct node ** childArray = (struct node **)malloc(anode->num_below*sizeof(struct node *));
  //stime = timestamp();
  for (i=0;i<3;i++){
    temp = (anode->max[i] + anode->min[i])/2;
    targetDir[i] = (target.xyz[i]-temp);
    targetMagnitude += targetDir[i]*targetDir[i];
  }
  
  targetMagnitude = sqrt(targetMagnitude);
  for (i=0;i<3;i++){
    temp = (anode->max[i] + anode->min[i])/2;
    targetDir[i] = targetDir[i]/targetMagnitude;
    targetDir[i] *= anode->maxRadius;
    targetPoint[i] = temp + targetDir[i];
    testRadius += pow(target.xyz[i] - targetPoint[i],2);
  }
  testRadius = sqrt(testRadius);
  if (targetMagnitude < anode->maxRadius || testRadius < radius){ //THIS MEAND NODE MAX RADIUS IS IN RADIUS OF TARGET
    caSize = 2;
    start = 0;
    end = 1;
    childArray[0] = anode->left; //, radius, target, sendSize);
    childArray[1] = anode->right;    
  }
  //if (my_global_rank == 14 && target.num == 925)
  //  printf("LRank: %03u;\nradius: %f\nMax: (%15f,%15f,%15f);\nMin: (%15f,%15f,%15f);\nmaxRadius: %15f;\nnum_below: %15u\ntarget: (%15f,%15f,%15f)\n========\n",
  //	   my_global_rank,
  //	   radius,
  //	   anode->max[0],anode->max[1],anode->max[2],
  //	   anode->min[0],anode->min[1],anode->min[2],
  //	   anode->maxRadius,
  //	   anode->num_below,
  //	   target.xyz[0],target.xyz[1],target.xyz[2]);
  while (caSize > 0){
    anode = childArray[start];
    targetMagnitude = testRadius = 0;
    if (anode->num_below > 1){
      for (i=0;i<3;i++){
	temp = (anode->max[i] + anode->min[i])/2;
	targetDir[i] = (target.xyz[i]-temp);
	targetMagnitude += targetDir[i]*targetDir[i];
      }      
      targetMagnitude = sqrt(targetMagnitude);
      if (targetMagnitude < anode->maxRadius){
	caSize += 1;
	start = (start + 1)%numOfLeaves;
	end = (end + 1)%numOfLeaves;
	childArray[end] = anode->left; 
	end = (end + 1)%numOfLeaves;
	childArray[end] = anode->right;   
      }else{
	for (i=0;i<3;i++){
	  temp = (anode->max[i] + anode->min[i])/2;
	  targetDir[i] = targetDir[i]/targetMagnitude;
	  targetDir[i] *= anode->maxRadius;
	  targetPoint[i] = temp + targetDir[i];
	  testRadius += pow(target.xyz[i] - targetPoint[i],2);
	}
	testRadius = sqrt(testRadius);
	if (testRadius < radius){ 
	  caSize += 1;
	  start = (start + 1)%numOfLeaves;
	  end = (end + 1)%numOfLeaves;
	  childArray[end] = anode->left;
	  end = (end + 1)%numOfLeaves;
	  childArray[end] = anode->right;   
	}else{
	  start = (start + 1)%numOfLeaves;
	  caSize -= 1;
	} // BOUNDARY TO TARGET ( TEST RADIUS)
      } // CENTER TO TARGET ( TARGET MAGNITUDE)
      
      
    }else{ // LEAF NODE
      for (i=0;i<3;i++){
	targetMagnitude += pow(target.xyz[i]-anode->center->xyz[i],2);
      }
      targetMagnitude = sqrt(targetMagnitude);
      for (i = 0;i<3;i++){
	tradius = radiiList[i];
	if (targetMagnitude < tradius){
	  
	  counters[i] += 1;
	}
      }
      //if (my_global_rank == 14 && target.num == 925 && target.num == anode->center->num)
      //	printf("count: %d\nCenter: (%15f,%15f,%15f);\n", count,target.xyz[0],target.xyz[1],target.xyz[2]);
      //for (i=0;i<caSize;i++)
      //	childArray[i] = childArray[i+1];
      //
      start = (start + 1)%numOfLeaves;
      caSize -= 1;	
    }
    //it++;
  }
  //etime = timestamp();
  //dtime = etime-stime;
  //printf("myrank: %d; iterations: %d; time: %f\n", my_global_rank, it, dtime);
  //free(childArray);
  //return count;
}
