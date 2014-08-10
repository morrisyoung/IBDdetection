// Chunk_level.cpp
// function: processing all the trees in one chunk; sequential or multi-threaded
//          we also perform block_finding algorithm and prepare for the Block_level function

#include "Chunk_level.h"
#include "IBDdetection.h"
#include "Global.h"
#include "Basic.h"
#include "Parser.h"
#include "Block_level.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <list>
#include <vector>
#include <unordered_map>
#include <string.h>     /* memcpy */


using namespace std;


// input: the filehandle; the pool
// output: the coordinate of present tree; the updated pool (if this is the last chunk, there is no new tree added)
long int get_tree(FILE * filehandle, list<char *> * pool_pointer)
{
    // allocate the memory; if need, release in the future
    char * tree = (char *)malloc(sizeof(char) * TREELEN);
    while(!feof(filehandle))
    {
        pthread_mutex_lock(&mut_read);
        fgets(tree, TREELEN, filehandle);
        pthread_mutex_unlock(&mut_read);
        if(tree_judge(tree))break;
    }

    if(tree_judge(tree))
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
    // DEBUG
    cout << (*pool_pointer).size() << endl;

    long int coordinate = get_coordinate((*pool_pointer).back());
    char * tree_last;

    if(coordinate - package->start >= FILTER)   // we should appropriately set FILTER to avoid 0 value
    {
        tree_last = (* pool_pointer).back();
        package->end = coordinate;
        // !!! potential problems here !!!
        unchanged_pairs_hash(tree_first, tree_last, &(package->hashtable));

        //============ should always judge whether or not this is the last block ============
        coordinate = get_tree(filehandle, pool_pointer);
        if(coordinate >= end_exp)
            package->last = 1;
        else
            package->last = 0;

        return tree_last;
    }

    // if we enter this function, it must have another block within this chunk
    while(1)
    {
        tree_last = (* pool_pointer).back();
        coordinate = get_tree(filehandle, pool_pointer);
        if(coordinate >= end_exp)   // there must be two trees before
        {
            package->last = 1;
            package->end = get_coordinate(tree_last);
            unchanged_pairs_hash(tree_first, tree_last, &(package->hashtable));
            return tree_last;
        }
        if(coordinate - package->start >= FILTER)
        {
            if(get_coordinate(tree_last) == get_coordinate(tree_first))
            {
                tree_last = (*pool_pointer).back();
                package->end = get_coordinate(tree_last);
                unchanged_pairs_hash(tree_first, tree_last, &(package->hashtable));
                //============ should always judge whether or not this is the last block ============
                coordinate = get_tree(filehandle, pool_pointer);
                if(coordinate >= end_exp)
                    package->last = 1;
                else
                    package->last = 0;

                return tree_last;
            }
            else
            {
                package->last = 0;
                package->end = get_coordinate(tree_last);
                unchanged_pairs_hash(tree_first, tree_last, &(package->hashtable));
                return tree_last;
            }
        }
    }
}


// the working thread for chunk_level function
void * chunk_level(void * p)
{
    chunk_package * package = (chunk_package *)p;
    FILE * filehandle = fopen(filename, "r");
    char tree[TREELEN];

    if(package->seq == 1)
    {
        while(1)
        {
            // still use a locker to protect
            pthread_mutex_lock(&mut_read);
            fgets(tree, TREELEN, filehandle);
            pthread_mutex_unlock(&mut_read);

            if(!tree_judge(tree))continue;
            break;  // after we go out, we can process normally
        }
    }
    else    // we should find the start coordinate, and at the same time fill the boundary block package
    {
        //===================== find the start of this working threads =======================
        //============ detect all the following chunks and initialize them =============
        // we use two tree vector to assist in the boundary_block extraction process
        char * tree1 = (char *)malloc(sizeof(char) * TREELEN);  // the present
        char * tree2 = (char *)malloc(sizeof(char) * TREELEN);  // the previous
        char * tree_temp;
        long int coordinate;

        while(1)
        {
            // still use a locker to protect
            pthread_mutex_lock(&mut_read);
            fgets(tree1, TREELEN, filehandle);
            pthread_mutex_unlock(&mut_read);

            if(!tree_judge(tree1))continue;

            coordinate = get_coordinate(tree1);
            tree_temp = tree1;
            tree1 = tree2;
            tree2 = tree_temp;
            if(coordinate >= package->start_exp)
                break;
        }
        package->start = coordinate;
        boundary_block[package->seq-2].start = get_coordinate(tree1);
        boundary_block[package->seq-2].end = get_coordinate(tree2);
        unchanged_pairs(tree1, tree2, boundary_block[package->seq-2].table);

        // DEBUG
        //cout << package->seq-2 << endl;
        //cout << boundary_block.size() << endl;
        //boundary_block[package->seq-2] = p;

        //=============== free the temporary memory ================
        free(tree1);
        free(tree2);
    }

    //======================= the works for a chunk formally starts from here =======================
    // by now, we should have:
    // tree[TREELEN]; full chunk package; filled boundary block package (previous block package)
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
    list<char *> pool;  // this is a queue; please use HEAP

    // these are local things in this scope, and they will be updated after each loop
    block_package p1;
    block_package p2;
    block_package p3;

    block_package * pointer1 = &p1;
    block_package * pointer2 = &p2;
    block_package * pointer3 = &p3;
    block_package * pointer4;   // used to change the pointers
    void * pointer_temp1;
    block_boundary_package * pointer_temp2;

    // first of all we should put the first tree into the pool
    // in the following code, tree_temp is the pointer to the last tree of previous block
    char * tree_temp = (char *)malloc(sizeof(char) * TREELEN);
    memcpy(tree_temp, tree, sizeof(char)*TREELEN);
    pool.push_back(tree_temp);

    // we assume that there are at least two blocks in this chunk; then we step into the loop
    //==================== blcok #2 and #3 ======================
    pointer2->first = 1;
    pointer2->chunk_num = package->seq;
    pointer2->start = package->start;
    // tree_temp is the last tree from last block
    tree_temp = block_finding(pointer2, &pool, filehandle, tree_temp, package->end_exp);

    pointer3->chunk_num = package->seq;
    pointer3->start = p2.end;
    tree_temp = block_finding(pointer3, &pool, filehandle, tree_temp, package->end_exp);

    //================= block level function =================
    // (void *) is used to transfer different types of pointers
    if(package->seq == 1)   // there are no previous block
    {
        block_level(table, pointer_temp1, (void *)pointer2, (void *)pointer3, &pool);
    }
    else    // use the boundary block as the previous block
    {
        pointer_temp2 = &boundary_block[package->seq-2];
        block_level(table, (void *)pointer_temp2, (void *)pointer2, (void *)pointer3, &pool);
    }

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
        pointer3->chunk_num = package->seq;
        pointer3->start = pointer2->end;
        tree_temp = block_finding(pointer3, &pool, filehandle, tree_temp, package->end_exp);

        //================ block level function ================
        block_level(table, (void *)pointer1, (void *)pointer2, (void *)pointer3, &pool);
    }

    //============= preform the last block level function ==============
    pointer1 = pointer2;
    pointer2 = pointer3;

    pointer3->chunk_num = package->seq;
    pointer3->start = pointer2->end;
    tree_temp = block_finding(pointer3, &pool, filehandle, tree_temp, package->end_exp);

    //================= block level function =================
    // (void *) is used to transfer different types of pointers
    if(package->seq == THREADS)   // there are no previous block
    {
        block_level(table, (void *)pointer1, (void *)pointer2, pointer_temp1, &pool);
    }
    else    // use the boundary block as the previous block
    {
        pointer_temp2 = &boundary_block[package->seq-1];
        block_level(table, (void *)pointer1, (void *)pointer2, (void *)pointer_temp2, &pool);
    }

    // return: the thread#1 should not terminate, but others should
    printf("this is the end for chunk #%d: end:%ld end_expected:%ld\n", package->seq, pointer2->end, package->end_exp);
    if(package->seq == 1)   // this is 1. the first thread(main thread) in multi-threads; 2. the only thread in sequential version
        return 0;
    pthread_exit(NULL);             // end other threads (sub-threads)
}


// merge all the chunks (with the tables in the packages)
void chunk_merge(chunk_package * package, int seq)
{
    // merge these two tables
    long int * tableA = package[seq-1].table; // long int pointer
    long int * tableB = package[seq].table;

    long int i, j;
    long int name, name1;
    for(i = 1; i <= SAMPLE; i++)
        for(j = i+1; j <= SAMPLE; j++)
        {
            name = (i - 1) * SAMPLE + j - 1;
            name1 = SAMPLE * SAMPLE - 1 - name;     // the other side in the matrix
            if(tableA[name] >= 0)
            {
                if(tableB[name1] != -1) // this can be closed
                {
                    IBDreport(i, j, tableA[name], tableB[name1]);
                }
                else    // this cannot be closed right now, so we pass it down to the next table
                {
                    tableB[name] = tableA[name];
                }
            }
        }

    cout << "finish merge chunk #" << seq << " and chunk #" << seq+1 << endl;
    return;
}

// end of Chunk_level.cpp