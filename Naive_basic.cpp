// Naive_basic.cpp
// function: some basic functions which will be used by other parts of the whole program

#include "Naive_basic.h"
#include "Naive_global.h"
#include <math.h>
#include <stdio.h>      /* printf, scanf, NULL */


int string_int(char * string)  // used for the number of processes, and the tree FORMAT
{
	int i = 0;
	int sum = (int)(string[i++])-48;
	while(string[i] != '\0')
	{
		sum = sum * 10 + ((int)(string[i++]) - 48);
	}
	return(sum);
}


// transform the string format of a number to long int format
// for recombination breakpoint, which can be at most 100,000,000, so we use a "long int"
// also for the sample name; we also use the "long int" for it
long int string_long(char * string)
{
	int i = 0;
	long int sum = (long)(string[i++])-48;
	while(string[i] != '\0')
	{
		sum = sum * 10 + ((long)(string[i++]) - 48);
	}
	return(sum);
}


// for tMRCA transformation, from string to double
double string_double(char * string)
{
	int i = 0;
	double sum = ((double)(string[i++])-48);
	while(string[i] != '.')
	{
		sum = sum * 10 + ((double)(string[i++]) - 48);
	}
	i++;
	int n = i-1;
	while(string[i] != '\0')
	{
		sum = sum + ((double)(string[i])-48)/pow(10, (i-n));
		i++;
	}
	return(sum);
}


// print the all-pair tMRCAs
void print_MRCA(double * table, long int SAMPLE)
{
	long int i,j;
	for(i = 1; i <= SAMPLE; i++)
		for(j = i+1; j <= SAMPLE; j++)
		{
			long int name = (i - 1) * SAMPLE + j - 1;
			printf("pair '%ld'--'%ld' has a tMRCA as %f\n", i, j, table[name]);
		}
	return;
}


// judge whether or not this line is a tree
int tree_judge(char * tree)
{
	if(!FORMAT)	// the format of the tree: Nexus(0) or Newick(1)
	{
		// Nexus tree
		if(tree[1] == 't' && tree[2] == 'r' && tree[3] == 'e' && tree[4] == 'e' && tree[5] == ' ')
			return 1;
		else
			return 0;
	}
	else
	{
		// Newick tree
		if(tree[0] == 'N' && tree[1] == 'E' && tree[2] == 'W' && tree[3] == 'I' && tree[4] == 'C' && tree[5] == 'K')
			return 1;
		else
			return 0;
	}
}


// get the coordinate of present tree
long int get_coordinate(char * tree)
{
	if(!FORMAT)	// Nexus
	{
		int i = 0;
		while(1)
		{
			if(tree[i] == 'p' && tree[i+1] == 'o' && tree[i+2] == 's' && tree[i+3] == '_')
			{
				i += 4;
				break;
			}
			i++;
		}

		char buffer[10];
		int j = 0;
		while(tree[i] != ' ')
		{
			buffer[j++] = tree[i];
			i++;
		}
		buffer[j] = '\0';

		return string_long(buffer);
	}
	else
	{
		int i = 0;
		while(tree[i++] != '[');
		char buffer[10];
		int j = 0;
		while(tree[i] != ']')
		{
			buffer[j++] = tree[i];
			i++;
		}
		buffer[j] = '\0';

		return string_long(buffer);
	}
}


void IBDreport(long int sample1, long int sample2, long int coordinate1, long int coordinate2)
{
	if(coordinate2 - coordinate1 < CUTOFF)
		return;
	else
	{
		char buffer[50];
		sprintf(buffer, "%ld-%ld [%ld,%ld]\n", sample1, sample2, coordinate1, coordinate2);
		// we should use this global locker to control the writing behaviour
		pthread_mutex_lock(&mut_write);
		fputs(buffer, file_out);
		pthread_mutex_unlock(&mut_write);
		return;
	}
}
// end of Naive_basic.cpp