// Global.h
// function: some global variables which will be used by the program

#ifndef GLOBAL_H
#define GLOBAL_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include "Package.h"
#include <unordered_map>
#include <string>


using namespace std;


// declare the global variables
extern double PRECISION;   // used for tMRCA's partial hashing
extern double NUMERIC;		// this is also for the float number hashing's use
extern double TOLERANCE;   // the tMRCA change tolerance
extern long int SAMPLE;    // the sample size
extern long int TREELEN;   // the string lenth of a Nexus/Newick tree
extern int THREADS;			// the number of threads we used
extern long int CUTOFF;		// the IBD segment cutoff value
extern long int FILTER;		// the filter value used for candidate extraction; half of the CUTOFF
extern long int DISCRETIZATION;	// process trees every "discretization" base pairs
extern long int CLENGTH;	// the length of the chromosome; we don't know it in Nexus
extern int FORMAT;			// the format of the tree: Nexus(0) or Newick(1)
extern int INPUTSRC;		// the source of the trees: tree file(0) or stdin(1)


// the following are about the multi-threads
extern pthread_t thread[10];  // we support at most 10 threads, because even a working station will
						// always have at most about 10 cores
extern pthread_mutex_t mut_read;  // mutex used by all the threads; used in reading the tree file
extern pthread_mutex_t mut_write;  // mutex used by all the threads; used in saving the IBD segments into file

// the filehandle for the output file; global, to permit all the threads access it; using the mut above to protect
extern FILE * file_out;

// the boundary block_package pointer list
//extern vector<block_boundary_package> boundary_block;

extern char filename[50];

// the following will be used in the LCA(RMQ) algorithm; to avoid data transfer, we can make the global accessbie
// they will be fixed after we know the SAMPLE size
extern int BLOCK_NUM;
extern int BLOCK_SIZE;
extern int BLOCK_SIZE_LAST;
extern unordered_map<string, unordered_map<string, int>> MODE_BLOCK;
extern unordered_map<string, unordered_map<string, int>> MODE_BLOCK_LAST;


#endif

// end of Global.h