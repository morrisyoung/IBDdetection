// Block_level.cpp
// function: this is the Block_level functions;
// algorithm:
// 1. find candidates;
// 2. verify;
// 3. update tables and report if possible.
// about the table A/B: -2: no present segment; -1: can't see the start of present segment from this chunk; other: segment start

#include "Block_level.h"
#include "IBDdetection.h"
#include "Global.h"
#include "Parser.h"
#include "Basic.h"
#include <unordered_map>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "LCA.h"
#include "RMQmode.h"
#include <sys/time.h>


using namespace std;


// input:
//	table: the start coordinate table for the segment (-1: unseen start from this block; -2: no segment exists)
//	list: previous/present/next block packages
//	pool: the pool (address) storing all the trees in present block and next block (doubly block)
// output:
//	saved IBD segment during the process
//	updated tables (A and B)
void block_level(long int * table, void * previous0, void * present0, void * next0, list<char *> * pool)
{
	// algorithm:
	// 1. go through all the hashing tables, and get the candidates waiting to be verified
	// 2. go through all the trees in present block, and update table and report if needed

	//========================= construct the candidate lists ============================
	// the following two are local, which means they will be dropped after this function
	unordered_map<long int, candidate> can_list1;	// alpha and beta
	unordered_map<long int, candidate> can_list2;	// delta and epsilon


	// TODO integrate the pointers and the new alphas


	block_package * present = (block_package *)present0;

	//============================
	// alpha and beginning related
	//============================
	if(present->first != 1)
	{
		block_package * previous = (block_package *)previous0;

		// normal alpha filling
		for(auto itr = previous->hashtable.begin(); itr != previous->hashtable.end(); itr ++)
		{
			std::unordered_map<long int, double>::const_iterator got = present->hashtable.find((*itr).first);
  			if(got == present->hashtable.end())  // this is alpha
  			{
  				candidate can;
  				can.type1 = 1;
  				can.type2 = 0;
  				can.tMRCA = (*itr).second;
  				can_list1[(*itr).first] = can;
  			}
		}
	}
	else
	{
		if(present->chunk_num != 1)
		{
			block_boundary_package * previous = (block_boundary_package *)previous0;

			// another type of alpha checks; type1 initialize (initialize start of unseen and seen)
			long int i, j;
			long int name, name1;
			for(i = 1; i <= SAMPLE; i++)
				for(j = i+1; j <= SAMPLE; j++)
				{
					name = (i - 1) * SAMPLE + j - 1;
					std::unordered_map<long int, double>::const_iterator got = present->hashtable.find(name);
					if(previous->table[name] != -1)
					{
						table[name] = -1;	// we don't know the actual start
						name1 = SAMPLE * SAMPLE - 1 - name;     // the other side in the matrix
						table[name1] = -1;

						// type1 initialize
						if(got == present->hashtable.end())  // this is alpha
			  			{
			  				candidate can;
			  				can.type1 = 1;
			  				can.type2 = 0;
			  				can.tMRCA = previous->table[name];
			  				can_list1[name] = can;
			  			}
					}
					else
					{
						if(got != present->hashtable.end())
						{
							table[name] = present->start;
						}
						else
						{
							table[name] = -2;
						}
					}
				}
		}
		else
		{
			// type2 initialize (initialize start of seen)
			long int i, j;
			long int name;
			for(i = 1; i <= SAMPLE; i++)
				for(j = i+1; j <= SAMPLE; j++)
				{
					name = (i - 1) * SAMPLE + j - 1;
					std::unordered_map<long int, double>::const_iterator got = present->hashtable.find(name);
					if(got == present->hashtable.end())  // this is a changed pair
					{
						table[name] = -2;
					}
					else  // this is an unchanged pair; the segment starts at 0
					{
						table[name] = 0;
					}
				}
		}
	}
	//============================
	// beta and ending related
	//============================
	if(present->last != 1)
	{
		block_package * next = (block_package *)next0;

		// normal beta checks
		for(auto itr = next->hashtable.begin(); itr != next->hashtable.end(); itr ++)
		{
			std::unordered_map<long int, double>::const_iterator got = present->hashtable.find((*itr).first);
  			if(got == present->hashtable.end())
  			{
  				// this is a beta, but we should judge whether it has already been in can_list1
  				std::unordered_map<long int, candidate>::const_iterator got1 = can_list1.find((*itr).first);
  				if(got1 != can_list1.end())  // find you
  				{
  					can_list1[(*itr).first].type2 = 1;
  				}
  				else
  				{
					candidate can;
  					can.type1 = 0;
  					can.type2 = 1;
  					can.tMRCA = 0;
  					can_list1[(*itr).first] = can;
  				}
  			}
		}
	}
	else
	{
		if(present->chunk_num != THREADS)
		{
			block_boundary_package * next;
			next = (block_boundary_package *)next0;

			// another type of beta checks
			long int i, j, name;
			for(i = 1; i <= SAMPLE; i++)
				for(j = i+1; j <= SAMPLE; j++)
				{
					name = (i - 1) * SAMPLE + j - 1;
					std::unordered_map<long int, double>::const_iterator got = present->hashtable.find(name);
					if(next->table[name] != 0 && got == present->hashtable.end())
					{
		  				candidate can;
		  				can.type1 = 0;
		  				can.type2 = 1;
		  				can.tMRCA = 0;
		  				can_list1[name] = can;
					}
				}
		}
		/*
		else
		{
			// there are no beta checks

		}
		*/
	}
	//============================
	// no matter what happened, perform delta filling (epsilon no need to get filled)
	//============================
	for(auto itr = present->hashtable.begin(); itr != present->hashtable.end(); itr ++)
	{
		candidate can;
		can.type1 = 1;
		can.type2 = 0;
		can.tMRCA = (*itr).second;
		can_list2[(*itr).first] = can;
	}


	// DEBUG
	printf("@@@\n");


	//================================== verification ==================================
	// otherwise, we should verify whatever we should verify
	double tMRCA;
	long int coordinate;
	long int sample1;
	long int sample2;
	long int name;
	long int name1;


	// for the verification, we will have two algorithms to process the tree: 1. direct searching; 2. LCA
	// so we should set a threshold for the selection of each algorithm
	if(can_list1.size() + can_list2.size() >= 100 && SAMPLE >= 100)
	{

		// DEBUG
		cout << can_list1.size() << endl;
		cout << can_list2.size() << endl;

		// LCA pre-processing
		// the following two must be allocated in this space
	    unordered_map<string, unordered_map<string, int>> MODE_BLOCK;
	    unordered_map<string, unordered_map<string, int>> MODE_BLOCK_LAST;
	    RMQmode MODE = RMQmode(&MODE_BLOCK, &MODE_BLOCK_LAST);

		while(1)	// FCFS model for the pool of trees
		{
			// DEBUG
			cout << (*pool).size() << "!!" << endl;

			// get a tree from the pool
			char * tree = (*pool).front();
			// now I have all the trees and I can begin verifying.

			// LCA pre-processing
			// what happened inside? -> the string initial value
		    // the following two must be defined/allocated in this space
		    //============== timing model ===============
		    struct timeval start;
		    struct timeval end;
		    double diff;
		    gettimeofday(&start, NULL);

		    // function
		    unordered_map<string, int> upResult_val;
		    unordered_map<string, int> upResult_pos;
		    LCApackage PACKAGE = LCA(tree, MODE, &upResult_val, &upResult_pos);

		    gettimeofday(&end, NULL);
		    diff = (double)(end.tv_sec-start.tv_sec)+ (double)(end.tv_usec-start.tv_usec)/1000000;
		    printf("Time used totally is %f seconds.\n", diff);
		    //============== timing done ==============



		    // testing another method of pre-processing
		    gettimeofday(&start, NULL);
		    // function
		    double table_tMRCA[SAMPLE * SAMPLE];
			parser(tree, table_tMRCA);
			// function done
		    gettimeofday(&end, NULL);
		    diff = (double)(end.tv_sec-start.tv_sec)+ (double)(end.tv_usec-start.tv_usec)/1000000;
		    printf("Time used totally is %f seconds.\n", diff);


		    // DEBUG
			cout <<  "!!" << endl;



			for(auto itr1 = can_list1.begin(); itr1 != can_list1.end(); itr1 ++)
			{
				name = (*itr1).first;	// this is the pair
				//(*itr1).second	// this is the candidate
				//============ get the sample pair =============
				name += 1;
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
				name -= 1;
				//==============================================

				tMRCA = getMRCA(sample1, sample2, PACKAGE, MODE);
				//tMRCA = table_tMRCA[name];
				if(tMRCA - (*itr1).second.tMRCA > TOLERANCE || tMRCA - (*itr1).second.tMRCA < -TOLERANCE)
				{
					// alpha or beta
					if(  (*itr1).second.type1  )		// this is alpha
					{
						// terminate; remove alpha
						(*itr1).second.type1 = 0;
						coordinate = get_coordinate(tree);
						if(table[name] == -1)	// there are no starting point for this segment
						{
							name1 = SAMPLE * SAMPLE - 1 - name;		// the other side in the matrix
							table[name] = -2;	// no segment presently
							table[name1] = coordinate;
						}
						else	// report and set to 0
						{
							IBDreport(sample1, sample2, table[name], coordinate);
							table[name] = -2;	// no segment presently
						}

						if(  (*itr1).second.type2  )	// this pair continues as beta
						{
							// record the coordinate and the tMRCA
							table[name] = coordinate;
							(*itr1).second.tMRCA = tMRCA;
						}
						continue;	// process beta only after we have processed all the alphas
					}
					if(  (*itr1).second.type2  )	// this is beta
					{
						// terminate; remove alpha
						coordinate = get_coordinate(tree);
						table[name] = coordinate;
						(*itr1).second.tMRCA = tMRCA;
					}
				}
			}

		    // DEBUG
			cout <<  "!!" << endl;

			for(auto itr2 = can_list2.begin(); itr2 != can_list2.end(); itr2 ++)
			{
				name = (*itr2).first;	// this is the pair
				//(*itr2).second	// this is the candidate

				//============ get the sample pair =============
				name += 1;
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
				name -= 1;
				//==============================================

				tMRCA = getMRCA(sample1, sample2, PACKAGE, MODE);
				//tMRCA = table_tMRCA[name];
				if(tMRCA - (*itr2).second.tMRCA > TOLERANCE || tMRCA - (*itr2).second.tMRCA < -TOLERANCE)
				{
					if(  (*itr2).second.type1  )		// this is delta
					{
						// terminate; remove belta
						(*itr2).second.type1 = 0;
						coordinate = get_coordinate(tree);
						if(table[name] == -1)	// there are no starting point for this segment
						{
							name1 = SAMPLE * SAMPLE - 1 - name;		// the other side in the matrix
							table[name] = -2;	// no segment presently
							table[name1] = coordinate;
						}
						else	// report and set to 0
						{
							IBDreport(sample1, sample2, table[name], coordinate);
							table[name] = -2;	// no segment presently
						}

						// change from delta to epsilon
						// record the coordinate and the tMRCA
						(*itr2).second.type2 = 1;
						table[name] = coordinate;
						(*itr2).second.tMRCA = tMRCA;
					}
					else
					{
						// terminate; remove alpha
						coordinate = get_coordinate(tree);
						table[name] = coordinate;
						(*itr2).second.tMRCA = tMRCA;
					}
				}
			}


			// DEBUG
			cout <<  "!!" << endl;


			// test whether or not this is the last coordinate in this block
			if(get_coordinate(tree) == present->end)
			{
				break;
			}
			else
			{
				free(tree);
				(*pool).pop_front();
				continue;
			}
		}
	}
	else
	{
		// DEBUG
		cout << can_list1.size() << endl;
		cout << can_list2.size() << endl;


		while(1)	// FCFS model for the pool of trees
		{
			// DEBUG
			cout << (*pool).size() << endl;

			// get a tree from the pool
			char * tree = (*pool).front();
			// now I have all the trees and I can begin verifying.

			for(auto itr1 = can_list1.begin(); itr1 != can_list1.end(); itr1 ++)
			{
				name = (*itr1).first;	// this is the pair
				//(*itr1).second	// this is the candidate
				//============ get the sample pair =============
				name += 1;
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
				name -= 1;
				//==============================================

				tMRCA = tMRCA_find(tree, sample1, sample2);
				if(tMRCA - (*itr1).second.tMRCA > TOLERANCE || tMRCA - (*itr1).second.tMRCA < -TOLERANCE)
				{
					// alpha or beta
					if(  (*itr1).second.type1  )		// this is alpha
					{
						// terminate; remove alpha
						(*itr1).second.type1 = 0;
						coordinate = get_coordinate(tree);
						if(table[name] == -1)	// there are no starting point for this segment
						{
							name1 = SAMPLE * SAMPLE - 1 - name;		// the other side in the matrix
							table[name] = -2;	// no segment presently
							table[name1] = coordinate;
						}
						else	// report and set to 0
						{
							IBDreport(sample1, sample2, table[name], coordinate);
							table[name] = -2;	// no segment presently
						}

						if(  (*itr1).second.type2  )	// this pair continues as beta
						{
							// record the coordinate and the tMRCA
							table[name] = coordinate;
							(*itr1).second.tMRCA = tMRCA;
						}
						continue;	// process beta only after we have processed all the alphas
					}
					if(  (*itr1).second.type2  )	// this is beta
					{
						// terminate; remove alpha
						coordinate = get_coordinate(tree);
						table[name] = coordinate;
						(*itr1).second.tMRCA = tMRCA;
					}
				}
			}

			for(auto itr2 = can_list2.begin(); itr2 != can_list2.end(); itr2 ++)
			{
				name = (*itr2).first;	// this is the pair
				//(*itr2).second	// this is the candidate

				//============ get the sample pair =============
				name += 1;
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
				name -= 1;
				//==============================================

				tMRCA = tMRCA_find(tree, sample1, sample2);
				if(tMRCA - (*itr2).second.tMRCA > TOLERANCE || tMRCA - (*itr2).second.tMRCA < -TOLERANCE)
				{
					if(  (*itr2).second.type1  )		// this is delta
					{
						// terminate; remove belta
						(*itr2).second.type1 = 0;
						coordinate = get_coordinate(tree);
						if(table[name] == -1)	// there are no starting point for this segment
						{
							name1 = SAMPLE * SAMPLE - 1 - name;		// the other side in the matrix
							table[name] = -2;	// no segment presently
							table[name1] = coordinate;
						}
						else	// report and set to 0
						{
							IBDreport(sample1, sample2, table[name], coordinate);
							table[name] = -2;	// no segment presently
						}

						// change from delta to epsilon
						// record the coordinate and the tMRCA
						(*itr2).second.type2 = 1;
						table[name] = coordinate;
						(*itr2).second.tMRCA = tMRCA;
					}
					else
					{
						// terminate; remove alpha
						coordinate = get_coordinate(tree);
						table[name] = coordinate;
						(*itr2).second.tMRCA = tMRCA;
					}
				}
			}

			// test whether or not this is the last coordinate in this block
			if(coordinate == present->end)
			{
				break;
			}
			else
			{
				free(tree);
				(*pool).pop_front();
				continue;
			}
		}
	}
	//================================== verification done ==================================



	// DEBUG
	printf("@@@\n");



	// if this is the last block or this is in the last chunk, we should terminate some deltas/epsilons
	if(present->last == 1)
	{
		if(present->chunk_num != THREADS)	// has next block
		{
			block_boundary_package * next = (block_boundary_package *)next0;

			// type1 ending (check the boundary block)
			long int name;
			long int sample1, sample2;
			long int coordinate;

			for(auto itr = can_list2.begin(); itr != can_list2.end(); itr ++)
			{
				name = (*itr).first;
				//============ get the sample pair =============
				name += 1;
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
				name -= 1;
				//==============================================
  				if(next->table[name] == -1)
  				{
					coordinate = present->end;
					if(table[name] == -1)	// there are no starting point for this segment
					{
						name1 = SAMPLE * SAMPLE - 1 - name;		// the other side in the matrix
						table[name] = -2;	// no segment presently
						table[name1] = coordinate;
					}
					else	// report and set to 0
					{
						IBDreport(sample1, sample2, table[name], coordinate);
						table[name] = -2;	// no segment presently
					}
  					continue;
				}
			}
		}
		else	// no next block
		{
			// type2 ending (directly end for all deltas/epsilons)
			for(auto itr = can_list2.begin(); itr != can_list2.end(); itr ++)
			{
				name = (*itr).first;
				//============ get the sample pair =============
				name += 1;
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
				name -= 1;
				//==============================================
				coordinate = CLENGTH;
				if(table[name] == -1)	// there are no starting point for this segment
				{
					name1 = SAMPLE * SAMPLE - 1 - name;		// the other side in the matrix
					table[name] = -2;	// no segment presently
					table[name1] = coordinate;
				}
				else	// report and set to 0
				{
					IBDreport(sample1, sample2, table[name], coordinate);
					table[name] = -2;	// no segment presently
				}
			}
		}
	}

	return;
}

// end of Block_level.cpp