// List.h
// function: construct the list structure in C/C++; actually this is the linked list

#ifndef LIST_H
#define LIST_H


using namespace std;


// there are basically two types of lists:
// 1. [[1,2,4], [3,7,6], [5], [9,8,0]]; two-dimensional list, storing all the classified samples
// 2. [123.45, 123.46, 123.47, 123.48]; one-dimensional list, storing all the tMRCAs correspondingly
typedef struct Node_int  // to store the classified samples
{
	int element;  // "int" is enough for, say, 5000 chromosomes
	struct Node_int * next;
}Node_int;

typedef struct List_int  // list of int
{
	Node_int * start;
	Node_int * end;
	int length;
}List_int;

typedef struct Node_list  // for 2-dimensional list, say, l1 here, each element pointing to List_int
{
	List_int * start;
	struct Node_list * next;
}Node_list;

typedef struct List_list  // list of list
{
	Node_list * start;
	Node_list * end;
	int length;
}List_list;

typedef struct Node_double  // to store the tMRCA of each group of samples
{
	double element;
	struct Node_double * next;
}Node_double;

typedef struct List_double  // list of double
{
	Node_double * start;
	Node_double * end;
	int length;
}List_double;


List_list * createList_list();

List_int * createList_int();

List_double * createList_double();

void appendList_list(List_list *);

void appendList_int(List_list *, long int);

void appendList_double(List_double *, double);

void extendList_list(List_list *);

void extendList_double(List_double *, double);

void cleanList(List_list *, List_double *);

#endif

// end of List.h