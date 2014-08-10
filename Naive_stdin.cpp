// Naive_stdin.cpp
// process the trees from stdin
// input: tree_stdin (havintg the first tree inside); stdin
// output: the results, directly saving them into the file_out
// note: if we read trees from stdin, then there can be and can only be at most one working thread

#include "Naive_stdin.h"
#include "Naive_global.h"
#include "Naive_parser.h"
#include "Naive_basic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void stdin_process()
{
    // the following should be HERE other than outside because all the threads will share that variable:
    // the basic idea of adding Newick format is that, everytime we want to parse a tree, we prepare the following
    // variable as coordinate_current
    long int coordinate_current = 0;
    long int seg_len_last = 0;  // used for storing the cached segment length
    long int coordinate;
	char c;

    coordinate = get_coordinate(tree_stdin);
    //==== behave accordingly for coordinate_current ====
    if(!FORMAT) // Nexus
    {
        coordinate_current = coordinate;
    }
    else    // Newick
    {
        coordinate_current = coordinate_current + seg_len_last;
        seg_len_last = coordinate;
    }

    //============= start the work ==============
    printf("Sequential work for stdin starts from here...\n");

    //====================== preparation ========================
    long int * table_coordinate = (long int *)malloc(sizeof(long int) * SAMPLE * SAMPLE);
    double * table_tMRCA = (double *)malloc(sizeof(double) * SAMPLE * SAMPLE);

    // the following: solved
    // --> there are some issues about the malloc here
    // the allocated memory will change with the process of the program
    // so we here use the STACK memory; but there may be some memory overflow issues if SAMPLE is too large

    //long int table_coordinate[SAMPLE * SAMPLE];
    //double table_tMRCA[SAMPLE * SAMPLE];

    long int name;
    for(long int i = 1; i <= SAMPLE; i++)
    {
        for(long int j = i+1; j <= SAMPLE; j++)
        {
            name = (i - 1) * SAMPLE + j - 1;
            table_coordinate[name] = 0;
        }
    }
    parser(tree_stdin, table_tMRCA);


    //===================== here we go: the LOOP ======================
    // we don't test the present first tree
    // get a new tree and its coordinate
	tree_stdin[0] = '\0';
    fscanf(stdin, "%[^\n]", tree_stdin);
    fscanf(stdin, "%c", &c);
    while(!tree_judge(tree_stdin))
    {
        tree_stdin[0] = '\0';
        fscanf(stdin, "%[^\n]", tree_stdin);
        fscanf(stdin, "%c", &c);
    }
    coordinate = get_coordinate(tree_stdin);

    //==== behave accordingly for coordinate_current ====
    if(!FORMAT) // Nexus
    {
        coordinate_current = coordinate;
    }
    else    // Newick
    {
        coordinate_current = coordinate_current + seg_len_last;
        seg_len_last = coordinate;
    }

    long int count = 0;
    while(1)	// continue with a tree and it's coordinate
    {
        stdin_update(coordinate_current, tree_stdin, table_coordinate, table_tMRCA);
        //===== test =====
        count ++;
        if(count % 100 == 0)
            printf("program has processed %ld trees.\n", count);

        // get a tree; if there are no more trees (the end for all the trees): break
		while(strcmp(tree_stdin, ""))
	    {
	        tree_stdin[0] = '\0';
	        fscanf(stdin, "%[^\n]", tree_stdin);
	        if(!strcmp(tree_stdin, ""))continue;

	        fscanf(stdin, "%c", &c);

	        if(tree_judge(tree_stdin))break;
	    }

		if(!strcmp(tree_stdin, ""))
		{
            //==== behave accordingly for the end of the chromosome ====
            if(FORMAT) // Newick
            {
                CLENGTH = coordinate_current + seg_len_last;
            }
            break;
		}

        coordinate = get_coordinate(tree_stdin);

        //==== behave accordingly for coordinate_current ====
        if(!FORMAT) // Nexus
        {
            coordinate_current = coordinate;
        }
        else    // Newick
        {
            coordinate_current = coordinate_current + seg_len_last;
            seg_len_last = coordinate;
        }
    }

    // detect the last possible segment for each pair
	for(long int i = 1; i <= SAMPLE; i++)
		for(long int j = i+1; j <= SAMPLE; j++)
		{
			long int name = (i - 1) * SAMPLE + j - 1;
			IBDreport(i, j, table_coordinate[name], CLENGTH);
		}

    printf("Sequential work for stdin ends here...\n");

    //==== free the local heap memory ====
    free(table_coordinate);
    free(table_tMRCA);

    return;
}

// end of Naive_stdin.cpp