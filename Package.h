// Package.h
// function: some structures used by the whole program

#ifndef PACKAGE_H
#define PACKAGE_H

#include <vector>
#include <unordered_map>


using namespace std;


// the variable space used for chunks
typedef struct chunk_package
{
	int seq;
	long int start;		// the start coordinate of the chromosome for this chunk
	long int start_exp;	// the expected start coordinate for this chunk
	long int end_exp;	// the expected end coordinate for this chunk
	long int * table;	// the record table
}chunk_package;

//========================== for the following two ============================
// why we use table for boundary block, but hashtable for the in-chunk block?
// there are lots of unchanged pairs for the boundary block, but not so many for the in-chunk block
// the variable space used for blocks
typedef struct block_package
{
	int first;				// whether or not this is the first block in the first chunk
	int last;				// whether or not this is the last block in the last chunk
	int chunk_num;			// the number of the chunk this block belongs to; used to judge the last block in the last chunk/first block in the first chunk
	//vector<char *> vector;	// the addresses of all the trees; the trees are in HEAP
	// it seems that we can't use changable vector in a struct
	// instead we use the following to mark the range of the trees in this block
	// they may be 0 for example, the boundary block
	long int start;			// the start coordinate of this block
	long int end;			// the end coordinate of this block
	unordered_map<long int, double> hashtable;
							// the hashtable storing all the unchanged pairs among this block
							// the hashing value is their tMRCA
}block_package;


// specially for the boundary block
/* we don't use boundary blocks any more
typedef struct block_boundary_package
{
	long int start;			// the start coordinate of this block
	long int end;			// the end coordinate of this block
	//double table[SAMPLE * SAMPLE];
	double * table;
							// the value is the tMRCA value
}block_boundary_package;
*/


#endif

// end of Package.h