#include <iostream>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include "MRCA_hash.h"
#include <unordered_map>
#include "Global.h"
#include "Parser.h"
#include "RMQmode.h"
#include "LCA.h"
#include "basic.h"
#include <string>


double PRECISION = 0;
double NUMERIC = 0;
double TOLERANCE = 0;
long int SAMPLE = 0;
long int TREELEN = 0;
long int CUTOFF;
pthread_mutex_t mut_write;  // mutex used by all the threads; used in saving the IBD segments into file


// the filehandle for the output file; global, to permit all the threads access it; using the mut above to protect
FILE * file_out;


using namespace std;


int main()
{
    // set all the global parameters when entering the program
    PRECISION = 0.1;    // although it seems that we can still move back, but things may not be that optimistic
    NUMERIC = 0.00001;  // this is for hashing safe; this problem results from the float number precision issue
    TOLERANCE = 0.01;
    SAMPLE = 2000;
    TREELEN = 200000;

    // for testing the n^2 algorithm and the nlogn algorithm, change the SAMPLE value and the tree file name

    char filename[] = "test_2000.trees";
    //char filename[] = "tree_5000.trees";
    //char filename[] = "twotrees.txt";
    FILE * file = fopen(filename, "r");

    char tree1[TREELEN];
    //char tree2[TREELEN];

    // get the first tree
    fgets(tree1, TREELEN, file);
    while(!tree_judge(tree1))
        fgets(tree1, TREELEN, file);

    // get the second tree
    fgets(tree2, TREELEN, file);
    while(get_coordinate(tree2) < 500000)
        fgets(tree2, TREELEN, file);


    printf("start and end coordinate:\n");
    cout << get_coordinate(tree1) << " " << get_coordinate(tree2) << endl;
    cout << "now enter the operation loop..." << endl;


    //============================ testing the nlogn algorithm ================================
    struct timeval start;
    struct timeval end;
    double diff;
    double time1 = 0;
    double time2 = 0;

    // one testing loop
    //===============================================
    // processing with timing; 20 times average
    for(int i = 1; i <= 20; i++)
    {
        unordered_map<long int, double> result1;
        unordered_map<long int, double> result2;

        gettimeofday(&start, NULL);
        unchanged_pairs_hash(tree1, tree2, &result1);
        gettimeofday(&end, NULL);
        diff = (double)(end.tv_sec-start.tv_sec)+ (double)(end.tv_usec-start.tv_usec)/1000000;
        printf("Time used totally is %f seconds (n^2 algorithm).\n", diff);
        time1 += diff;
        printf("Time used totally (overall) is %f seconds (n^2 algorithm).\n", time1/i);

        // processing with timing
        gettimeofday(&start, NULL);
        IBD_hash(tree1, tree2, &result2);
        gettimeofday(&end, NULL);
        diff = (double)(end.tv_sec-start.tv_sec)+ (double)(end.tv_usec-start.tv_usec)/1000000;
        printf("Time used totally is %f seconds (nlogn algorithm).\n", diff);
        time2 += diff;
        printf("Time used totally (overall) is %f seconds (nlogn algorithm).\n", time2/i);


        printf("the size of the two results: (n^2 and nlogn)\n");
        cout << result1.size() << " " << result2.size() << endl;

        // test the correctness
        for(auto itr = result2.begin(); itr != result2.end(); itr ++)
        {
            std::unordered_map<long int, double>::const_iterator got = result1.find((*itr).first);
            if(got != result1.end())
            {
                result1.erase((*itr).first);
            }
        }

        cout << "the # of different elements: " << result1.size() << endl;

        // who they are:
        long int sample1, sample2, name;
        for(auto itr = result1.begin(); itr != result1.end(); itr ++)
        {
            cout << (*itr).first;
            name = (*itr).first + 1;
            if(name % SAMPLE == 0)
            {
                sample1 = name / SAMPLE;
                sample2 = SAMPLE;
            }
            else
            {
                sample1 = name / SAMPLE + 1;
                sample2 = name % SAMPLE;
            }
            printf("(%ld %ld) ", sample1, sample2);
        }
        cout << endl;
    }


    /*
    // another testing loop
    //===============================================
    // get the second tree
    char tree3[TREELEN];
    fgets(tree3, TREELEN, file);
    while(get_coordinate(tree3) < 1000000)
        fgets(tree3, TREELEN, file);

    unordered_map<long int, double> result3;
    unordered_map<long int, double> result4;

    // processing with timing
    gettimeofday(&start, NULL);
    unchanged_pairs_hash(tree2, tree3, &result3);
    gettimeofday(&end, NULL);
    diff = (double)(end.tv_sec-start.tv_sec)+ (double)(end.tv_usec-start.tv_usec)/1000000;
    printf("Time used totally is %f seconds (n^2 algorithm).\n", diff);

    // processing with timing
    gettimeofday(&start, NULL);
    IBD_hash(tree2, tree3, &result4);
    gettimeofday(&end, NULL);
    diff = (double)(end.tv_sec-start.tv_sec)+ (double)(end.tv_usec-start.tv_usec)/1000000;
    printf("Time used totally is %f seconds (nlogn algorithm).\n", diff);

    printf("the size of the two results: (n^2 and nlogn)\n");
    cout << result3.size() << " " << result4.size() << endl;


    for(auto itr = result4.begin(); itr != result4.end(); itr ++)
    {
        std::unordered_map<long int, double>::const_iterator got = result3.find((*itr).first);
        if(got != result3.end())
        {
            result3.erase((*itr).first);
        }
    }

    cout << "the difference: " << result3.size() << endl;
    //===============================================
    */

    fclose(file);

    return 0;
}