#ifndef HEADER_FILE
#define HEADER_FILE
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "mpi.h"

struct data_struct{
  long int num;
  float xyz[3];
};


struct node{
  float max[3], min[3], maxRadius;
  struct node *left, *right;
  int num_below;
  struct data_struct *center; //leafValue;
};

struct snode{
  float max[3], min[3];
  struct node *left, *right;
};

struct Gnode{
  float max[3], min[3], maxRadius;
  struct Gnode *left, *right, *parent;
  int this_rank, num_below, assigned;
  struct data_struct *center; //leafValue;
};

struct commgroupcollection{
  int this_num_ranks;
  int *ranks;// = {0,0,0};
  MPI_Group localgroup;
  MPI_Comm localcomm;
  struct commgroupcollection * prev;
  struct commgroupcollection * next;
  

};

//MPI_Comm MPI_COMM_WORLD;
MPI_Comm MPI_LOCAL_COMM, MPI_TEMP_COMM, dup_comm_world;
MPI_Group world_group, local_group;
MPI_Datatype array_type;
MPI_Datatype ld_type, li_type;
MPI_Status stat;
struct commgroupcollection * myCommCollection, * tempCollection;
int num_ranks, global_num_ranks, numRanges, maxLevel;
int my_rank, my_global_rank;
int timePrint;
int maxminflag,largestdimflag,globalsortflag,splitranksflag;
int getallcountflag, alltoallflag;
int getbucketsflag,getcountsflag,inAdjustLflag,afterAdjustLflag,Bcastflag;

void my_Bcast_int(void *, int, int);
void my_Bcast_ld(void *, int, int);
void AllgatherLD(void *, void *, int);
void AllgatherINT(void *, void *, int);


void compareFunc(struct data_struct*, struct data_struct, int);
struct Gnode * buildEmptyGtree(struct Gnode *, int, int);
void getGNode(void *);
void printGNode(void *);
void printGTree(void *);
void getSendSize(struct Gnode *, float, struct data_struct *, int, int *);
void initAssigned(struct Gnode *);
void getSendSize1(struct Gnode *, float, struct data_struct *, int, int *);
void getSendSize1Target(struct Gnode *, float, struct data_struct, int *,struct Gnode **);
int sendSizeTest(struct node *, float, struct data_struct *, int);
void searchTest(struct data_struct *,struct data_struct*, int);
void compareTargets(struct data_struct *, struct data_struct *, int, int);
void compareTargetsLocalTree(struct node *, struct data_struct *, int);
void globalTreeMaster(struct Gnode *, struct node *);
void  getSendArray(struct Gnode *, float, struct data_struct *, int , struct data_struct *, int *, int);
void getSendArray1Target(struct Gnode *, float, struct data_struct , int *, int);

struct data_struct* globalSort(void *, int *, int, int *);

void localSearch(struct node *, struct data_struct,struct node **, long int * );

void getMaxMin(void*, int, int, float*, float*);
void getLargestDimension(float *, float *, int *);
void getNode(int, void *);
void printNode(void *);
void buildTree(void *, int, void *, int);

void getMaxMinGlobal(void*, int, int, float*, float*);
void getLargestDimensionGlobal(float *, float *, int *);
void getNodeGlobal(int, void *, int);
void printNodeGlobal(void *);
struct node * buildTreeGlobal(void *, int, void *, int);
//struct node * splitRanks(void *, int , void *, void *, int );
void splitRanks();
     
void getArraySize(const char*, int*);
void readFromFile(char*, const int, void*);
void readFromFileAllRead(int, void*);
void readFile1(int , void* );
void printFile( const int, void*);

double timestamp();
int compare_datastruct(const void*, const void*, int);
int compare_x(const void*, const void*);
int compare_y(const void*, const void*);
int compare_z(const void*, const void*);
int compare_longfloat(const void*, const void*);

void getallCount(int, const int, void*, void*);
void getCounts(int, const int, void*, void *, void *, void *);
void checkBalance(void *, void *);
void printNodeL(void*);
void adjustL(int, const int, void*, void *, void*, void *, void*);
void do_sort(struct data_struct*, int, int);
void printCounts(void *);
void verify(float, float);

struct data_struct*  AllToAllSend(void *, int *, void *);
struct data_struct*  AllToAllIsend(void *, int *, void *);
void create_array_datatype();
#endif
