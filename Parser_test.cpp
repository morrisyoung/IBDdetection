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
#include "Basic.h"
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


    // tree with 20 samples in
    //char tree1[] = "    tree NumGen_tree_1_1_pos_0 = [&U] ((((12.1:25.088, 3.1:25.088):425.908, ((20.1:108.422, (16.1:3.251, 2.1:3.251):105.170):26.295, 17.1:134.717):316.279):517.064, ((6.1:158.176, 8.1:158.176):301.232, 18.1:459.408):508.651):11.239, ((((5.1:8.879, 7.1:8.879):126.897, 14.1:135.776):156.553, 9.1:292.329):2.762, ((((1.1:65.607, 4.1:65.607):33.919, (10.1:6.591, 11.1:6.591):92.936):172.922, (13.1:14.976, 19.1:14.976):257.472):8.811, 15.1:281.259):13.833):684.207);";



    //================================== test the tMRCA_find function ===================================
    double * table1 = (double *)malloc(sizeof(double) * SAMPLE * SAMPLE);
    double coordinate1;
    coordinate1 = parser(tree1, table1);

    long int i, j, name;
    double tMRCA1, tMRCA2;
    cout << "begin testing:" << endl;
    for(i = 1; i <= SAMPLE; i++)
    {
        if(i%100 == 0)printf("%ld\n", i);
        for(j = i+1; j <= SAMPLE; j++)
        {
            tMRCA1 = tMRCA_find(tree1, i, j);
            name = (i - 1) * SAMPLE + j - 1;
            tMRCA2 = table1[name];
            if(tMRCA1 - tMRCA2 > TOLERANCE || tMRCA1 - tMRCA2 < -TOLERANCE)
            {
                cout << "bing!" << i << " vs " << j << " : " << tMRCA1 << " and " << tMRCA2 << endl;
            }
        }
    }
    cout << "tesing done!" << endl;
    //===================================================================================================


    /*
    // testing all the parsers: (actually the nlogn and n^2 algorithms for the unchanged pair detection)
    //================================= function module and its timing =================================
    printf("Now is the nlogn algorithm.\n");
    struct timeval start;
    struct timeval end;
    double diff;
    gettimeofday(&start, NULL);

    unordered_map<long int, int> result;

    IBD_hash(tree1, tree2, &result);

    gettimeofday(&end, NULL);

    diff = (double)(end.tv_sec-start.tv_sec)+ (double)(end.tv_usec-start.tv_usec)/1000000;
    printf("Time used totally is %f seconds.\n", diff);
    //==================================================================================================


    //======================================= testing the parser =======================================
    printf("Now is the n^2 algorithm:\n");
    gettimeofday(&start, NULL);
    double * table1 = (double *)malloc(sizeof(double) * SAMPLE * SAMPLE);
    double coordinate1;
    coordinate1 = parser(tree1, table1);

    double * table2 = (double *)malloc(sizeof(double) * SAMPLE * SAMPLE);
    double coordinate2;
    coordinate2 = parser(tree2, table2);

    //free(table1);
    //free(table2);

    gettimeofday(&end, NULL);

    diff = (double)(end.tv_sec-start.tv_sec)+ (double)(end.tv_usec-start.tv_usec)/1000000;
    printf("Time used totally is %f seconds.\n", diff);
    //==================================================================================================


    //============================ compare the results from two methods ================================
    cout << "there is " << result.size() << " in the result hash table." << endl;

    printf("Now compare the result:\n");

    unordered_map<long int, int> temp;

    long int key;
    for(auto itr = result.begin(); itr != result.end(); itr ++)
    {
        key = (*itr).first;
        if(table1[key] - table2[key] > TOLERANCE || table1[key] - table2[key] < -TOLERANCE)
            temp.emplace(key, 1);
    }

    cout << "haha here is the result:" << endl;
    for(auto itr = temp.begin(); itr != temp.end(); itr ++)
    {
        cout << (*itr).first << endl;
    }

    printf("Now is another comparison:\n");
    long int i, j;
    long int name;
    for(i = 1; i <= SAMPLE; i++)
        for(j = i+1; j <= SAMPLE; j++)
        {
            name = (i - 1) * SAMPLE + j - 1;
            if((table1[name] - table2[name] <= TOLERANCE && table1[name] - table2[name] >= -TOLERANCE))
            {
                std::unordered_map<long int, int>::const_iterator got = result.find(name);
                if(got == result.end())
                {
                    cout << "bing!! " << i << " vs " << j << " " << table1[name] << " vs " << table2[name] << endl;
                }
                else
                    result[name] = 0;
            }
        }

    printf("hehe here is the result:\n");
    for(auto itr = result.begin(); itr != result.end(); itr ++)
    {
        if((*itr).second == 1)
            cout << (*itr).first << endl;
    }
    //==================================================================================================
    */


    fclose(file);
    return 0;
}