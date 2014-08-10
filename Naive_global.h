// Global.h
// function: some global variables which will be used by the program

#ifndef NAIVE_GLOBAL_H
#define NAIVE_GLOBAL_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>


// declare the global variables
extern double PRECISION;   // used for tMRCA's partial hashing
extern double NUMERIC;		// this is also for the float number hashing's use
extern double TOLERANCE;   // the tMRCA change tolerance
extern long int SAMPLE;    // the sample size
extern long int TREELEN;   // the string lenth of a Nexus/Newick tree
extern int THREADS;			// the number of threads we used
extern long int CUTOFF;		// the IBD segment cutoff value
extern long int DISCRETIZATION;	// process trees every "discretization" base pairs
extern long int CLENGTH;	// the length of the chromosome; we don't know it in Nexus
extern int FORMAT;			// the format of the tree: Nexus(0) or Newick(1)
extern int INPUTSRC;		// the source of the trees: tree file(0) or stdin(1)


// the following are about the multi-threads
extern pthread_t thread[20];  // we support at most 10 threads, because even a working station will
						// always have at most about 10 cores
extern pthread_mutex_t mut_read;  // mutex used by all the threads; used in reading the tree file
extern pthread_mutex_t mut_write;  // mutex used by all the threads; used in saving the IBD segments into file
// the filehandle for the output file; global, to permit all the threads access it; using the mut above to protect
extern FILE * file_out;
// the boundary block_package pointer list (not used for the naive algorithm)
//extern vector<block_boundary_package> boundary_block;


extern char filename[50];
extern char filename_out[50];


extern int FROMONE;		// the sample name is from one or zero; default is one


extern char * tree_stdin;	// used for the global memory space of the tree from stdin

// !!!! AUTION here !!!!
// as in the stdin there may be one line with many characters in, the allocated heap may be
// overflowed; so we use a constant large buffer size here as 200000
extern long int stdin_buffer;	// the buffer size of stdin (the maximum possible length of each line from the stdin)


#endif

// end of Global.h