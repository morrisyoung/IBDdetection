// Verify_chunk.cpp
// function: processing all the trees in one chunk; sequential or multi-threaded
//          we also perform block_finding algorithm and prepare for the Block_level function

#include "Verify_chunk.h"
#include "Package.h"
#include "Global.h"
#include "Basic.h"
#include "Parser.h"
#include "Verify_block.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <list>
#include <vector>
#include <unordered_map>
#include "MRCA_hash.h"


using namespace std;


// input: the filehandle; the pool
// output: the coordinate of present tree; the updated pool (if this is the last chunk, there is no new tree added)
long int get_tree(FILE * filehandle, list<char *> * pool_pointer)
{
    // allocate the memory; if need, release in the future
    char * tree = (char *)malloc(sizeof(char) * TREELEN);
    while(!feof(filehandle))
    {
        //pthread_mutex_lock(&mut_read);
        fgets(tree, TREELEN, filehandle);
        //pthread_mutex_unlock(&mut_read);
        if(tree_judge(tree))break;
    }

    if(!feof(filehandle))
    {
        (*pool_pointer).push_back(tree);
        return get_coordinate(tree);
    }
    else    // this is the end of the tree file
    {
        free(tree);
        return CLENGTH;
    }
}


// input: see below
// output: we need to update the following in the block package:
// 1. whether or not this is the last block
// 2. the end coordinate of this new block
// 3. the updated hashtable
// and update the pool
// at the same time, return the pointer of the last tree in the block
char * block_finding(block_package * package, list<char *> * pool_pointer, FILE * filehandle, char * tree_first, long int end_exp)
{
    // we only consider that there is only one thread

    char * tree_last = (*pool_pointer).back();
    long int coordinate = get_coordinate(tree_last);

    /* in one thread, there is no possibility that the present coordinate may be beyond CLENGTH
    if(coordinate >= end_exp)   // we should appropriately set FILTER to avoid 0 value
    {
        package->end = coordinate;
        // !!! potential problems here !!!
        unchanged_pairs_hash(tree_first, tree_last, &(package->hashtable));
        // or we should use the nlogn algorithm
        package->last = 1;

        return tree_last;
    }
    */

    if(coordinate - package->start >= FILTER)   // we should appropriately set FILTER to avoid 0 value
    {
        // we are sure that there are only two trees, and we are pointing to the second tree
        //
        //
        
        package->end = coordinate;
        // !!! potential problems here !!!
        //unchanged_pairs_hash(tree_first, tree_last, &(package->hashtable));
        // or we should use the nlogn algorithm
        IBD_hash(tree_first, tree_last, &(package->hashtable));

        // prepare a new tree and judge whether or not the present block is the last block
        coordinate = get_tree(filehandle, pool_pointer);
        if(coordinate >= end_exp && package->chunk_num == THREADS)
            package->last = 1;
        else
            package->last = 0;

        // we should manually set the unchanged pairs here, because it still may be a segment even if the tMRCA between the
        // two trees are different
        all_pairs_hash(tree_first, &(package->hashtable));

        return tree_last;
    }

    // if we enter this function, it must have another block within this chunk
    while(1)
    {
        coordinate = get_tree(filehandle, pool_pointer);
        if(coordinate - package->start >= FILTER)
        {
            // there may be one tree or two trees before
            if(get_coordinate(tree_last) == get_coordinate(tree_first))
            {
                // there are only two trees, and we are right pointing to the second tree
                //
                //

                tree_last = (*pool_pointer).back();

                // we should manually set the unchanged pairs here, because it still may be a segment even if the tMRCA between the
                // two trees are different
                all_pairs_hash(tree_first, &(package->hashtable));

                package->end = get_coordinate(tree_last);

                // prepare a new tree and judge whether or not the present block is the last block
                if(coordinate >= end_exp && package->chunk_num == THREADS)
                    package->last = 1;
                else
                    package->last = 0;

                return tree_last;
            }
            else
            {
                // there are at least three trees, and it's possible that we are pointing to the third tree (meaning only three trees here)
                // we should check whether or not there are only three trees, then we should use all_pairs_hash for the package->hashtable
                package->end = get_coordinate(tree_last);

                //unchanged_pairs_hash(tree_first, tree_last, &(package->hashtable));
                // or we should use the nlogn algorithm
                IBD_hash(tree_first, tree_last, &(package->hashtable));


                // prepare a new tree and judge whether or not the present block is the last block
                if(coordinate >= end_exp && package->chunk_num == THREADS)
                    package->last = 1;
                else
                    package->last = 0;

                return tree_last;
            }
        }

        if(coordinate >= end_exp)
        {
            tree_last = (*pool_pointer).back();
            package->end = get_coordinate(tree_last);
            // !!! potential problems here !!!
            //unchanged_pairs_hash(tree_first, tree_last, &(package->hashtable));
            // or we should use the nlogn algorithm
            IBD_hash(tree_first, tree_last, &(package->hashtable));

            package->last = 1;

            return tree_last;
        }
        
        tree_last = (*pool_pointer).back();
    }
}


// the working thread for chunk_level function
void * chunk_level(void * p)
{
    chunk_package * package = (chunk_package *)p;
    FILE * filehandle = fopen(filename, "r");
    char * tree = (char *)malloc(sizeof(char) * TREELEN);

    if(package->seq == 1)
    {
        while(1)
        {
            // still use a locker to protect
            //pthread_mutex_lock(&mut_read);
            fgets(tree, TREELEN, filehandle);
            //pthread_mutex_unlock(&mut_read);

            if(!tree_judge(tree))continue;
            break;  // after we go out, we can process normally
        }
    }
    else    // we should find the start coordinate, and at the same time fill the boundary block package
    {
        //===================== find the start of this working threads =======================
        long int coordinate;

        while(1)
        {
            // still use a locker to protect
            //pthread_mutex_lock(&mut_read);
            fgets(tree, TREELEN, filehandle);
            //pthread_mutex_unlock(&mut_read);

            if(!tree_judge(tree))continue;

            coordinate = get_coordinate(tree);
            if(coordinate >= package->start_exp)
                break;
        }
        package->start = coordinate;
    }

    //======================= the works for a chunk formally starts from here =======================
    // by now, we should have:
    // tree[TREELEN]; full chunk package
    // so we can:
    // start from the present tree

    // testing
    printf("this is the working thread#%d/%d: start:%ld start_expected:%ld end_expected:%ld\n", package->seq, THREADS, package->start, package->start_exp, package->end_exp);

    // construct the block_package list
    // what do you need? See below:
    //void block_level(long int * table, block_package * package, list<char *> * pool);
    // what are they?
    // table: SAMPLE * SAMPLE size long int table, storing the long segment start/end coordinate
    // package: has three elements, each of which is block_package; we should fill in them
    // pool: a pointer pointing to a list (doubly), in which all the addresses for present trees are stored


    //====================== preparation ========================
    long int * table = (package->table);
    long int name;
    for(long int i = 1; i <= SAMPLE; i++)
    {
        for(long int j = i+1; j <= SAMPLE; j++)
        {
            name = (i - 1) * SAMPLE + j - 1;
            table[name] = 0;
        }
    }
    double * table_tMRCA = (double *)malloc(sizeof(double) * SAMPLE * SAMPLE);
    parser(tree, table_tMRCA);
    list<char *> pool;  // this is a queue; please use HEAP (for memory overflaw safe)

    // these are local things in this scope, and they will be updated after each loop
    block_package p1;
    block_package p2;
    block_package p3;

    block_package * pointer1 = &p1;
    block_package * pointer2 = &p2;
    block_package * pointer3 = &p3;
    block_package * pointer4;   // used to change the pointers
    block_package * pointer_temp;

    // first of all we should put the first tree into the pool
    // in the following code, tree_temp is the pointer to the last tree of previous block/the first block of present block
    char * tree_temp = tree;
    pool.push_back(tree_temp);
    //===========================================================

    // we assume that there are at least two blocks in this chunk; then we step into the loop
    //==================== blcok #2 and #3 ======================
    pointer2->first = 1;
    pointer2->chunk_num = package->seq;
    pointer2->start = package->start;
    // tree_temp is the last tree from last block
    tree_temp = block_finding(pointer2, &pool, filehandle, tree_temp, package->end_exp);

    pointer3->first = 0;
    pointer3->chunk_num = package->seq;
    pointer3->start = p2.end;
    tree_temp = block_finding(pointer3, &pool, filehandle, tree_temp, package->end_exp);

    //======= block level function =======
    // (void *) is used to transfer different types of pointers
    // there are no previous block
    int count = 1;
    printf("working on #%d block for thread #%d; start: %ld; end: %ld; interval: %ld\n", count, package->seq, pointer2->start, pointer2->end, (pointer2->end - pointer2->start));
    block_level(table, table_tMRCA, pointer_temp, pointer2, pointer3, &pool);
    count ++;

    //===================== here we go: the LOOP ======================
    while(pointer3->last != 1)
    {
        // algorithm:
        // 1. change the pointers
        // 2. find a new block
        // 3. block level function
        //================= exchange the pointers to make them logically transfer ==============
        pointer4 = pointer1;
        pointer1 = pointer2;
        pointer2 = pointer3;
        pointer3 = pointer4;

        //================ find a new block ===================
        pointer3->first = 0;
        pointer3->chunk_num = package->seq;
        pointer3->start = pointer2->end;
        tree_temp = block_finding(pointer3, &pool, filehandle, tree_temp, package->end_exp);

        //================ block level function ================
        printf("working on #%d block for thread #%d; start: %ld; end: %ld; interval: %ld\n", count, package->seq, pointer2->start, pointer2->end, (pointer2->end - pointer2->start));
        block_level(table, table_tMRCA, pointer1, pointer2, pointer3, &pool);
        count ++;
    }

    //============= perform the last block level function ==============
    pointer1 = pointer2;
    pointer2 = pointer3;
    // (void *) is used to transfer different types of pointers
    // there are no following block
    printf("working on #%d block for thread #%d; start: %ld; end: %ld; interval: %ld\n", count, package->seq, pointer2->start, pointer2->end, (pointer2->end - pointer2->start));
    block_level(table, table_tMRCA, pointer1, pointer2, pointer_temp, &pool);


    //=========== before we return, release the local HEAP ============
    free(table_tMRCA);


    // return: the thread#1 should not terminate, but others should
    printf("this is the end for chunk #%d: end:%ld end_expected:%ld\n", package->seq, pointer2->end, package->end_exp);
    if(package->seq == 1)   // this is 1. the first thread(main thread) in multi-threads; 2. the only thread in sequential version
        return NULL;
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
        long int * table = package[0].table; // long int pointer
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
        long int * tableA = package[seq-1].table; // long int pointer
        long int * tableB = package[seq].table;
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
        long int * table = package[seq].table; // long int pointer
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
        cout << "finish merge chunk #" << seq << " and chunk #" << seq+1 << endl;
    return;
}

// end of Verify_chunk.cpp