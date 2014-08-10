// Basic.h
// function: some basic functions which will be used by other parts of the whole program

#ifndef BASIC_H
#define BASIC_H

#include <string>

using namespace std;


int string_int(char *);

long int string_long(char * string);

double string_double(char * string);

void print_MRCA(double *, long int);

double partial_tMRCA(double, double);

void intdashint(int, int, string *);

int tree_judge(char *);

long int get_coordinate(char *);

void IBDreport(long int, long int, long int, long int);

#endif

// end of Basic.h