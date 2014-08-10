// Parser.cpp
// function: the stack-based parsing algorithm; using the self-defined List and related operations
// 1.
// input:	the string of a tree (an in-order traversal format of the tree)
// output:	the table in which we can find the all-pair tMRCAs
// 2.
// input:	two trees
// output:	whether or not one specific pair change their tMRCA

#include <stdio.h>
#include "List.h"
#include "Parser.h"
#include "Basic.h"
#include "Global.h"
#include <string.h>
#include <unordered_map>


using namespace std;


// input: the tree
// output: updated all-pair tMRCAs table
long int parser(char * tree, double * table)
{
	// get the breakpoint of this tree (for Nexus, it is the start coordinate of this segment)
	long int coordinate;
	int i = 0;
	while(tree[i] != '\0')
	{
		if(tree[i] == 'p' && tree[i+1] == 'o' && tree[i+2] == 's' && tree[i+3] == '_')
		{
			char breakpoint[10];
			i = i+4;
			int j = 0;
			while(tree[i] != ' ')
			{
				breakpoint[j++] = tree[i++];
			}
			breakpoint[j] = '\0';
			coordinate = string_long(breakpoint);
			break;
		}
		i++;
	}

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

	return coordinate;
}


// input: tree
// output: the SAMPLE size of the tree
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
			while(tree[i] != '.')
			{
				node[j++] = tree[i++];
			}
			node[j] = '\0';
			long int node_value = string_long(node);
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
					while(tree[i] != '.')
					{
						node[j++] = tree[i++];
					}
					node[j] = '\0';
					long int node_value = string_long(node);
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


// input: two trees; one specific pair; tolerance for the comparison
// output: whether or not that pair has not changed tMRCA between these two trees
int tMRCA_compare(char * tree1, char * tree2, long int sample1, long int sample2)
{

	double tMRCA1, tMRCA2;

	tMRCA1 = tMRCA_find(tree1, sample1, sample2);
	tMRCA2 = tMRCA_find(tree2, sample1, sample2);

	// DEBUG the tMRCA_find results
	//printf("tMRCA from tree1: %f\n", tMRCA1);
	//printf("tMRCA from tree2: %f\n", tMRCA2);

	// compare the two tMRCAs and get the conclusion
	if(tMRCA1 - tMRCA2 <= TOLERANCE && tMRCA1 - tMRCA2 >= -TOLERANCE)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


// input: two trees
// output: the updated table, in which there are unchanged pairs between this two trees, and their tMRCA;
//			if that pair is unchanged, we use a -1 for it
void unchanged_pairs(char * tree1, char * tree2, double * result_pointer)
{
	// for the basic functional routines, it's better to use the HEAP rather than the STACK!!
	double * table1 = (double *)malloc(sizeof(double) * SAMPLE * SAMPLE);
    double * table2 = (double *)malloc(sizeof(double) * SAMPLE * SAMPLE);

	parser(tree1, table1);
	parser(tree2, table2);

	long int i, j;
	long int name;

	for(i = 1; i <= SAMPLE; i++)
		for(j = i+1; j <= SAMPLE; j++)
		{
			name = (i - 1) * SAMPLE + j - 1;
			if((table1[name] - table2[name] <= TOLERANCE) && (table1[name] - table2[name] >= -TOLERANCE))
			{
				result_pointer[name] = table1[name];
			}
			else
			{
				result_pointer[name] = -1;	// changed pair
			}
		}
	free(table1);
	free(table2);

	return;
}


// input: two trees
// output: the updated hashtable, in which there are unchanged pairs between this two trees, and their tMRCA
void unchanged_pairs_hash(char * tree1, char * tree2, unordered_map<long int, double> * result_pointer)
{
	(*result_pointer).clear();

	// for the basic functional routines, it's better to use the HEAP rather than the STACK!!
	double * table1 = (double *)malloc(sizeof(double) * SAMPLE * SAMPLE);
    double * table2 = (double *)malloc(sizeof(double) * SAMPLE * SAMPLE);

	parser(tree1, table1);
	parser(tree2, table2);

	long int i, j;
	long int name;

	for(i = 1; i <= SAMPLE; i++)
		for(j = i+1; j <= SAMPLE; j++)
		{
			name = (i - 1) * SAMPLE + j - 1;
			if((table1[name] - table2[name] <= TOLERANCE) && (table1[name] - table2[name] >= -TOLERANCE))
			{
				(*result_pointer)[name] = table1[name];
			}
		}
	free(table1);
	free(table2);

	return;
}


// input: two trees
// output: the updated hashtable, in which there are changed pairs between this two trees, and their new tMRCA
void changed_pairs_hash(char * tree1, char * tree2, unordered_map<long int, double> * result_pointer)
{
	// for the basic functional routines, it's better to use the HEAP rather than the STACK!!
	double * table1 = (double *)malloc(sizeof(double) * SAMPLE * SAMPLE);
    double * table2 = (double *)malloc(sizeof(double) * SAMPLE * SAMPLE);

	parser(tree1, table1);
	parser(tree2, table2);

	long int i, j;
	long int name;

	for(i = 1; i <= SAMPLE; i++)
		for(j = i+1; j <= SAMPLE; j++)
		{
			name = (i - 1) * SAMPLE + j - 1;
			if((table1[name] - table2[name] > TOLERANCE) || (table1[name] - table2[name] < -TOLERANCE))
			{
				(*result_pointer)[name] = table2[name];
			}
		}
	free(table1);
	free(table2);

	return;
}


// input: one tree
// output: the updated hashtable, in which there are all pairs with their tMRCA values
void all_pairs_hash(char * tree, unordered_map<long int, double> * result_pointer)
{
	// for the basic functional routines, it's better to use the HEAP rather than the STACK!!
	double * table = (double *)malloc(sizeof(double) * SAMPLE * SAMPLE);

	parser(tree, table);

	long int i, j;
	long int name;

	for(i = 1; i <= SAMPLE; i++)
		for(j = i+1; j <= SAMPLE; j++)
		{
			name = (i - 1) * SAMPLE + j - 1;
			(*result_pointer)[name] = table[name];
		}
	free(table);

	return;
}

// end of Parser.cpp