// MRCA_hash.h
// hashing-based tMRCA unchanged pairs searching algorithm

#ifndef IBD_HASH_H
#define IBD_HASH_H

#include <unordered_map>
#include <forward_list>
#include <vector>


using namespace std;


// the MRCA struct for the h1/h11, storing the tMRCA value and the pointed position in list1
typedef struct MRCA
{
    double tMRCA;
    int start;
    int middle;
    int end;
}MRCA;


void parser1(char *);

void parser2(char *, unordered_map<long int, double> *);

void intersection(double, int, int, int, unordered_map<long int, double> *);

void IBD_hash(char *, char *, unordered_map<long int, double> *);


#endif

// end of MRCA_hash.h