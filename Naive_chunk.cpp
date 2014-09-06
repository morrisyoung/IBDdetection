// Naive_chunk.cpp
// perform the chunk level works for the naive algorithm

#include "Naive_global.h"
#include "Naive_chunk.h"
#include "Naive_main.h"
#include "Naive_parser.h"
#include "Naive_basic.h"
#include <stdio.h>
#include <stdlib.h>


// the working thread for chunk_level function
void * chunk_level(void * p)
{
    // the following should be HERE other than outside because all the threads will share that variable:
    // the basic idea of adding Newick format is that, everytime we want to parse a tree, we prepare the following
    // variable as coordinate_current
    long int coordinate_current = 0;
    long int coordinate_temp;
    long int seg_len_last = 0;  // used for storing the cached segment length

    chunk_package * package = (chunk_package *)p;
    FILE * filehandle = fopen(filename, "r");
    //char tree[TREELEN];
    char * tree = (char *)malloc(sizeof(char) * TREELEN);

    if(package->seq == 1)
    {
        long int coordinate;
        while(1)
        {
            // still use a locker to protect
            pthread_mutex_lock(&mut_read);
            fgets(tree, TREELEN, filehandle);
            pthread_mutex_unlock(&mut_read);

            if(!tree_judge(tree))continue;

            coordinate = get_coordinate(tree);
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

            break;  // after we go out, we can process normally
        }
    }
    else    // we should find the start coordinate, and at the same time fill the boundary block package
    {
        //===================== find the start of this working threads =======================
        //============ detect all the following chunks and initialize them =============
        long int coordinate;

        while(1)
        {
            // still use a locker to protect
            pthread_mutex_lock(&mut_read);
            fgets(tree, TREELEN, filehandle);
            pthread_mutex_unlock(&mut_read);

            if(!tree_judge(tree))continue;

            coordinate = get_coordinate(tree);
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

            if(coordinate_current >= package->start_exp)
                break;
        }
        package->start = coordinate_current;
    }

    //======================= the works for a chunk formally starts from here =======================
    // by now, we should have:
    // tree[TREELEN]; full chunk package
    // so we can:
    // start from the present tree

    // testing
    printf("this is the working thread#%d/%d: start:%ld start_expected:%ld end_expected:%ld\n", package->seq, THREADS, package->start, package->start_exp, package->end_exp);

    //====================== preparation ========================
    long int end_exp = package->end_exp;
    long int coordinate;
    long int * table_coordinate = package->table_coordinate;
    double * table_tMRCA = package->table_tMRCA;

    //================= initialize all the tables ==================
    // algorithm for multi-chunks:
    // 1. we always don't report the first and the last segment within a chunk, but rather
    // we only report those in-chunk segment
    // 2. finally we use "0" and CLENGTH to detect all the boundary segments

    // table_coordinate:
    // if there is a -1 in, we know there is no tMRCA change for that pair
    // the "name" is used to store the first changed coordinate
    // the "SAMPLE*SAMPLE-1-name" is used to store the last changed coordinate for that pair
    long int name;
    for(long int i = 1; i <= SAMPLE; i++)
    {
        for(long int j = i+1; j <= SAMPLE; j++)
        {
            name = (i - 1) * SAMPLE + j - 1;
            table_coordinate[name] = -1;
        }
    }
    parser(tree, table_tMRCA);

    //===================== here we go: the LOOP ======================
    // we don't test the present first tree
    // get a new tree and its coordinate; but we should use the DISCRETIZATION to judge

    // Sep.5: (as the number of trees under the present algorithm is not constant, we will make some changes)
    // we will use the pre-defined threshold to find the next tree, other than find the next D farther tree


    long int THRESHOLD = coordinate_current + DISCRETIZATION;
    while(1)    // until we have the next right tree
    {
        pthread_mutex_lock(&mut_read);
        fgets(tree, TREELEN, filehandle);
        pthread_mutex_unlock(&mut_read);
        while(!tree_judge(tree))
        {
            pthread_mutex_lock(&mut_read);
            fgets(tree, TREELEN, filehandle);
            pthread_mutex_unlock(&mut_read);
        }
        coordinate = get_coordinate(tree);
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

        if(coordinate_current >= THRESHOLD)break;
    }

    // #######################################################
    // #### work from here for the in-loop DISCRETIZATION ####
    // #######################################################
    long int count = 1;
    while(coordinate_current < end_exp)
    {

        //DEBUG
        pthread_mutex_lock(&mut_count);
        TEST++;
        pthread_mutex_unlock(&mut_count);


        naive_update(coordinate_current, tree, table_coordinate, table_tMRCA);
        //===== test =====
        count ++;
        if(count % 100 == 0)
            printf("thread#%d has processed %ld trees.\n", package->seq, count);

        // get a tree; if there are no more trees (the end for all the trees): break
        THRESHOLD = THRESHOLD + DISCRETIZATION;

        // ### there are 4 possible exits for the following loop ###
        // 1. find the next tree and it's still within end_exp
        // 2. find the next tree but it's unfortunately beyond the end_exp
        // 3. don't find the next tree but we have already gone beyond the end_exp
        // 4. don't find the next tree (we have additional 0 or more trees) but we have already met the end of the file
        while(coordinate_current < end_exp)
        {
            while(!feof(filehandle))
            {
                pthread_mutex_lock(&mut_read);
                fgets(tree, TREELEN, filehandle);
                pthread_mutex_unlock(&mut_read);
                if(tree_judge(tree))break;
            }
            if(feof(filehandle))
            {
                //==== behave accordingly for the end of the chromosome ====
                if(FORMAT) // Newick
                {
                    coordinate_current = coordinate_current + seg_len_last;
                    CLENGTH = coordinate_current;
                }
                else    // Nexus
                {
                    coordinate_current = CLENGTH;
                }
                break;
            }
            coordinate = get_coordinate(tree);
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

            if(coordinate_current >= THRESHOLD)break;
        }

        // the only judgement to jump out this loop is always "coordinate_current >= end_exp"; so do nothing here

    }

    if(THREADS != 1 && package->seq != THREADS)  // there is one more tree
    {
        //DEBUG
        pthread_mutex_lock(&mut_count);
        TEST++;
        pthread_mutex_unlock(&mut_count);

        
        naive_update(coordinate_current, tree, table_coordinate, table_tMRCA);
    }

    //===== free the local heap memory ======
    free(tree);

    // return: the thread#1 should not terminate, but others should
    printf("this is the end for chunk #%d: end:%ld end_expected:%ld\n", package->seq, coordinate_current, end_exp);
    if(package->seq == 1)   // this is 1. the first thread(main thread) in multi-threads; 2. the only thread in sequential version
        return 0;
    pthread_exit(NULL);             // end other threads (sub-threads)
}


// merge all the chunks (with the tables in the packages)
void chunk_merge(chunk_package * package, int seq)
{
    // merging algorithm
    // 1. if this is the first chunk: if it's a -1, record "0"; else, use "0" to report; then use the last point to substitute the first point
    // 2. if this is not the first chunk: if this is a -1, pass the previous point here; otherwise, report with the previous point, and then use the last point to substitute the first point
    // 3. if this is the last chunk, we should use the CLENGTH to terminate
    if(seq == 0)
    {
        long int * table = package[0].table_coordinate; // long int pointer
        long int name;
        for(long int i = 1; i <= SAMPLE; i++)
        {
            for(long int j = i+1; j <= SAMPLE; j++)
            {
                name = (i - 1) * SAMPLE + j - 1;
                if(table[name] == -1)     // there is no change in this chunk
                {
                    table[name] = 0;
                }
                else
                {
                    IBDreport(i, j, 0, table[name]);
                    table[name] = table[SAMPLE * SAMPLE - 1 - name];
                }
            }
        }
    }
    else
    {
        long int * tableA = package[seq-1].table_coordinate; // long int pointer
        long int * tableB = package[seq].table_coordinate;
        long int name;
        for(long int i = 1; i <= SAMPLE; i++)
        {
            for(long int j = i+1; j <= SAMPLE; j++)
            {
                name = (i - 1) * SAMPLE + j - 1;
                if(tableB[name] == -1)     // there is no change in this chunk
                {
                    tableB[name] = tableA[name];
                }
                else
                {
                    IBDreport(i, j, tableA[name], tableB[name]);
                    tableB[name] = tableB[SAMPLE * SAMPLE - 1 - name];
                }
            }
        }
    }

    if(seq == THREADS - 1)
    {
        // finally use the CLENGTH to terminate all potential segments
        long int * table = package[seq].table_coordinate; // long int pointer
        long int name;
        for(long int i = 1; i <= SAMPLE; i++)
        {
            for(long int j = i+1; j <= SAMPLE; j++)
            {
                name = (i - 1) * SAMPLE + j - 1;
                IBDreport(i, j, table[name], CLENGTH);
            }
        }
    }

    if(seq > 0)
        printf("Finish merging chunk #%d and chunk #%d\n", seq, seq+1);
    return;
}

// end of Naive_chunk.cpp