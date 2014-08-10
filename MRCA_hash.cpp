// MRCA_hash.cpp
// hashing-based tMRCA unchanged pairs searching algorithm

#include <unordered_map>
#include <iostream>
#include <forward_list>
#include <vector>
#include "MRCA_hash.h"
#include "Basic.h"
#include "Global.h"


using namespace std;


unordered_map<double, forward_list<MRCA *>> h11;
vector<long int> list1;
vector<int> list2(10000);  // then we can add one element to one specific position
vector<int> list3;
forward_list<MRCA *> rubbish;	// used to collect the MRCAs in hashing table #2


// make all them global
// here are some important points!!!
// the h1 and h2 are the operating hash table, which should simulate the behavior of the stack
// the h11 and h21 are the resultpointer)s hash table, which contain all the final resultpointer)s; but we don't use h21 any more
// we get the final resultpointer for all the internal nodes only after they have both the two sub-trees


// input: the reference to list1, list2 and the resultpointer) hashing table, the first tree
// output: updated list1, list2 and the resultpointer) hashing table
// what's that:
// list1: in-order nodes list, vector<long int> data type; it is empty list before coming here
// list2: node name indexing position (in list1) list, vector<int> data type; it is initialized already
// hashing table: all the internal nodes (key as partial tMRCA value) and their responsible range of list
void parser1(char * tree)
{
	// this will be returned
	//unordered_map<double, forward_list<MRCA>> h11;
	//vector<long int> list1;
	//vector<int> list2(SAMPLE);  // then we can add one element to one specific position

	// this is local
	unordered_map<double, forward_list<MRCA *>> h1;

	// the stack is also local; should release after we finish using it
	forward_list<double> stack;

	// to get the actual start of a tree; from the "(" to start
	int i = 0;
	while(tree[i] != '(')i++;

	while(tree[i] != ';')
	{
		if(tree[i] == ',' || tree[i] == ' ' || tree[i] == ';')
		{
			i++;
			continue;
		}

		if(tree[i] == '(')
		{
			i++;
			stack.push_front(-1);
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
			char tmrca[20];  // I give this double type number 20 effective digits; but it is just kidding of course
			j = 0;
			while(tree[i] != ',' && tree[i] != ')' && tree[i] != '\0')
			{
				tmrca[j++] = tree[i];
				i++;
			}
			tmrca[j] = '\0';
			double tmrca_value = string_double(tmrca);
			
			// for the two lists:
			// the first one record the actual values of all the leaves;
			// the second one use (node-1) to index their actual positions in the first list
			list1.push_back(node_value);
			list2[node_value-1] = list1.size()-1;  // so the stored value is the index!!


			/*
			// for DEBUG use (float number hashing problem)
			if(node_value == 64 || node_value ==  170)
			{
				cout << stack.front() << endl;
				cout << node_value << " " << list2[node_value-1] << endl;
				cout << tmrca_value << " " << partial_tMRCA(tmrca_value, PRECISION) << endl;
			}
			*/


			if(stack.front() == -1)
			{
				stack.front() = tmrca_value;
				double tmrca_value1 = partial_tMRCA(tmrca_value, PRECISION);
				// tmrca_value1 is the hashing key, and tmrca_value is the actual tMRCA value
				std::unordered_map<double, forward_list<MRCA *>>::const_iterator got = h1.find(tmrca_value1);
  				if(got == h1.end())
  				{
  					forward_list<MRCA *> temp;
  					MRCA * mrcapointer = (MRCA *)malloc(sizeof(MRCA));
					(*mrcapointer) = {tmrca_value, list2[node_value-1], list2[node_value-1], 0};
  					temp.push_front(mrcapointer);
  					h1.emplace(tmrca_value1, temp);
  				}
  				else
  				{
  					MRCA * mrcapointer = (MRCA *)malloc(sizeof(MRCA));
					(*mrcapointer) = {tmrca_value, list2[node_value-1], list2[node_value-1], 0};
  					h1[tmrca_value1].push_front(mrcapointer);
  					//got->second.push_front(mrca);  // with some errors in compiling
  				}
			}
			else
			{
				// we have used h1 as a stack, so just use the last element
				double tmrca_value1 = partial_tMRCA(stack.front(), PRECISION);
				h1[tmrca_value1].front()->end = list2[node_value-1];
			}
			continue;
		}

		if(tree[i] == ')')
		{
			if(tree[i+1] != ';')
			{
				// here we can get a tP value
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

				// this hashing key will be used several times
				double tmrca_value1 = partial_tMRCA(stack.front(), PRECISION);

				int start = h1[tmrca_value1].front()->start;
				int end = h1[tmrca_value1].front()->end;

				// POSSIBLY SPEEDUP
				// add to the final resultpointer) hashing table
				MRCA * mrcapointer = h1[tmrca_value1].front();
				h1[tmrca_value1].pop_front();

				std::unordered_map<double, forward_list<MRCA *>>::const_iterator got = h11.find(tmrca_value1);
  				if(got == h11.end())
  				{
  					forward_list<MRCA *> temp;
  					temp.push_front(mrcapointer);
  					h11[tmrca_value1] = temp;
  					//h11.emplace(partial_tMRCA(stack.front(), PRECISION), temp);
  				}
  				else
  				{
  					h11[tmrca_value1].push_front(mrcapointer);
					//got->second.push_front(mrca);
  					//h1[tmrca_value1].push_front(mrca);
  				}


  				// mrca_value is the actual tMRCA value, mrca_value1 is the partial hashing key
				double tmrca_value = stack.front() + tP;
				stack.pop_front();
				tmrca_value1 = partial_tMRCA(tmrca_value, PRECISION);

				// judge this is a left sub-tree or a right sub-tree
				if(stack.front() == -1)
				{
					// this is a left sub-tree
					stack.front() = tmrca_value;

					std::unordered_map<double, forward_list<MRCA *>>::const_iterator got = h1.find(tmrca_value1);
	  				if(got == h1.end())
	  				{
	  					forward_list<MRCA *> temp;
	  					MRCA * mrcapointer = (MRCA *)malloc(sizeof(MRCA));
						(*mrcapointer) = {tmrca_value, start, end, 0};
	  					temp.push_front(mrcapointer);
	  					h1[tmrca_value1] = temp;
	  					//h11.emplace(partial_tMRCA(stack.front(), PRECISION), temp);
	  				}
	  				else
	  				{
	  					MRCA * mrcapointer = (MRCA *)malloc(sizeof(MRCA));
						(*mrcapointer) = {tmrca_value, start, end, 0};
	  					h1[tmrca_value1].push_front(mrcapointer);
						//got->second.push_front(mrca);
	  					//h1[tmrca_value1].push_front(mrca);
	  				}
				}
				else
				{
					// this is a right sub-tree
					h1[partial_tMRCA(stack.front(), PRECISION)].front()->end = end;
				}
			}
			else
			{
				// here we are finished the parsing; just continue
				i++;

				// remember to move up here!!
				double tmrca_value1 = partial_tMRCA(stack.front(), PRECISION);
				MRCA * mrcapointer = h1[tmrca_value1].front();
				h1[tmrca_value1].pop_front();

				std::unordered_map<double, forward_list<MRCA *>>::const_iterator got = h11.find(tmrca_value1);
  				if(got == h11.end())
  				{
  					forward_list<MRCA *> temp;
  					temp.push_front(mrcapointer);
  					h11[tmrca_value1] = temp;
  					//h11.emplace(partial_tMRCA(stack.front(), PRECISION), temp);
  				}
  				else
  				{
  					h11[tmrca_value1].push_front(mrcapointer);
					//got->second.push_front(mrca);
  					//h1[tmrca_value1].push_front(mrca);
  				}

  				// finally we should empty the stack
				stack.pop_front();
			}
			continue;
		}
	}
	return;
}


// input: the reference to the hashing table, list1 and list2 from parser1, the reference to
//		the resultpointer) table/vector/array, the second tree
// output: the updated resultpointer) repository
// what's that:
// local list3: in-order position (in list1) list, vector<int> data type
// local h2: the stack behavior temporary hashing table (we always need a stack for the traversal)
// local h21: the final resultpointer) hashing table; it seems that we don't need h21
void parser2(char * tree, unordered_map<long int, double> * resultpointer)
{
	//unordered_map<double, forward_list<MRCA>> h2, h21;
	// temporarily we dob't consider the h21
	unordered_map<double, forward_list<MRCA *>> h2;

	// the following stack is local; should be released after this function
	forward_list<double> stack;

	// to get the actual start of a tree; from the "(" to start
	int i = 0;
	while(tree[i] != '(')i++;

	while(tree[i] != ';')
	{
		if(tree[i] == ',' || tree[i] == ' ' || tree[i] == ';')
		{
			i++;
			continue;
		}

		if(tree[i] == '(')
		{
			i++;
			stack.push_front(-1);
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

			// for the list and the stack:
			// the list is used to put all the leaves in-order, using their previous positions
			// the stack is usual as the previous one, assisting in the stack operation
			list3.push_back(list2[node_value-1]);

			/*
			// DEBUG: see the positions of specific nodes in list3
			if(node_value ==  64 || node_value == 170)
			{
				cout << node_value << " " << int(list3.size()-1) << endl;
			}
			*/

			if(stack.front() == -1)
			{
				stack.front() = tmrca_value;
				double tmrca_value1 = partial_tMRCA(tmrca_value, PRECISION);

				std::unordered_map<double, forward_list<MRCA *>>::const_iterator got = h2.find(tmrca_value1);
  				if(got == h2.end())
  				{
  					forward_list<MRCA *> temp;
  					MRCA * mrcapointer = (MRCA *)malloc(sizeof(MRCA));
					(*mrcapointer) = {tmrca_value, int(list3.size()-1), int(list3.size()-1), 0};
  					temp.push_front(mrcapointer);
  					h2[tmrca_value1] = temp;
  					//h2.emplace(partial_tMRCA(stack.front(), PRECISION), temp);
  				}
  				else
  				{
  					MRCA * mrcapointer = (MRCA *)malloc(sizeof(MRCA));
  					(*mrcapointer) = {tmrca_value, int(list3.size()-1), int(list3.size()-1), 0};
  					h2[tmrca_value1].push_front(mrcapointer);
					//got->second.push_front(mrca);
  					//h2[tmrca_value1].push_front(mrca);
  				}
			}
			else
			{
				h2[partial_tMRCA(stack.front(), PRECISION)].front()->end = list3.size()-1;
			}
			continue;
		}


		if(tree[i] == ')')
		{
			double tmrca_value = stack.front();
			double tmrca_value1 = partial_tMRCA(tmrca_value, PRECISION);
			// perform the intersection from here

			std::unordered_map<double, forward_list<MRCA *>>::const_iterator got1 = h11.find(tmrca_value1);
			double key1 = int((tmrca_value1 + PRECISION) * (1/PRECISION) + NUMERIC) * 1.0 / (1/PRECISION);	// the float number will always have problems, which is frustrating
			std::unordered_map<double, forward_list<MRCA *>>::const_iterator got2 = h11.find(key1);
			double key2 = int((tmrca_value1 - PRECISION) * (1/PRECISION) + NUMERIC) * 1.0 / (1/PRECISION);	// the float number will always have problems, which is frustrating
			std::unordered_map<double, forward_list<MRCA *>>::const_iterator got3 = h11.find(key2);
			if(got1 != h11.end() || got2 != h11.end() || got3 != h11.end())
			{
				int start = h2[tmrca_value1].front()->start;
				int middle = h2[tmrca_value1].front()->middle;
				int end = h2[tmrca_value1].front()->end;
				intersection(tmrca_value, start, middle, end, resultpointer);
			}

			/*
			// DEBUG
			if(tmrca_value1 == 124.8)
			{
				printf("@@\n");
				cout << key1 << endl;
				cout << h2[tmrca_value1].front()->start << endl;
				cout << h2[tmrca_value1].front()->middle << endl;
				cout << h2[tmrca_value1].front()->end << endl;
				printf("@@\n");
			}
			*/


			// judge whether or not it is the end of the string
			if(tree[i+1] != ';')
			{
				// we can get a tP here
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
				
				// get the updated tMRCA and the concatenated list
				// get the information from the un-useful element
				double tmrca_value1 = partial_tMRCA(stack.front(), PRECISION);
				int start = h2[tmrca_value1].front()->start;
				int end = h2[tmrca_value1].front()->end;
				rubbish.push_front(h2[tmrca_value1].front());
				h2[tmrca_value1].pop_front();

				// we don't care h21 any more because we are only testing this algorithm
				// no we should care, because h21 is used to store the final resultpointer), and h2 for stack
				// we have performd the intersection already, so just ignore the h21
				// the following is Python codes --!
				//list_temp1 = h2[stack[-1]//1].pop()
				//if stack[-1]//1 in h21:
				//	h21[stack[-1]//1].append(list_temp1)
				//else:
				//	h21[stack[-1]//1] = [list_temp1]

				double tmrca_value = stack.front() + tP;
				tmrca_value1 = partial_tMRCA(tmrca_value, PRECISION);
				stack.pop_front();

				// judge this is a left sub-tree or a right sub-tree
				if(stack.front() == -1)
				{
					// this is a left sub-tree
					stack.front() = tmrca_value;

					std::unordered_map<double, forward_list<MRCA *>>::const_iterator got = h2.find(tmrca_value1);
	  				if(got == h2.end())
	  				{
	  					forward_list<MRCA *> temp;
	  					MRCA * mrcapointer = (MRCA *)malloc(sizeof(MRCA));
						(*mrcapointer) = {tmrca_value, start, end, 0};
	  					temp.push_front(mrcapointer);
	  					h2[tmrca_value1] = temp;
	  					//h11.emplace(partial_tMRCA(stack.front(), PRECISION), temp);
	  				}
	  				else
	  				{
	  					MRCA * mrcapointer = (MRCA *)malloc(sizeof(MRCA));
						(*mrcapointer) = {tmrca_value, start, end, 0};
	  					h2[tmrca_value1].push_front(mrcapointer);
						//got->second.push_front(mrca);
	  					//h2[tmrca_value1].push_front(mrca);
	  				}
				}
				else
				{
					// this is a right sub-tree
					h2[partial_tMRCA(stack.front(), PRECISION)].front()->end = end;
				}
			}
			else
			{
				// here we are finished the parsing; just continue
				i++;  // --> for break out the while loop

				// remember to move up here!!
				double tmrca_value1 = partial_tMRCA(stack.front(), PRECISION);
				rubbish.push_front(h2[tmrca_value1].front());
				h2[tmrca_value1].pop_front();
				// don't need to move up, because we don't use h21 any more!!
				/*

				std::unordered_map<double, forward_list<MRCA>>::const_iterator got = h21.find(partial_tMRCA(stack.front(), PRECISION));
  				if(got == h21.end())
  				{
  					forward_list<MRCA> temp;
  					temp.push_front(mrca);
  					h21[partial_tMRCA(stack.front(), PRECISION)] = temp;
  					//h21.emplace(partial_tMRCA(stack.front(), PRECISION), temp);
  				}
  				else
  				{
  					h21[partial_tMRCA(stack.front(), PRECISION)].push_front(mrca);
					//got->second.push_front(mrca);
  					//h21[tmrca_value1].push_front(mrca);
  				}
  				*/
  				stack.pop_front();
			}
			continue;
		}
	}
	return;
}


// input: the tMRCA value, #(list_left, list_right)# -> (start, middle, end, list3), h1, list1, resultpointer)
// output: updated resultpointer) table; it should be a hashing table
void intersection(double tMRCA, int start, int middle, int end, unordered_map<long int, double> * resultpointer)
{
	// the start, middle and end are all the positions in list3
	//list_temp = h1[MRCA//1]  # should be like: [ [tMRCA1, start, middle, end], [tMRCA2, start, middle, end], [tMRCA3, start, middle, end], [tMRCA4, start, middle, end] ...          ]
	double tmrca_value = tMRCA;
	double tmrca_value1 = partial_tMRCA(tmrca_value, PRECISION);
	forward_list<MRCA *> list_temp;


	/*
	// DEBUG: the float number hashing problem
	if(start <= 457 && middle >= 457 && end > 457 && end <= 458)
	{
		printf("%f\n", tMRCA);
	}
	if(tmrca_value1 == 124.8)
	{
		printf("@@@\n");
	}
	*/



	for(int i = 0; i < 3; i++)
	{
		// three possible hashing positions
		if(i == 0)
		{
			double key = int((tmrca_value1 + PRECISION) * (1/PRECISION) + NUMERIC) * 1.0 / (1/PRECISION);	// the float number will always have problems, which is frustrating
			std::unordered_map<double, forward_list<MRCA *>>::const_iterator got = h11.find(key);
	  		if(got == h11.end() || (tmrca_value1 - 1.0) + PRECISION - tmrca_value > TOLERANCE)
	  		{
	  			continue;
	  		}
	  		list_temp = h11[key];
		}
		else
		{
			if(i == 1)
			{
				std::unordered_map<double, forward_list<MRCA *>>::const_iterator got = h11.find(tmrca_value1);
		  		if(got == h11.end())
		  		{
		  			continue;
		  		}
		  		list_temp = h11[tmrca_value1];
			}
			else
			{
				double key = int((tmrca_value1 - PRECISION) * (1/PRECISION) + NUMERIC) * 1.0 / (1/PRECISION);
				std::unordered_map<double, forward_list<MRCA *>>::const_iterator got = h11.find(key);
		  		if(got == h11.end() || tmrca_value - (tmrca_value1 - 1.0) > TOLERANCE)
		  		{
		  			continue;
		  		}
		  		list_temp = h11[key];
			}
		}

		// extract all the within-tolerance MRCAs
		forward_list<MRCA *> list_can;
		for(auto itr = list_temp.begin(); itr != list_temp.end(); itr++)
		{
			if((*itr)->tMRCA - tmrca_value <= TOLERANCE && (*itr)->tMRCA - tmrca_value >= -TOLERANCE)
			{
				// this is a candidate
				list_can.push_front(*itr);
			}
		}

		// perform the intersection
		for(auto itr = list_can.begin(); itr != list_can.end(); itr++)
		{
			// can: [tMRCA, start, middle, end], the previous positions in the list
			// start, middle and end are all the positions in list3

			//perform the intersection:
			// we put positions in the  following lists
			forward_list<int> list_left_can;
			forward_list<int> list_right_can;
			int leaf;
			for(int j = start; j <= middle; j++)
			{
				leaf = list3[j];
				if(leaf >= (*itr)->start && leaf <= (*itr)->middle)
				{
					list_left_can.push_front(leaf);
				}
			}
			for(int j = middle + 1; j <= end; j++)
			{
				leaf = list3[j];
				if(leaf >= (*itr)->middle + 1 && leaf <= (*itr)->end)
				{
					list_right_can.push_front(leaf);
				}
			}
			// report
			if(!list_left_can.empty() && !list_right_can.empty())
			{
				// we can't remove them!!!
				// even if we have intersection, we still can't move them; because in the future we may use them again to construct the intersection; so, how to detect the changed pairs?
				// report: we have list_left_can and list_right_can here, with both previous positions in
				for(auto itr1 = list_left_can.begin(); itr1 != list_left_can.end(); itr1++)
					for(auto itr2 = list_right_can.begin(); itr2 != list_right_can.end(); itr2++)
					{
						long int name;
						if(list1[*itr1] < list1[*itr2])
						{
							name = (list1[*itr1] - 1) * SAMPLE + list1[*itr2] - 1;
						}
						else
						{
							name = (list1[*itr2] - 1) * SAMPLE + list1[*itr1] - 1;
						}
						(*resultpointer).emplace(name, (*itr)->tMRCA);
					}
			}


			//perform the reversed intersection:
			forward_list<int> list_left_can1;
			forward_list<int> list_right_can1;
			for(int j = middle + 1; j <= end; j++)
			{
				leaf = list3[j];
				if(leaf >= (*itr)->start && leaf <= (*itr)->middle)
				{
					list_left_can1.push_front(leaf);
				}
			}
			for(int j = start; j <= middle; j++)
			{
				leaf = list3[j];
				if(leaf >= (*itr)->middle + 1 && leaf <= (*itr)->end)
				{
					list_right_can1.push_front(leaf);
				}
			}
			// report
			if(!list_left_can1.empty() && !list_right_can1.empty())
			{
				// we can't remove them!!!
				// even if we have intersection, we still can't move them; because in the future we may use them again to construct the intersection; so, how to detect the changed pairs?
				// report: we have list_left_can and list_right_can here, with both previous positions in
				for(auto itr1 = list_left_can1.begin(); itr1 != list_left_can1.end(); itr1++)
					for(auto itr2 = list_right_can1.begin(); itr2 != list_right_can1.end(); itr2++)
					{
						long int name;
						if(list1[*itr1] < list1[*itr2])
						{
							name = (list1[*itr1] - 1) * SAMPLE + list1[*itr2] - 1;
						}
						else
						{
							name = (list1[*itr2] - 1) * SAMPLE + list1[*itr1] - 1;
						}
						(*resultpointer).emplace(name, (*itr)->tMRCA);
					}
			}
		}
	}
	return;
}


// input: two trees waiting to be compared, the resultpointer) repository
// output: the updated resultpointer) repository (it's a hashing table with "long int: int" pair in)
void IBD_hash(char * tree1, char * tree2, unordered_map<long int, double> * resultpointer)
{
	// NOTE:
	// in practice, we can store the second hashing table, and when next time we enter, we can directly use it
	// we can achieve this by storing the second hashing table as global in this scope

	(*resultpointer).clear();

	// clean the global variables
	h11.clear();
	list1.clear();
	list3.clear();
	rubbish.clear();

	parser1(tree1);

	// DEBUG
	/*
	for(auto itr = h11[124.8].begin(); itr != h11[124.8].end(); itr ++)
	{
		cout << (*itr)->tMRCA << ":" << (*itr)->start << " " << (*itr)->middle << " " << (*itr)->end << " " << endl;
	}
	*/


	parser2(tree2, resultpointer);

	// empty the temporary rubbish pool
	for(auto itr = rubbish.begin(); itr != rubbish.end(); itr ++)
	{
		free(*itr);
	}

	// free the h11 memory for fear that there will be memory overflow
	for(auto itr = h11.begin(); itr != h11.end(); itr ++)
	{
		// (*itr) is a forward_list<MRCA *>
		for(auto itr1 = (*itr).second.begin(); itr1 != (*itr).second.end(); itr1 ++)
		{
			// (*itr1) is a (MRCA *)
			free(*itr1);
		}
	}
	return;
}

// end of MRCA_hash.cpp