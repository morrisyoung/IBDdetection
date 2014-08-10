// Verify_chunk.h

#ifndef CHUNK_LEVEL_H
#define CHUNK_LEVEL_H

#include "Package.h"
#include <stdio.h>
#include <list>


using namespace std;


long int get_tree(FILE *, list<char *> *);

char * block_finding(block_package *, list<char *> *, FILE *, char *, long int);

void * chunk_level(void *);

void chunk_merge(chunk_package *, int);

#endif

// end of Verify_chunk.h