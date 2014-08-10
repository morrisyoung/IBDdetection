//  Prepare.cpp
// function: fill all the necessary global variables here

#include <stdio.h>
#include <iostream>
#include "Global.h"
#include "Basic.h"
#include <string.h>
#include "Parser.h"
#include "RMQmode.h"


using namespace std;


void prepare()
{
	FILE * file = fopen(filename, "r");
	TREELEN = 200000;
	char tree[TREELEN];
	fgets(tree, TREELEN, file);
	while(!tree_judge(tree))
		fgets(tree, TREELEN, file);

	PRECISION = 0.1;    // although it seems that we can still move back, but things may not be that optimistic
	NUMERIC = 0.00001;  // this is for hashing safe; this problem results from the float number precision issue
	FILTER = CUTOFF/2;
	if(FILTER == 0)
		FILTER = 1;
	SAMPLE = parser_sample(tree);
	TREELEN = (long int)(strlen(tree) * 1.2);  // I think coefficient 1.2 is enough for a tree
	// judge the format: (FORMAT)
	//
	//
	//

	RMQmode();

	fclose(file);
	return;
}

// end of Prepare.cpp