// Verify_block.cpp
// function: this is the Block_level functions;
// algorithm:
// 1. find candidates;
// 2. verify;
// 3. update tables and report if possible.
// about the table A/B: -2: no present segment; -1: can't see the start of present segment from this chunk; other: segment start


// special notes:
// we don't provide multi-threads for this implementation (all-verification algorithm -- block finding part; verification part)


// Jul 25
// we have two tables, one for storing all the ongoing IBD segments' start coordinate, and one for their tMRCA
// we store the first tMRCA for further comparison because we want to make things comparable
// (a tMRCA value within the tolerant range of the first tMRCA value will be regarded as the same with the first tMRCA value)


#include "Verify_block.h"
#include "Package.h"
#include "Global.h"
#include "Parser.h"
#include "Basic.h"
#include <unordered_map>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "Verify_lca.h"
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
void block_level(long int * table, double * table_tMRCA, block_package * previous, block_package * present, block_package * next, list<char *> * pool)
{
	// algorithm:
	// 1. go through all the hashing tables, and get the candidates waiting to be verified
	// 2. go through all the trees in present block, and update table and report if needed

	//========================= construct the candidate lists ============================
	// the following two are local, which means they will be dropped after this function
	unordered_map<long int, candidate> can_list1;	// alpha and beta
	unordered_map<long int, candidate> can_list2;	// delta and epsilon

	//==============
	// alpha filling
	//==============
	if(present->first != 1)	// otherwise there will be no alphas
	{
		for(auto itr = previous->hashtable.begin(); itr != previous->hashtable.end(); itr ++)
		{
			std::unordered_map<long int, double>::const_iterator got = present->hashtable.find((*itr).first);
  			if(got == present->hashtable.end())  // this is alpha
  			{
  				candidate can;
  				can.type1 = 1;
  				can.type2 = 0;
  				//can.tMRCA = (*itr).second;
  				can.tMRCA = table_tMRCA[(*itr).first];
  				can_list1[(*itr).first] = can;
  			}
		}
	}
	//=================
	// beta and filling
	//=================
	if(present->last != 1)
	{
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
	//============================
	// no matter what happened, perform delta filling (epsilon no need to get filled)
	//============================
	for(auto itr = present->hashtable.begin(); itr != present->hashtable.end(); itr ++)
	{
		candidate can;
		can.type1 = 1;
		can.type2 = 0;
		//can.tMRCA = (*itr).second;
		can.tMRCA = table_tMRCA[(*itr).first];
		can_list2[(*itr).first] = can;
	}
	//==============##########=========############==============
	// for the last block, if one pair is neither delta nor alpha, we should set it as beta
	// if this is the last block, we should regard them all as possible betas, because
	// we want to close all the pairs using the CLENGTH
	if(present->last == 1)
	{
		for(long int i = 1; i <= SAMPLE; i++)
			for(long int j = i+1; j <= SAMPLE; j++)
			{
				long int name = (i - 1) * SAMPLE + j - 1;
				// check whether this is delta
  				std::unordered_map<long int, candidate>::const_iterator got = can_list2.find(name);
  				if(got != can_list2.end())
  					continue;

  				// this is a beta, but we should judge whether it has already been in can_list1 (alpha)
  				std::unordered_map<long int, candidate>::const_iterator got1 = can_list1.find(name);
  				if(got1 != can_list1.end())  // find you
  				{
  					can_list1[name].type2 = 1;
  				}
  				else
  				{
					candidate can;
  					can.type1 = 0;
  					can.type2 = 1;
  					can.tMRCA = 0;
  					can_list1[name] = can;
  				}
			}
	}
	//============================
	// working table initialization -> no need, because we initialize it at the very first
	//============================
	//
	//
	//
	//
	//


	//================================== verification ==================================
	// otherwise, we should verify whatever we should verify
	double tMRCA;
	long int coordinate;
	long int sample1, sample2, name;
	char * tree;

	// DEBUG
	//cout << can_list1.size() << endl;
	//cout << can_list2.size() << endl;

	// get a tree from the pool
	tree = (*pool).front();
	coordinate = get_coordinate(tree);
	while(1)	// FCFS model for the pool of trees
	{
		// now I have all the trees and I can begin verifying.

		//====== LCA pre-processing ======
	    LCA_preprocess(tree);

	    // alpha and beta checks
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
			tMRCA = getMRCA(sample1, sample2);
			//tMRCA = tMRCA_find(tree, sample1, sample2);

			if(tMRCA - (*itr1).second.tMRCA > TOLERANCE || tMRCA - (*itr1).second.tMRCA < -TOLERANCE)
			{
				// alpha or beta
				if(  (*itr1).second.type1  )		// this is alpha
				{
					// terminate; remove alpha
					(*itr1).second.type1 = 0;

					// DEBUG
					//cout << "hehe: " << table[name] << " " << coordinate << endl;


					IBDreport(sample1, sample2, table[name], coordinate);

					if(  (*itr1).second.type2  )	// this pair continues as beta
					{
						table[name] = coordinate;
						table_tMRCA[name] = tMRCA;
						(*itr1).second.tMRCA = tMRCA;
					}
					continue;	// process beta only after we have processed all the alphas
				}
				if(  (*itr1).second.type2  )	// this is beta
				{
					// we don't need to remove beta, because we will perform till the end of this block
					table[name] = coordinate;
					table_tMRCA[name] = tMRCA;
					(*itr1).second.tMRCA = tMRCA;
				}
			}
		}

		// delta and epsilon checks
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
			tMRCA = getMRCA(sample1, sample2);
			//tMRCA = tMRCA_find(tree, sample1, sample2);

			if(tMRCA - (*itr2).second.tMRCA > TOLERANCE || tMRCA - (*itr2).second.tMRCA < -TOLERANCE)
			{
				if(  (*itr2).second.type1  )		// this is delta
				{
					// terminate; remove delta
					(*itr2).second.type1 = 0;

					// DEBUG
					//cout << "not hehe: " << table[name] << " " << coordinate << endl;

					IBDreport(sample1, sample2, table[name], coordinate);

					// change from delta to epsilon
					// record the coordinate and the tMRCA
					(*itr2).second.type2 = 1;
					table[name] = coordinate;	// no segment presently
					table_tMRCA[name] = tMRCA;
					(*itr2).second.tMRCA = tMRCA;
				}
				else	// this is epsilon
				{
					// we should run the epsilon till the end
					table[name] = coordinate;
					table_tMRCA[name] = tMRCA;
					(*itr2).second.tMRCA = tMRCA;
				}
			}
		}

		// get a new qualified tree from the pool; but if it is already the last tree, use it
		if(coordinate == present->end)
			break;

		long int coordinate_temp = coordinate;
		while(1)
		{
			free(tree);
			(*pool).pop_front();
			tree = (*pool).front();
			coordinate = get_coordinate(tree);
			// test whether or not this is the last coordinate in this block
			if(coordinate == present->end)
				break;
			if(coordinate - coordinate_temp > DISCRETIZATION)
				break;
		}

	}
	//================================== verification done ==================================

	//========= if this is the last block, terminate all deltas and betas ==========
	if(present->last == 1)
	{
		// for delta/epsilon (there are only possibly this two kinds of segments)
		// and for the betas (even the betas after alphas)
		// so just for all pairs
		/*
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
			// no matter this is a delta or epsilon, we shuold always perform termination for it
			IBDreport(sample1, sample2, table[name], CLENGTH);
		}
		*/
		for(long int i = 1; i <= SAMPLE; i++)
			for(long int j = i+1; j <= SAMPLE; j++)
			{
				name = (i - 1) * SAMPLE + j - 1;
				// no matter this is a delta or epsilon, we shuold always perform termination for it
				IBDreport(i, j, table[name], CLENGTH);
			}
	}

	return;
}

// end of Verify_block.cpp