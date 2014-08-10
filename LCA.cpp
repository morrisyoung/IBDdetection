// LCA.cpp
// Bender&Colton's LCA algorithm; The LCA Problem Revisited (2000);
// start from parsing the Nexus/Newick tree
// note: if the number of chromosomes is too small, use the in-order traversal method instead

// Jul 4
// the problem for LCA: the data structure is not optimized, so the running speed is terrible
// we can further optimize it, but the resulting preformance should not be super impressive

// Jul 16
// I have made all that can be global global, so there is some speedup;
// but the constant query still consume some time

#include <math.h>
#include <iostream>
#include "LCA.h"
#include "RMQmode.h"
#include "Basic.h"
#include "Global.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <sys/time.h>


using namespace std;


//the following are global in this scope, so we should update (clean, refill) them everytime we call the LCA pre-processing
vector<int> E;  // used as the list of labels; not initialized
vector<int> L;  // used as the list of levels; not initialized
vector<double> r;  // used for the internal nodes; not initialized
vector<int> R;  // used for the leaves; use the names of nodes to index; initialized

vector<int> list_up_val;	// 5000: 714, so it won't go beyond 1000
vector<int> list_up_pos;	// the same with above
vector<string> list_up_type;
unordered_map<string, int> upResult_val;
unordered_map<string, int> upResult_pos;


//================================================= the Euler tour ========================================
// function: a simple and subtle parser; linear time Euler tour to get all the necessary lists for LCA
// the description for the E L r and R:
// E: not initialized; put all the labels
// L: not initialized; put the levels of all nodes; so using the stack to help
// r: not initialized; put the tMRCA value of the internal nodes
// R: initialized; use the names of the nodes to index the positions of these nodes' positions in E
// input: the tree (Nexus/Newick)
// output: the updated E, L, r and R
void LCA_list_construction(char * tree)
{
	//R.assign(SAMPLE, 0);
	// the following stack and l2 are all local things, which will later be dropped
	vector<int> stack;  // used to store all the internal nodes temporarily
	int parent = 0;  // record the in-time present internal node in the stack

	vector<double> l2;  // store the MRCA of corresponding nodes; initialized as None

	int i = 0;
	while(tree[i] != '(')i++;

	while(tree[i] != ';')
	{
		if(tree[i] == ' ' || tree[i] == ';')
		{
			i++;
			// do nothing here!!
			continue;
		}

		if(tree[i] == ',')
		{
			i++;
			E.push_back(stack.back());
			L.push_back(stack.size()-1);
			continue;
		}

		if(tree[i] == '(')
		{
			i++;
			stack.push_back(parent);
			E.push_back(stack.back());
			L.push_back(stack.size()-1);
			r.push_back(0);
			parent++;
			l2.push_back(0);
			continue;
		}

		if(tree[i] >= 48 && tree[i] <= 57)
		{
			char node[5];
			int j = 0;
			while(tree[i] != '.')
			{
				node[j++] = tree[i++];
			}
			node[j] = '\0';
			long int node_value = string_long(node);
			while(tree[i++] != ':');
			char tmrca[20];  // I give this double type number 20 effective digits; it is just kidding of course
			j = 0;
			while(tree[i] != ',' && tree[i] != ')' && tree[i] != '\0')
			{
				tmrca[j++] = tree[i];
				i++;
			}
			tmrca[j] = '\0';
			double tmrca_value = string_double(tmrca);

			E.push_back(node_value);
			L.push_back(stack.size());
			R[node_value-1] = E.size() - 1;

			if(l2.back() == 0)  // this is the left sub-tree
			{
				l2.back() = tmrca_value;
			}
			continue;
		}

		if(tree[i] == ')')  // there may be a ":" following the ")"
		{
			if(tree[i+1] != ';')
			{
				// two possibilities: left leaf and right leaf
				i+=2;
				char tmrca[20];  // I give this double type number 20 effective digits
				int j = 0;
				while(tree[i] != ',' && tree[i] != ')' && tree[i] != '\0')
				{
					tmrca[j++] = tree[i];
					i++;
				}
				tmrca[j] = '\0';
				double tP = string_double(tmrca);

				E.push_back(stack.back());
				L.push_back(stack.size()-1);
				r[E.back()] = l2.back();
				stack.pop_back();

				double temp = l2.back();
				l2.pop_back();
				if(l2.back() == 0)  // back from the left sub-tree
				{
					temp += tP;
					l2.back() = temp;
				}
			}
			else
			{
				i++;
				E.push_back(stack.back());
				L.push_back(stack.size()-1);
				r[E.back()] = l2.back();
				stack.pop_back();
				l2.pop_back();
			}
			continue;
		}
	}
	return;
}
//====================================================================================================


//======================================= RMQ pre-processing =========================================
// type: dynamic programming, nlogn time where n is the length of the list
// input: a list containing numbers waiting to be compared
// output: two hashing tables, of which one record the minimum value of all the queries, and
//	the other the in-list position of that minimum value; the query are length of 2^k
void Dynamic()
{
	int N = list_up_val.size();

	int expo = 1;
	while(1)
	{
		for(int i = 0; i < N; i++)
		{
			if(i + (expo - 1) >= N)break;
			string key;
			intdashint(i, i+expo-1, &key);

			if(expo == 1)
			{
				upResult_val[key] = list_up_val[i];
				upResult_pos[key] = i;
			}
			else
			{
				string key1;
				intdashint(i, i+(expo/2)-1, &key1);
				string key2;
				intdashint(i+(expo/2), i+expo-1, &key2);

				if(upResult_val[key1] < upResult_val[key2])
				{
					upResult_val[key] = upResult_val[key1];
					upResult_pos[key] = upResult_pos[key1];
				}
				else
				{
					upResult_val[key] = upResult_val[key2];
					upResult_pos[key] = upResult_pos[key2];
				}
			}
		}
		expo = expo*2;
		if(expo > N)break;
	}

	return;
}


// type: total linear time pre-processing; two parts: linear time mode finding, and dynamic ST filling
// input: the list of node levels
// output: list_up_val, list_up_pos, list_up_type, upResult_val, upResult_pos
void LCA_RMQ()
{
	//================== fill the upper list, with value, position and block type ==================
	// we don't need to clean the following everytime we come here
	// the following is local; but it will be used by another local, so we just make it global
	//list_up_val.assign(BLOCK_NUM, 0);

	// the following belong to me; but we don't need to initialize them everytime if we initialize them in the global space in advance
	//list_up_pos.assign(BLOCK_NUM, 0);
	
	//string s;
	//list_up_type.assign(BLOCK_NUM, s);  // as there will be strings, so give them NULL temporarily

	for(int i = 0; i < BLOCK_NUM; i++)
	{
		// the actual range in previous L
		int start = i * BLOCK_SIZE;
		int end = (i+1) * BLOCK_SIZE - 1;

		if(i != BLOCK_NUM-1)
		{
			//========================= RMQ pre-processing ============================
			// not separate function any more
			// type: linear time pre-processing to find the +-1 mode in one pattern
			// input: block list, MODE_BLOCK
			// output: block type, minimum position, minimum value
			string key;
			for(int i1 = start + 1; i1 <= end; i1++)
			{
				if(L[i1] > L[i1-1])
				{
					key.push_back('1');
				}
				else
				{
					key.push_back('0');
				}
			}
			string query;
			query.push_back('0');
			query.push_back('-');
			query.push_back(BLOCK_SIZE-1 + 48);  // the BLOCK_SIZE won't go beyond 9 (5000 -> 7)
			
			int pos = MODE_BLOCK[key][query];
			int val = L[start + pos];

			list_up_val[i] = val;
			list_up_pos[i] = pos;
			list_up_type[i] = key;
		}
		else
		{
			//========================= RMQ pre-processing ============================
			// type: linear time pre-processing to find the +-1 mode in one pattern
			// input: block list, MODE_BLOCK
			// output: block type, minimum position, minimum value
			string key;
			for(int i1 = start + 1; i1 <= end; i1++)
			{
				if(L[i1] > L[i1-1])
				{
					key.push_back('1');
				}
				else
				{
					key.push_back('0');
				}
			}
			string query;
			query.push_back('0');
			query.push_back('-');
			query.push_back(BLOCK_SIZE-1 + 48);  // the BLOCK_SIZE won't go beyond 9 (5000 -> 7)
			
			int pos = MODE_BLOCK[key][query];
			int val = L[start + pos];

			list_up_val[i] = val;
			list_up_pos[i] = pos;
			list_up_type[i] = key;
		}
	}
	
	//===================================== dynamic ST filling ====================================
	// now we have these two lists; we should further perform the nlogn processing
	Dynamic();
	
	// the following lists or tables are important to get the final results: (transfer back if needed!!)
	// list_up_pos, list_up_type, upResult_val, upResult_pos, (global: L, MODE_BLOCK, MODE_BLOCK_LAST)
	return;
}
//======================================================================================================


// function: the entrance to the LCA processing
void LCA_preprocess(char * tree)
{
	// prepare the data structures for further processing
	//================================================================
	// R is fixed with size of SAMPLE; others will be cleaned everytime
	if(R.empty())
	{
		R.assign(SAMPLE, 0);
	}
	E.clear();
	L.clear();
	r.clear();

	if(list_up_val.empty())
	{
		list_up_val.assign(BLOCK_NUM, 0);
	}
	if(list_up_pos.empty())
	{
		list_up_pos.assign(BLOCK_NUM, 0);
	}
	if(list_up_type.empty())
	{
		string s;
		list_up_type.assign(BLOCK_NUM, s);  // as there will be strings, so give them empty string temporarily
	}

	//upResult_val;  -> don't need to renew
	//upResult_pos;  -> the same with above
	//================================================================

	LCA_list_construction(tree);  // initializing the E L R r now

    // there has some problems in this fuction
	LCA_RMQ();

	return;
}


//======================================== constant tMRCA query ==========================================
// algorithm:
// 1. use the L to perform the RMQ algorithm;
// 2. use the R to find the index (i,j) to be processed with RMQ;
// 3. use the index of the RMQ and E to get the label of that internal node;
// 4. use the r to get the actual tMRCA value
// call:
// double tMRCA = getMRCA(leaf1, leaf2, package, mode);
double getMRCA(long int leaf1, long int leaf2)
{
	double tMRCA;

	int pos1 = R[leaf1-1];
	int pos2 = R[leaf2-1];
	if(pos1 > pos2)  // let pos1 < pos2
	{
		int temp = pos1;
		pos1 = pos2;
		pos2 = temp;
	}

	// now query (pos1, pos2)
	double coordinate1 = pos1*1.0/BLOCK_SIZE;
	double coordinate2 = pos2*1.0/BLOCK_SIZE;

	if(int(coordinate1) == int(coordinate2))
	{
		// they are in the same block, so we can directly find that block and then report
		int block_num = int(coordinate1);
		int pos11 = pos1 - block_num * BLOCK_SIZE;
		int pos22 = pos2 - block_num * BLOCK_SIZE;
		string query;
		intdashint(pos11, pos22, &query);
		int index = MODE_BLOCK[list_up_type[block_num]][query] + block_num * BLOCK_SIZE;
		tMRCA = r[E[index]];
	}
	else
	{
		// they are in different blocks, so we should deal them separately and then
		// deal with the upper list
		if(int(coordinate1) + 1 == int(coordinate2))
		{
			// they are in adjacent blocks
			int block_num1 = int(coordinate1);
			int pos11 = pos1 - block_num1 * BLOCK_SIZE;
			string query1;
			intdashint(pos11, BLOCK_SIZE-1, &query1);
			int index1 = MODE_BLOCK[list_up_type[block_num1]][query1] + block_num1 * BLOCK_SIZE;

			int block_num2 = int(coordinate2);
			int pos22 = pos2 - block_num2 * BLOCK_SIZE;
			string query2;
			intdashint(0, pos22, &query2);
			int index2 = MODE_BLOCK[list_up_type[block_num2]][query2] + block_num2 * BLOCK_SIZE;

			if(L[index1] < L[index2])
				tMRCA = r[E[index1]];
			else
				tMRCA = r[E[index2]];
		}
		else
		{
			// they are not in adjacent blocks, so there is another index to be compared
			int block_num1 = int(coordinate1);
			int pos11 = pos1 - block_num1 * BLOCK_SIZE;
			string query1;
			intdashint(pos11, BLOCK_SIZE-1, &query1);
			int index1 = MODE_BLOCK[list_up_type[block_num1]][query1] + block_num1 * BLOCK_SIZE;

			int block_num2 = int(coordinate2);
			int pos22 = pos2 - block_num2 * BLOCK_SIZE;
			string query2;
			intdashint(0, pos22, &query2);
			int index2 = MODE_BLOCK[list_up_type[block_num2]][query2] + block_num2 * BLOCK_SIZE;

			// process the third index on which there may be a minimum value
			// this is the range:
			// [block_num1+1, block_num2-1]
			int index3;
			if(block_num1 + 1 == block_num2 - 1)
			{
				// there is only one block between
				index3 = int(list_up_pos[block_num1+1] + (block_num1+1) * BLOCK_SIZE);
			}
			else
			{
				// there are more than one lists between
				int k = int(log((block_num2-1) - (block_num1+1))/log(2));
				int interval = int(pow(2, k));
				string query1;
				intdashint(block_num1+1, block_num1+1+interval-1, &query1);
				string query2;
				intdashint(block_num2-1-interval+1, block_num2-1, &query2);
				if(upResult_val[query1] < upResult_val[query2])
					index3 = upResult_pos[query1];  // this is the block number, not the actual index
				else
					index3 = upResult_pos[query2];
				index3 = int(list_up_pos[index3] + index3 * BLOCK_SIZE);
			}

			// now compare index1, index2 and index3 in the L
			if(L[index1] < L[index2])
			{
				if(L[index1] < L[index3])
					tMRCA = r[E[index1]];
				else
					tMRCA = r[E[index3]];
			}
			else
			{
				if(L[index2] < L[index3])
					tMRCA = r[E[index2]];
				else
					tMRCA = r[E[index3]];
			}
		}
	}
	return tMRCA;
}
//=====================================================================================================

// end of LCA.cpp