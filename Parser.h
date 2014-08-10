// Parser.h
// different types of parsers; perform different functions; all based on the stack and 
// in-order traversal for the tree

#ifndef PARSER_H
#define PARSER_H

#include <unordered_map>

#define TRUE 1
#define FALSE 0


using namespace std;


// input: the tree
// output: coordinate; updated all-pair tMRCAs table
long int parser(char *, double *);

// get the SAMPLE size automatically
long int parser_sample(char *);

// input: the tree and the sample pair
// output: the tMRCA of that pair in that tree
double tMRCA_find(char *, long int, long int);

// input: two trees; one specific pair
// output: whether or not that pair has not changed tMRCA between these two trees
int tMRCA_compare(char *, char *, long int, long int);

// input: two trees
// output: the updated table, in which there are unchanged pairs between this two trees, and their tMRCA
void unchanged_pairs(char *, char *, double *);

// input: two trees
// output: the updated hashtable, in which there are unchanged pairs between this two trees, and their tMRCA
void unchanged_pairs_hash(char *, char *, unordered_map<long int, double> *);

// input: two trees
// output: the updated hashtable, in which there are changed pairs between this two trees, and their new tMRCA
void changed_pairs_hash(char *, char *, unordered_map<long int, double> *);

// input: one tree
// output: the updated hashtable, in which there are all pairs with their tMRCA values
void all_pairs_hash(char *, unordered_map<long int, double> *);

#endif

// end of Parser.h