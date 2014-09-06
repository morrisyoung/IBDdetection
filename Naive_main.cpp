// Naive_main.cpp
// function: the Naive_xxx are all prepared for the previous n^2 naive algorithm
// this project includes the following files:
// Naive_global.h
// Naive_main.cpp Naive_main.h
// Naive_chunk.cpp Naive_chunk.h
// Naive_stdin.cpp Naive_stdin.h
// Naive_parser.cpp Naive_parser.h
// List.cpp List.h


// Jul 24
// now this is a workable but better version of the n^2 naive algorithm's implementation
// we don't use c++11 libraries, so the compilation will have no problems even in the clusters
// we have not wrapped it up because we are only testing the algorithm now


// Aug 5
// I'm wrapping up and add the Newick parser into the program

// Aug 6:
// there are still some issues about how to judge the trees; presently, I use " tree" to judge the Nexus tree,
// and "NEWICK" to judge the Newick tree; but there may be some problems

// the difference between Nexus and Newick:
// coordinate

// special notes:
// 1. for the sample: 
// 24:0.0230624  or  24.1:0.0230624 are all possible, in which the additional "1" means the simulation
// and the sample name may start from 0 or 1 (default); no matter it's from 0 or 1, we formalize it from 1
// 2. there can be no more than 10 working threads
// 3. why we should set the CLENGTH by hand?
// in Nexus format, we need that information exactly, because we will use it to construct the last possible segments;
// in Newick format, we will use it to divide those trees;
//  if it's less than the actual value, the last thread will bear some more tasks;
//  if it's more than the actual value, some threads will have no tasks at all.


// Aug 8:
// there are some issues about the free(); I don't know wherer does these errors come from
// what I should do to test the stdin reading this afternoon:
//  1. save the trees into file at the same time, then run two versions to compare;
//  2. use the theoretical formula to predict


#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "Naive_global.h"
#include "Naive_main.h"
#include "Naive_chunk.h"
#include "Naive_parser.h"
#include "Naive_basic.h"
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */
#include "Naive_stdin.h"
    

//========================== global variable definition ==========================
// the following are the default value of all the global variables
double PRECISION;
double NUMERIC;
double TOLERANCE = -1;
long int SAMPLE;
long int TREELEN;
int THREADS = 1;
long int CUTOFF = -1;
long int DISCRETIZATION = 0;
long int CLENGTH = 0;
int FORMAT = -1;
int INPUTSRC = -1;

// about multi-threading
pthread_t thread[20];
pthread_mutex_t mut_read;
pthread_mutex_t mut_write;
//vector<block_boundary_package> boundary_block;

// output filehandle
FILE * file_out;

// input filename
char filename[50];
int filename_yes = 0;
char filename_out[50];
int filename_out_yes = 0;

int FROMONE;

char * tree_stdin;

long int stdin_buffer = 300000;


//DEBUG
long int TEST = 0;
pthread_mutex_t mut_count;  // to protect the above variable


//================================================================================


//================================= the entrance to the program ====================================
void input_error()
{
    printf("The parameters you entered are not in right format. Please keep the following format:\n");
    printf("./excutable -f TREE_FILE -o OUTPUT_FILE -F FORMAT -t TYPE -m CUTOFF -e EPSILON -d DISCRETIZATION -l LENGTH_OF_CHROMOSOME -T THREADS\n");
    printf("The default values are (if there is):\n");
    printf("FORMAT: %d (tree format; 0 means Nexus and 1 means Newick)\n", FORMAT);
    printf("TYPE: %d (input source; 0 means tree file and 1 means stdin)\n", INPUTSRC);
    printf("CUTOFF: %ld\n", CUTOFF);
    printf("EPSILON: %f\n", TOLERANCE);
    printf("DISCRETIZATION: %ld\n", DISCRETIZATION);
    printf("LENGTH_OF_CHROMOSOME: %ld\n", CLENGTH);
    printf("THREADS: %d\n", THREADS);
}


int main(int argc, char * argv[])
{
    //============================================================================================
    // set all the global parameters when entering the program
    // if there is a need, we changed the following variables: (according to the input parameters)
    // some of them are necessary to initialize (CLENGTH, INPUTSRC&&filename, FORMAT, INPUTSRC)
    // TOLERANCE; THREADS; CUTOFF; DISCRETIZATION; CLENGTH; FORMAT; INPUTSRC; filename[50]

    // command line format: ./IBDReport -f TREE_FILE -F FORMAT -t TYPE -m CUTOFF -e EPSILON -d DISCRETIZATION -l LENGTH_OF_CHROMOSOME -T THREADS
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
                        filename_yes = 1;
                        break;
                    }
                    case 'o':
                    {
                        count++;
                        i = 0;
                        while((filename_out[i]=argv[count][i]) != '\0')
                            i++;
                        filename_out_yes = 1;
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
                    case 'T':
                    {
                        count++;
                        THREADS = string_int(argv[count]);
                        break;
                    }
                    default:
                        input_error();
                        return(-1);
                }
                count++;
            }
        }
        // some error checks ...
        if(CLENGTH == 0)
        {
            printf("You must provide the chromosome length (-l) parameter...\n");
            return(-1);
        }
        if(INPUTSRC == -1)
        {
            printf("You must specify the input source (-t: 0 means tree file, 1 means stdin)...\n");
            return(-1);
        }
        else
        {
            if(!(INPUTSRC == 0 || INPUTSRC == 1))
            {
                printf("Please choose from 0 and 1 for the input source (-t: 0 means tree file, 1 means stdin)...\n");
                return(-1);
            }
        }
        if(INPUTSRC == 0 && filename_yes == 0)
        {
            printf("You choose to read trees from file (or default choice) (-t is 0), but you don't provide the filename...\n");
            return(-1);
        }
        if(INPUTSRC == 0 && filename_yes == 1)
        {
            FILE * fp;
            if((fp = fopen(filename, "r")) == NULL)
            {
                printf("Errors when opening the tree file... Does it exist?\n");
                return(-1);
            }
        }
        if(FORMAT == -1)
        {
            printf("You must provide the format (-F: 0 means Nexus, 1 means Newick) of the trees...\n");
            return(-1);
        }
        else
        {
            if(!(FORMAT == 0 || FORMAT == 1))
            {
                printf("Please choose from 0 and 1 for the format (-F: 0 means Nexus, 1 means Newick)...\n");
                return(-1);
            }
        }
        if(TOLERANCE == -1)
        {
            printf("You must provide the EPSILON (-e) for the tMRCA.\n");
            printf("We generally recommend you choose the last but two digit as the tolerance.\n");
            return(-1);
        }
        if(CUTOFF == -1)
        {
            printf("You must provide the cutoff value (-m) for the IBD segment.\n");
            return(-1);
        }
        if(THREADS > 10)
        {
            printf("Hey I have told you in README that please don't try THREADS larger than 10...\n");
            return(-1);
        }
        if(INPUTSRC == 1 && THREADS != 1)
        {
            printf("You choose to read the trees from stdin, but you want to work multi-threading, which is impossible...\nPlease use one thread when reading from stdin\n");
            return(-1);
        }
        if(filename_out_yes == 0)
        {
            printf("Please specify the output file name (-o)...\n");
            return(-1);
        }
    }

    // the output filehandle
    file_out = fopen(filename_out, "w");
    // fill other necessary variables
    TREELEN = 200000;
    char tree[TREELEN];
    if(INPUTSRC == 0)   // read from tree file
    {
        FILE * file_in = fopen(filename, "r");
        fgets(tree, TREELEN, file_in);
        while(!tree_judge(tree))
            fgets(tree, TREELEN, file_in);
        fclose(file_in);
    }
    else    // read from stdin
    {
        char c;
        tree_stdin = (char *)malloc(sizeof(char) * stdin_buffer);

        tree_stdin[0] = '\0';

        // FOR THE STDIN:
        // we read one line (ended by '\n') one time; then drop that '\n' and continue
        // there shuld not be an empty line if the trees are not ended, otherwise we will lose the following trees
        fscanf(stdin, "%[^\n]", tree_stdin);
        fscanf(stdin, "%c", &c);
        while(!tree_judge(tree_stdin))
        {
            tree_stdin[0] = '\0';
            fscanf(stdin, "%[^\n]", tree_stdin);
            fscanf(stdin, "%c", &c);
        }
    }

    if(INPUTSRC == 0)
    {
        FROMONE = 1;
        SAMPLE = parser_sample(tree);
        // we don't fear there may be a very long line, because we will limit the line length read from the filehandle
        // but this is not the case for lines from stdin; so we have a stdin_buffer
        TREELEN = (long int)(strlen(tree) * 1.2);  // I think coefficient 1.2 is enough for a tree
    }
    else
    {
        FROMONE = 1;
        SAMPLE = parser_sample(tree_stdin);
    }
    // judge the format: (FORMAT) --> Actually I don't even know the difference between this two kinds of trees
    //
    //
    //
    //=============================== program sumarization ==============================
    printf("The following are your running parameters:\n");
    printf("========================================================================\n");
    printf("epsilon(tolerance for tMRCA comparison): %f\n", TOLERANCE);
    printf("sample size: %ld\n", SAMPLE);
    printf("number of working threads: %d\n", THREADS);
    printf("cutoff value for true IBD segment: %ld\n", CUTOFF);
    printf("discretization value(sample trees every xxx bp): %ld\n", DISCRETIZATION);
    printf("chromosome length: %ld\n", CLENGTH);
    printf("sample name is from: %d\n", FROMONE);
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


    if(INPUTSRC == 1)   // from stdin
    {
        stdin_process();
        free(tree_stdin);
    }
    else    // from tree file
    {
        if(THREADS == 1)
        {
            // initialize the only working threads (not a new thread from this one)

            //================== allocate the global space for chunks =================
            chunk_package package[THREADS];  // we put all the parameters a working thread will need into the package
            for(int i = 0; i < THREADS; i++)
            {
                package[i].seq = i+1;
                // later for the start;
                package[i].start_exp = (long int)(CLENGTH*i/THREADS);
                package[i].end_exp = (long int)(CLENGTH*(i+1)/THREADS);
                package[i].table_coordinate = (long int *)malloc(sizeof(long int) * SAMPLE * SAMPLE);
                package[i].table_tMRCA = (double *)malloc(sizeof(double) * SAMPLE * SAMPLE);
            }
            // set the package for thread #0, but don't initialize that
            package[0].start = 0;

            //=================== start the first (and the only here) chunk here ====================
            chunk_level((void *)&package[0]);

            // we should also merge in this case, because we have not yet provided the potential first
            // segment and the last segment for one specific pair
            chunk_merge(package, 0);

            printf("The end bp of the chromosome: %ld\n", CLENGTH);

            printf("The sequential work ends here!\n");



            //DEBUG
            printf("The total processed trees are %ld\n", TEST+1);



            //=================== free all the global space for chunks ====================
            for(int i = 0; i < THREADS; i++)
            {
                free(package[i].table_coordinate);
                free(package[i].table_tMRCA);
            }
        }
        else
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
                package[i].table_coordinate = (long int *)malloc(sizeof(long int) * SAMPLE * SAMPLE);
                package[i].table_tMRCA = (double *)malloc(sizeof(double) * SAMPLE * SAMPLE);
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
            chunk_merge(package, 0);
            int i = 1;
            while(i < THREADS)
            {
                pthread_join(thread[i], NULL);
                chunk_merge(package, i);
                i++;
            }

            printf("The end bp of the chromosome: %ld\n", CLENGTH);

            printf("All the working threads finish now!! Program terminates ...\n");



            //DEBUG
            printf("The total processed trees are %ld\n", TEST+1);


            //=================== free all the global space ====================
            for(int i = 0; i < THREADS; i++)
            {
                free(package[i].table_coordinate);
                free(package[i].table_tMRCA);
            }
        }
    }

    //================== timing ===================
    gettimeofday(&end, NULL);
    diff = (double)(end.tv_sec-start.tv_sec)+ (double)(end.tv_usec-start.tv_usec)/1000000;
    printf("Time used totally is %f seconds.\n", diff);

    // release the output filehandle for security
    fclose(file_out);

    return 0;
}