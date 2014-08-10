// Verify_lca.h
// Bender&Colton's LCA algorithm; The LCA Problem Revisited (2000);
// start from parsing the Nexus/Newick tree

#ifndef VERIFY_LCA_H
#define VERIFY_LCA_H


using namespace std;


void LCA_list_construction(char *);  //Euler tour for a tree to get the E[1,...,2n-1], L[1,...,2n-1], R[1,...,n]

void Dynamic();

// RMQ pre-processing from here
void LCA_RMQ();

// the entrance to the pre-processing
void LCA_preprocess(char *);

// tMRCA query
double getMRCA(long int, long int);

#endif

// end of Verify_lca.h