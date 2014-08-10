// Basic.cpp
// function: some basic functions which will be used by other parts of the whole program

#include "Basic.h"
#include "Global.h"
#include <math.h>
#include <stdio.h>      /* printf, scanf, NULL */
#include <string>


using namespace std;


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


// get the hashed part of the tMRCA value
// precision is what we want to keep; simply drop all the digits after the precision digit to
// get the result
double partial_tMRCA(double tMRCA, double precision)
{
	// CAUTION HERE!!
	// we give all the tMRCAs an 1.0 off to avoid 0.xxx
	tMRCA += 1.0;

	char buffer[20];
	sprintf(buffer, "%f", tMRCA);

	// if the tMRCA is 0.xxx, we give them an additional 1
	//if(tMRCA[0] == '0')
	//{
	//	tMRCA[0] = '1';
	//}

	int i = 0;
	while(buffer[i++] != '.');
    while(precision != 1)
    {
        i++;
        precision *= 10;
    }
    buffer[i] = '\0';

    double tmrca = string_double(buffer);
    //return int((tmrca) * (1/PRECISION)) * 1.0 / 10;
    return tmrca;
}


// transfer the int-int to a string, as the hashing key
void intdashint(int leaf1, int leaf2, string * key_pointer)
{
	char buffer[20];
	sprintf(buffer, "%d-%d", leaf1, leaf2);
	int i = 0;
	while(buffer[i] != '\0')
	{
		(*key_pointer).push_back(buffer[i]);
		i++;
	}
	return;
}


// judge whether or not this line is a tree
int tree_judge(char * tree)
{
	// Nexus tree
	if(tree[1] == 't' && tree[2] == 'r' && tree[3] == 'e' && tree[4] == 'e' && tree[5] == ' ')
		return 1;
	else
		return 0;
}


// get the coordinate of present tree
long int get_coordinate(char * tree)
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
// end of Basic.cpp