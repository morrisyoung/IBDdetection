// Verify_main.cpp
// function: divide all the trees into several working chunks, then initialize them into threads

// note:
// 1. the result file is named as "result_xxx" where xxx is your tree file
// 2. if the input src is stdin, then we can only use the sequential version

// this is the verification-all algorithm.
// this project includes the following files:
// Global.h
// Verify_main.cpp Package.h
// Verify_chunk.cpp Verify_chunk.h
// Verify_block.cpp Verify_block.h
// Verify_lca.cpp Verify_lca.h
// MRCA_hash.cpp MRCA_hash.h
// Parser.cpp Parser.h
// List.cpp List.h
// Prepare.cpp Prepare.h
// RMQmode.cpp RMQmode.h
// Basic.cpp Basic.h


// as the LCA algorithm should be right, so if there are bugs in the testing, then it should be from the MRCA_hash algorithm -> use the
//  naive_hash algorithm instead to substitute


// special notes:
// as it's kind of hard to deal with the multi-threading (because if the boundary is not dealt with gracefully, the practical running time will be terrible), so
// we don't use multi-threading for this version of implementation


#include <iostream>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unordered_map>
#include <pthread.h>
#include "MRCA_hash.h"
#include "Global.h"
#include "Parser.h"
#include "RMQmode.h"
#include "Verify_lca.h"
#include "Basic.h"
#include "Package.h"
#include "Verify_chunk.h"
#include <vector>
#include "Prepare.h"


//========================== global variable definition ==========================
// the following are the default value of all the global variables
double PRECISION;
double NUMERIC;
double TOLERANCE = 0.01;
long int SAMPLE;
long int TREELEN;
int THREADS = 1;                // for this implementation, the THREADS will always be 1, because we don't want to use ulti-threads for this
long int CUTOFF = 1000000;            // FILL
long int FILTER;
long int DISCRETIZATION = 0;
long int CLENGTH = 100000000;       // FILL
int FORMAT = 0;
int INPUTSRC = 0;

// about multi-threading
pthread_t thread[20];
pthread_mutex_t mut_read;
pthread_mutex_t mut_write;
//vector<block_boundary_package> boundary_block; // we don't use boundary blocks any more

// output filehandle
FILE * file_out;

// input filename
char filename[50];
char filename_out[50];

// RMQmode related (LCA preparation)
int BLOCK_NUM;
int BLOCK_SIZE;
int BLOCK_SIZE_LAST;
unordered_map<string, unordered_map<string, int>> MODE_BLOCK;
unordered_map<string, unordered_map<string, int>> MODE_BLOCK_LAST;



//DEBUG
long int TEST = 0;   // test the number of trees in discretization case
pthread_mutex_t mut_count;


//================================================================================


using namespace std;


//================================= the entrance to the program ====================================
void input_error()
{
    printf("The parameters you entered are not in right format. Please keep the following format:\n");
    printf("./excutable -f TREE_FILE -F FORMAT -t TYPE -m CUTOFF -e EPSILON -d DISCRETIZATION -l LENGTH_OF_CHROMOSOME\n");
    printf("The default values are (if there is):\n");
    printf("FORMAT: %d (tree format; 0 means Nexus and 1 means Newick)\n", FORMAT);
    printf("TYPE: %d (input source; 0 means tree file and 1 means stdin)\n", INPUTSRC);
    printf("CUTOFF: %ld\n", CUTOFF);
    printf("EPSILON: %f\n", TOLERANCE);
    printf("DISCRETIZATION: %ld\n", DISCRETIZATION);
    printf("LENGTH_OF_CHROMOSOME: %ld\n", CLENGTH);
}


int main(int argc, char * argv[])
{
    //============================================================================================
    // set all the global parameters when entering the program
    // if there is a need, we changed the following variables: (according to the input parameters)
    // some of them are necessary to initialize
    // TOLERANCE; THREADS; CUTOFF; DISCRETIZATION; CLENGTH; FORMAT; INPUTSRC; filename[50]
    // something necessary: filename; filename_out

    // command line format: ./IBDReport -f TREE_FILE -F FORMAT -t TYPE -m CUTOFF -e EPSILON -d DISCRETIZATION -l LENGTH_OF_CHROMOSOME
    int i;
    if(argc % 2 == 0 || argc == 1)
    {
        input_error();
        return(-1);
    }
    else
    {
        int count = 1;
        while(count < argc)
        {
            if(argv[count][0] != '-')
            {
                input_error();
                return(-1);
            }
            else
            {
                switch(argv[count][1])
                {
                    case 'f':
                    {
                        count++;
                        i = 0;
                        while((filename[i]=argv[count][i]) != '\0')
                            i++;
                        break;
                    }
                    case 'o':
                    {
                        count++;
                        i = 0;
                        while((filename_out[i]=argv[count][i]) != '\0')
                            i++;
                        break;
                    }
                    case 'F':
                    {
                        count++;
                        FORMAT = string_int(argv[count]);
                        break;
                    }
                    case 't':
                    {
                        count++;
                        INPUTSRC = string_int(argv[count]);
                        break;
                    }
                    case 'm':
                    {
                        count++;
                        CUTOFF = string_long(argv[count]);
                        break;
                    }
                    case 'e':
                    {
                        count++;
                        TOLERANCE = string_double(argv[count]);
                        break;
                    }
                    case 'd':
                    {
                        count++;
                        DISCRETIZATION = string_long(argv[count]);
                        break;
                    }
                    case 'l':
                    {
                        count++;
                        CLENGTH = string_long(argv[count]);
                        break;
                    }
                    default:
                        input_error();
                        return(-1);
                }
                count++;
            }
        }
    }

    // the output filehandle
    //char buffer[50];
    //sprintf(buffer, "%s%s", "result_verify_", filename);
    file_out = fopen(filename_out, "w");

    // at last we run the prepare()
    prepare();
    //============================================================================================


    //=============================== program sumarization ==============================
    printf("The following are your running parameters:\n");
    printf("========================================================================\n");
    cout << "epsilon(tolerance for tMRCA comparison): " << TOLERANCE << endl;
    cout << "sample size(automatically detected from the trees): " << SAMPLE << endl;
    cout << "number of working threads: " << THREADS << endl;
    cout << "cutoff value for true IBD segment: " << CUTOFF << endl;
    cout << "filter value for IBD candidates(basically half of cutoff value): " << FILTER << endl;
    cout << "discretization value(sample trees every xxx bp): " << DISCRETIZATION << endl;
    cout << "chromosome length: " << CLENGTH << endl;
    printf("tree format: %s\n", FORMAT?"Newick":"Nexus");
    printf("input source: %s\n", INPUTSRC?"stdin":"tree file");
    if(!INPUTSRC)
    {
        printf("tree file name: %s\n", filename);
    }
    printf("result saved as: %s\n", filename_out);
    printf("========================================================================\n");
    printf("Now the program begins...\n");


    //=========== normal work begins from here: sequential or multi-threaded =============
    //================== timing ===================
    struct timeval start;
    struct timeval end;
    double diff;
    gettimeofday(&start, NULL);


    if(THREADS == 1)
    {
        // initialize the only working threads (not a new thread from this one)
        // using filehandle0 to process all the trees; sequentially

        //================== allocate the global space =================
        chunk_package package[THREADS];  // we put all the parameters a working thread will need into the package
        for(int i = 0; i < THREADS; i++)
        {
            package[i].seq = i+1;
            // later for the start;
            package[i].start_exp = (long int)(CLENGTH*i/THREADS);
            package[i].end_exp = (long int)(CLENGTH*(i+1)/THREADS);
            package[i].table = (long int *)malloc(sizeof(long int) * SAMPLE * SAMPLE);
        }

        // set the package for thread #0, but don't initialize that
        package[0].start = 0;

        //=================== start the first (and the only) chunk here ====================
        chunk_level((void *)&package[0]);

        // we don't need to extract
        //chunk_merge(package, 0);

        cout << "The sequential work ends here!" << endl;

        //DEBUG
        printf("The total processed trees are %ld\n", TEST);

        //=================== free all the global space ====================
        for(int i = 0; i < THREADS; i++)
        {
            free(package[i].table);
        }
    }
    else    // this won't be used in this implementation
    {
        //================== initialize the locker and ... =====================
        pthread_mutex_init(&mut_read, NULL);
        pthread_mutex_init(&mut_write, NULL);
        memset(&thread, 0, sizeof(thread));
        chunk_package package[THREADS];  // we put all the parameters a working thread will need into the package

        //================== allocate the global space =================
        for(int i = 0; i < THREADS; i++)
        {
            package[i].seq = i+1;
            // later for the start;
            package[i].start_exp = (long int)(CLENGTH*i/THREADS);
            package[i].end_exp = (long int)(CLENGTH*(i+1)/THREADS);
            package[i].table = (long int *)malloc(sizeof(long int) * SAMPLE * SAMPLE);
        }

        // set the package for thread #0, but don't initialize that
        package[0].start = 0;

        //===================== new design: immediately open all the threads ====================
        // we don't perform further works here; we just initialize several working threads
        int ret;
        for(int i = 1; i < THREADS; i++)
        {
            if((ret = pthread_create(&thread[i], NULL, chunk_level, (void *)&package[i])) != 0)
                printf("separate working thread for chunk #%d is not successfully initialized!\n", i+1);
            else
                printf("separate working thread for chunk #%d is successfully initialized!\n", i+1);
        }

        //=================== start the first chunk here ====================
        printf("Now the program works for all the chunks simutaneously...\n");
        chunk_level((void *)&package[0]);

        //============= waiting for the threads to terminate, then combine ===========
        int i = 1;
        while(i < THREADS)
        {
            pthread_join(thread[i], NULL);
            chunk_merge(package, i);
            i++;
        }

        cout << "All the working threads are finished by now!! Program terminate..." << endl;

        //=================== free all the global space ====================
        for(int i = 0; i < THREADS; i++)
        {
            free(package[i].table);
        }
    }

    //=================== close all the filehandles ====================
    fclose(file_out);

    //================== timing ===================
    gettimeofday(&end, NULL);
    diff = (double)(end.tv_sec-start.tv_sec)+ (double)(end.tv_usec-start.tv_usec)/1000000;
    printf("Time used totally is %f seconds.\n", diff);

    return 0;
}

// end of Verify_main.cpp