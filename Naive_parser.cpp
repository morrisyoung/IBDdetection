// Naive_parser.cpp
// function: parsing the tree, and update the working tables; if there is an IBD segment, report it

#include "Naive_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include "List.h"
#include "Naive_basic.h"
#include "Naive_global.h"
#include <string.h>


// input: the tree
// output: updated all-pair tMRCAs table (we only perform this one function here, as we may not know how to parse the coordinate for different tree formats)
void parser(char * tree, double * table)
{
	int i = 0;

	// get the start of the tree from here
	while(tree[i] != '(')i++;

	List_list * l1 = createList_list();  // store the nodes and their existing list
	List_double * l2 = createList_double();  // store the MRCA of corresponding nodes

	int j = i;
	while(tree[j++] != '\0');
	tree[j-4] = '\0';  // for the "4": I guess there is a '\n' in the string befure '\0'

	// the following used only for IBD update in the while loop
	Node_list * node_list = NULL;
	Node_int * node_int = NULL;
	Node_double * node_double = NULL;
	double value;  // used by the tMRCA update

	while(tree[i] != '\0')
	{
		if(tree[i] == ',' || tree[i] == ' ' || tree[i] == ';')
		{
			i += 1;
			continue;
		}

		if(tree[i] == '(')
		{
			i += 1;
			appendList_list(l1);
			appendList_double(l2, 0);  // previous None in Python
			continue;
		}

		if(tree[i] >= 48 && tree[i] <= 57)
		{
			char node[5];
			int j = 0;
			
			// 24:0.0230624  or  24.1:0.0230624 are all possible, in which the additional "1" means the simulation
			// interation times which is not useful here
			while(tree[i] != '.' && tree[i] != ':')
			{
				node[j++] = tree[i++];
			}
			node[j] = '\0';
			long int node_value;

			// the sample name is from 1 or 0
			if(FROMONE)
			{
				node_value = string_long(node);
			}
			else
			{
				node_value = string_long(node) + 1;
			}

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

			appendList_int(l1, node_value);  // we fix this kind of appending only to the last list in the list

			if((l2->end)->element == 0)  // two sub-nodes
			{
				l2->end->element = tmrca_value;
			}

			//=============== tMRCA update =================
			j = 0;
			node_list = l1->start;
			node_int = (node_list->start)->start;
			node_double = l2->start;

			for(j=1; j<=l1->length; j++)
			{
				do
				{
					if(node_int == NULL)  // there may be an empty list
					{
						break;
					}
					long int node1 = node_int->element;
					if(node1 != node_value)
					{
						long int name;
						if(node1 < node_value)
						{
							name = (node1 - 1) * SAMPLE + node_value - 1;  // here the name should be an long integer
						}
						else
						{
							name = (node_value - 1) * SAMPLE + node1 - 1;
						}


						// just simply record the all tMRCAs
						table[name] = node_double->element;


					}
					if(node_int->next == NULL){break;}
					else{node_int = node_int->next;}
				}
				while(1);
				if(j<l1->length)
				{
					node_list = node_list->next;
					node_int = (node_list->start)->start;
					node_double = node_double->next;
				}
			}
			continue;
		}

		if(tree[i] == ')')  // there must be a ":" following the ")"
		{

			// two possibilities: left leaf and right leaf
			extendList_list(l1);
			i += 2;
			char tmrca[20];  // I give this double type number 20 effective digits
			int j = 0;
			while(tree[i] != ',' && tree[i] != ')' && tree[i] != '\0')
			{
				tmrca[j++] = tree[i];
				i++;
			}
			tmrca[j] = '\0';

			value = string_double(tmrca);
			extendList_double(l2, value);
			continue;
		}
	}
	tree[i] = ')';  // retain the previous tree
	cleanList(l1, l2);

	return;
}


// input: tree
// output: the SAMPLE size of the tree; and the updated FROMONE value
long int parser_sample(char * tree)
{
	// get the start of the tree from here
	int i = 0;
	while(tree[i] != '(')i++;
	long int sample = 0;

	while(tree[i] != ';')
	{
		if(tree[i] == ',' || tree[i] == ' ' || tree[i] == ';' || tree[i] == '(' || tree[i] == ')')
		{
			i += 1;
			continue;
		}

		if(tree[i] >= 48 && tree[i] <= 57)
		{
			sample++;

			char node[5];
			int j = 0;
			// get the FROMONE value
			// 24:0.0230624  or  24.1:0.0230624 are all possible, in which the additional "1" means the simulation
			// interation times which is not useful here
			while(tree[i] != '.' && tree[i] != ':')
			{
				node[j++] = tree[i++];
			}
			node[j] = '\0';
			long int node_value = string_long(node);
			if(node_value == 0)
				FROMONE = 0;

			while(tree[i] != ',' && tree[i] != ')')i++;
			continue;
		}

		if(tree[i] == ':')
		{
			while(tree[i] != ',' && tree[i] != ')')i++;
			continue;
		}
	}

	return sample;
}


void naive_update(long int coordinate, char * tree, long int * table_coordinate, double * table_tMRCA)
{
	int i = 0;
	// get the start of the tree from here
	while(tree[i] != '(')i++;

	List_list * l1 = createList_list();  // store the nodes and their existing list
	List_double * l2 = createList_double();  // store the MRCA of corresponding nodes

	int j = i;
	while(tree[j++] != '\0');
	tree[j-4] = '\0';  // for the "4": I guess there is a '\n' in the string befure '\0'

	// the following used only for IBD update in the while loop
	Node_list * node_list = NULL;
	Node_int * node_int = NULL;
	Node_double * node_double = NULL;
	double value;  // used by the tMRCA update

	while(tree[i] != '\0')
	{
		if(tree[i] == ',' || tree[i] == ' ' || tree[i] == ';')
		{
			i += 1;
			continue;
		}

		if(tree[i] == '(')
		{
			i += 1;
			appendList_list(l1);
			appendList_double(l2, 0);  // previous None in Python
			continue;
		}

		if(tree[i] >= 48 && tree[i] <= 57)
		{
			char node[5];
			int j = 0;
			
			// 24:0.0230624  or  24.1:0.0230624 are all possible, in which the additional "1" means the simulation
			// interation times which is not useful here
			while(tree[i] != '.' && tree[i] != ':')
			{
				node[j++] = tree[i++];
			}
			node[j] = '\0';
			long int node_value;

			// the sample name is from 1 or 0
			if(FROMONE)
			{
				node_value = string_long(node);
			}
			else
			{
				node_value = string_long(node) + 1;
			}

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

			appendList_int(l1, node_value);  // we fix this kind of appending only to the last list in the list

			if((l2->end)->element == 0)  // two sub-nodes
			{
				l2->end->element = tmrca_value;
			}

			//=============== tMRCA update =================
			j = 0;
			node_list = l1->start;
			node_int = (node_list->start)->start;
			node_double = l2->start;

			for(j=1; j<=l1->length; j++)
			{
				do
				{
					if(node_int == NULL)  // there may be an empty list
					{
						break;
					}
					long int node1 = node_int->element;
					if(node1 != node_value)
					{
						long int name;
						if(node1 < node_value)
						{
							name = (node1 - 1) * SAMPLE + node_value - 1;  // here the name should be an long integer
						}
						else
						{
							name = (node_value - 1) * SAMPLE + node1 - 1;
						}

						//============== update the working tables, and report IBD segment ===============
						double tMRCA = node_double->element;

						if(tMRCA - table_tMRCA[name] > TOLERANCE || tMRCA - table_tMRCA[name] < -TOLERANCE)
						{
							if(table_coordinate[name] == -1)	// this is the first change in this chunk
							{
								table_coordinate[name] = coordinate;
								table_coordinate[SAMPLE * SAMPLE - 1 - name] = coordinate;
								table_tMRCA[name] = tMRCA;
							}
							else 	//  this is the beyond-first changed point
							{
								// first of all, report
								//============ get the sample pair =============
								name += 1;
								long int sample1, sample2;
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
								IBDreport(sample1, sample2, table_coordinate[SAMPLE * SAMPLE - 1 - name], coordinate);
								
								table_coordinate[SAMPLE * SAMPLE - 1 - name] = coordinate;
								table_tMRCA[name] = tMRCA;
							}
						}
						//=============================================================================
					}
					if(node_int->next == NULL){break;}
					else{node_int = node_int->next;}
				}
				while(1);
				if(j<l1->length)
				{
					node_list = node_list->next;
					node_int = (node_list->start)->start;
					node_double = node_double->next;
				}
			}
			continue;
		}

		if(tree[i] == ')')  // there must be a ":" following the ")", because we have moved the last ")" away
		{

			// two possibilities: left leaf and right leaf
			extendList_list(l1);
			i += 2;
			char tmrca[20];  // I give this double type number 20 effective digits
			int j = 0;
			while(tree[i] != ',' && tree[i] != ')' && tree[i] != '\0')
			{
				tmrca[j++] = tree[i];
				i++;
			}
			tmrca[j] = '\0';

			value = string_double(tmrca);
			extendList_double(l2, value);
			continue;
		}
	}
	tree[i] = ')';  // retain the previous tree
	cleanList(l1, l2);

	return;
}



// input: the tree
// output: update table_coordinate and table_tMRCA which are last changed coordinate and its tMRCA tables
void stdin_update(long int coordinate, char * tree, long int * table_coordinate, double * table_tMRCA)
{
	int i = 0;
	// get the start of the tree from here
	while(tree[i] != '(')i++;

	List_list * l1 = createList_list();  // store the nodes and their existing list
	List_double * l2 = createList_double();  // store the MRCA of corresponding nodes

	int j = i;
	while(tree[j++] != '\0');
	tree[j-4] = '\0';  // for the "4": I guess there is a '\n' in the string befure '\0'

	// the following used only for IBD update in the while loop
	Node_list * node_list = NULL;
	Node_int * node_int = NULL;
	Node_double * node_double = NULL;
	double value;  // used by the tMRCA update

	while(tree[i] != '\0')
	{
		if(tree[i] == ',' || tree[i] == ' ' || tree[i] == ';')
		{
			i += 1;
			continue;
		}

		if(tree[i] == '(')
		{
			i += 1;
			appendList_list(l1);
			appendList_double(l2, 0);  // previous None in Python
			continue;
		}

		if(tree[i] >= 48 && tree[i] <= 57)
		{
			char node[5];
			int j = 0;
			
			// 24:0.0230624  or  24.1:0.0230624 are all possible, in which the additional "1" means the simulation
			// interation times which is not useful here
			while(tree[i] != '.' && tree[i] != ':')
			{
				node[j++] = tree[i++];
			}
			node[j] = '\0';
			long int node_value;

			// the sample name is from 1 or 0
			if(FROMONE)
			{
				node_value = string_long(node);
			}
			else
			{
				node_value = string_long(node) + 1;
			}

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

			appendList_int(l1, node_value);  // we fix this kind of appending only to the last list in the list

			if((l2->end)->element == 0)  // two sub-nodes
			{
				l2->end->element = tmrca_value;
			}

			//=============== tMRCA update =================
			j = 0;
			node_list = l1->start;
			node_int = (node_list->start)->start;
			node_double = l2->start;

			for(j=1; j<=l1->length; j++)
			{
				do
				{
					if(node_int == NULL)  // there may be an empty list
					{
						break;
					}
					long int node1 = node_int->element;
					if(node1 != node_value)
					{
						long int name;
						if(node1 < node_value)
						{
							name = (node1 - 1) * SAMPLE + node_value - 1;  // here the name should be an long integer
						}
						else
						{
							name = (node_value - 1) * SAMPLE + node1 - 1;
						}

						//============== update the working tables, and report IBD segment ===============
						double tMRCA = node_double->element;

						if(tMRCA - table_tMRCA[name] > TOLERANCE || tMRCA - table_tMRCA[name] < -TOLERANCE)
						{
							//============ get the sample pair =============
							name += 1;
							long int sample1, sample2;
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
							IBDreport(sample1, sample2, table_coordinate[name], coordinate);
							
							table_coordinate[name] = coordinate;
							table_tMRCA[name] = tMRCA;
						}
						//=============================================================================
					}
					if(node_int->next == NULL){break;}
					else{node_int = node_int->next;}
				}
				while(1);
				if(j<l1->length)
				{
					node_list = node_list->next;
					node_int = (node_list->start)->start;
					node_double = node_double->next;
				}
			}
			continue;
		}

		if(tree[i] == ')')  // there must be a ":" following the ")", because we have moved the last ")" away
		{
			// two possibilities: left leaf and right leaf
			extendList_list(l1);
			i += 2;
			char tmrca[20];  // I give this double type number 20 effective digits
			int j = 0;
			while(tree[i] != ',' && tree[i] != ')' && tree[i] != '\0')
			{
				tmrca[j++] = tree[i];
				i++;
			}
			tmrca[j] = '\0';

			value = string_double(tmrca);
			extendList_double(l2, value);
			continue;
		}
	}
	tree[i] = ')';  // retain the previous tree
	cleanList(l1, l2);

	return;
}


// input: the tree and the sample pair
// output: the tMRCA of that pair in that tree
double tMRCA_find(char * tree, long int sample1, long int sample2)
{
	// algorithm:
	// 1. process to get one of the chromosomes
	// 2. from now, go up to an internal node (MRCA node)
	// 3. close this internal node, and at the same time find the other chromosome
	// 4. if find, record and break; otherwise, jump to step#2
	// 5. as we must find the pair in this tree, we don't need to detect existing errors about the pair
	long int person1, person2;
	double tMRCA = 0;

	// get the start of the tree from here
	int i = 0;
	int j = 0;
	while(tree[i] != '(')i++;

	// get one of the chromosomes from here
	while(tree[i] != '\0')
	{
		if(tree[i] == ',' || tree[i] == ' ' || tree[i] == ';' || tree[i] == '(')
		{
			i++;
			continue;
		}

		if(tree[i] == ')')
		{
			i++;
			while(tree[i] != ',' && tree[i] != ')' && tree[i] != '\0')i++;
			continue;
		}

		if(tree[i] >= 48 && tree[i] <= 57)
		{
			char node[5];
			j = 0;

			// 24:0.0230624  or  24.1:0.0230624 are all possible, in which the additional "1" means the simulation
			// interation times which is not useful here
			while(tree[i] != '.' && tree[i] != ':')
			{
				node[j++] = tree[i++];
			}
			node[j] = '\0';
			long int node_value;

			// the sample name is from 1 or 0
			if(FROMONE)
			{
				node_value = string_long(node);
			}
			else
			{
				node_value = string_long(node) + 1;
			}

			if(node_value == sample1 || node_value == sample2)
			{
				if(node_value == sample1)
				{
					person1 = sample1;
					person2 = sample2;
				}
				else
				{
					person1 = sample2;
					person2 = sample1;
				}
				// get the tMRCA, then wait for the futher processing
				while(tree[i++] != ':');
				char tmrca[20];  // I give this double type number 20 effective digits; it is just kidding of course
				j = 0;
				while(tree[i] != ',' && tree[i] != ')' && tree[i] != '\0')
				{
					tmrca[j++] = tree[i];
					i++;
				}
				tmrca[j] = '\0';
				tMRCA = string_double(tmrca);
				// now the pointer points to a ',' or ')'
				break;
			}
			else
			{
				// simply jump beyond the tMRCA, then continue in this while loop
				while(tree[i] != ',' && tree[i] != ')' && tree[i] != '\0')i++;
			}
			continue;
		}
	}

	// finding, closing and moving up
	// start from ',' or ')', and now we have already had a tMRCA
	int finish = 0;
	while(tree[i] != '\0')
	{
		if(tree[i] == ',')  // there is a ',', we shoud perform finding and closing; if successful, break out
		{
			int mark = 1;
			i++;
			while(mark)
			{
				// if this is the target, jump out; otherwise, skimp this node
				if(tree[i] >= 48 && tree[i] <= 57)
				{
					// extract the node and judge
					char node[5];
					j = 0;

					// 24:0.0230624  or  24.1:0.0230624 are all possible, in which the additional "1" means the simulation
					// interation times which is not useful here
					while(tree[i] != '.' && tree[i] != ':')
					{
						node[j++] = tree[i++];
					}
					node[j] = '\0';
					long int node_value;

					// the sample name is from 1 or 0
					if(FROMONE)
					{
						node_value = string_long(node);
					}
					else
					{
						node_value = string_long(node) + 1;
					}

					if(node_value == person2)
					{
						finish = 1;
						break;
					}
					while(tree[i] != ',' && tree[i] != ')' && tree[i] != '\0')i++;
					continue;
				}

				if(tree[i] == ')')
				{
					mark -= 1;
					if(mark != 0)
					{
						i++;
						while(tree[i] != ',' && tree[i] != ')' && tree[i] != '\0')i++;
					}
					continue;
				}

				if(tree[i] == '(')
				{
					mark += 1;
					i++;
					continue;
				}

				i++;
			}

			if(finish)  // we find the other node, jump out the whole loop
			{
				break;
			}
		}
		else  // this is the ')', and we should move up
		{
			// moving up and continue in this while loop
			while(tree[i++] != ':');
			char tmrca[20];  // I give this double type number 20 effective digits; it is just kidding of course
			j = 0;
			while(tree[i] != ',' && tree[i] != ')' && tree[i] != '\0')
			{
				tmrca[j++] = tree[i];
				i++;
			}
			tmrca[j] = '\0';
			tMRCA += string_double(tmrca);
			// now the pointer points to a ',' or '), continue
		}
	}

	return tMRCA;
}

// end of Naive_parser.cpp