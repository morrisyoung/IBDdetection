#include <iostream>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unordered_map>
#include "Global.h"
#include "LCA.h"
#include "Basic.h"
#include <string>
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */
#include "Prepare.h"
#include "Parser.h"


//========================== global variable definition ==========================
// the following are the default value of all the global variables
double PRECISION;
double NUMERIC;
double TOLERANCE = 0.01;
long int SAMPLE;
long int TREELEN;
int THREADS = 1;
long int CUTOFF = 0;
long int FILTER;
long int DISCRETIZATION = 0;
long int CLENGTH = 100000000;
int FORMAT = 0;
int INPUTSRC = 0;

// about multi-threading
pthread_t thread[10];
pthread_mutex_t mut_read;
pthread_mutex_t mut_write;
vector<block_boundary_package> boundary_block;

// output filehandle
FILE * file_out;

// input filename
char filename[50];

int BLOCK_NUM;
int BLOCK_SIZE;
int BLOCK_SIZE_LAST;
unordered_map<string, unordered_map<string, int>> MODE_BLOCK;
unordered_map<string, unordered_map<string, int>> MODE_BLOCK_LAST;
//================================================================================


using namespace std;


int main()
{
    //============================================================================================
    // set all the global parameters when entering the program
    // if there is a need, we changed the following variables: (according to the input parameters)
    // some of them are necessary to initialize
    // TOLERANCE; THREADS; CUTOFF; DISCRETIZATION; CLENGTH; FORMAT; INPUTSRC; filename[50]
    //
    //
    //
    //
    sprintf(filename, "%s", "test_2000.trees");

    // perform prepare() at last
    prepare();
    //============================================================================================



    //=================================== test the LCA implementation ===================================
    FILE * file = fopen(filename, "r");
    char tree[TREELEN];
    fgets(tree, TREELEN, file);
    while(!tree_judge(tree))
        fgets(tree, TREELEN, file);


    double * table1 = (double *)malloc(sizeof(double) * SAMPLE * SAMPLE);

    // timing
    struct timeval start;
    struct timeval end;
    double diff;
    gettimeofday(&start, NULL);

    // LCA pre-processing for every tree (we should perform the preprocess whenever there is a new tree)
    for(int count = 0; count < 100; count ++)
        LCA_preprocess(tree);

    gettimeofday(&end, NULL);
    // timing report
    diff = ((double)(end.tv_sec-start.tv_sec) * 1000 + (double)(end.tv_usec - start.tv_usec)/1000)/100;
    printf("Time used for linear processing is %f milliseconds.\n", diff);

    // another parser
    parser(tree, table1);

    // testing the results
    long int i, j, name;
    double tMRCA1, tMRCA2;
    cout << "begin testing:" << endl;
    for(i = 1; i <= SAMPLE; i++)
    {
        //if(i%100 == 0)printf("%ld\n", i);
        for(j = i+1; j <= SAMPLE; j++)
        {
            tMRCA1 = getMRCA(i, j);
            //tMRCA2 = tMRCA_find(tree1, i, j);
            name = (i - 1) * SAMPLE + j - 1;
            tMRCA2 = table1[name];
            if(tMRCA1 - tMRCA2 > TOLERANCE || tMRCA1 - tMRCA2 < -TOLERANCE)
            {
                cout << "bing!" << i << " vs " << j << " : " << tMRCA1 << " and " << tMRCA2 << endl;
            }

        }
    }
    cout << "testing done!" << endl;
    //==================================================================================================




    /*
    // plot the comparison curve (n^2 preprocessing and constant query; n preprocessing and constant query; n*nc query)
    //==================================================================================================
    // by now we have a tree1, and we can just use this tree to test


    // timing model: (we will use milliseconds 10^-3)
    //===============================================
    struct timeval start;
    struct timeval end;
    double diff;
    double * table = (double *)malloc(sizeof(double) * SAMPLE * SAMPLE);
    // query amount:
    //long int count = 0.00 * SAMPLE * SAMPLE;   // the query amount
    long int count = 5000;
    cout << "query amount: " << count << endl;
    long int itr = 0;   // used to terminate the testing
    double tMRCA;
    long int i, j, name;


    // n^2 pre-processing, constant query
    //=================================================
    gettimeofday(&start, NULL);

    parser(tree1, table);

    // further query
    itr = 0;
    for(i = 1; i <= SAMPLE; i++)
    {
        //if(i%100 == 0)printf("%ld\n", i);
        for(j = i+1; j <= SAMPLE; j++)
        {
            itr++;
            if(itr > count)
                break;
            name = (i - 1) * SAMPLE + j - 1;
            tMRCA = table[name];
        }
    }

    gettimeofday(&end, NULL);
    diff = (double)(end.tv_sec-start.tv_sec) * 1000 + (double)(end.tv_usec - start.tv_usec)/1000;
    printf("1. Time used is %f milliseconds.\n", diff);
    //=================================================

    // n pre-processing, constant query
    //=================================================
    LCA_preprocess(tree1);

    gettimeofday(&start, NULL);

    int iteration = 100;    // the averaging times
    for(int i1 = 0; i1 < iteration; i1++)
    {

        // further query
        itr = 0;
        for(i = 1; i <= SAMPLE; i++)
        {
            //if(i%100 == 0)printf("%ld\n", i);
            for(j = i+1; j <= SAMPLE; j++)
            {
                itr++;
                if(itr > count)
                    break;
                tMRCA = getMRCA(i, j);
            }
        }
    }

    gettimeofday(&end, NULL);
    diff = ((double)(end.tv_sec-start.tv_sec) * 1000+ (double)(end.tv_usec-start.tv_usec)/1000)/iteration;
    //diff = (double)(end.tv_sec-start.tv_sec) * 1000+ (double)(end.tv_usec-start.tv_usec)/1000;
    printf("2. Time used is %f milliseconds.\n", diff);
    //=================================================

    // n direct query for all the pairs
    //=================================================
    gettimeofday(&start, NULL);

    // further query
    itr = 0;
    for(i = 1; i <= SAMPLE; i++)
    {
        //if(i%100 == 0)printf("%ld\n", i);
        for(j = i+1; j <= SAMPLE; j++)
        {
            itr++;
            if(itr > count)
                break;
            tMRCA = tMRCA_find(tree1, i, j);
        }
    }

    gettimeofday(&end, NULL);
    diff = (double)(end.tv_sec-start.tv_sec) * 1000+ (double)(end.tv_usec-start.tv_usec)/1000;
    printf("3. Time used is %f milliseconds.\n", diff);
    //=================================================

    // finish testing
    //==================================================================================================
    */




    fclose(file);
    return 0;
}