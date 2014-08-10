// RMQmode.cpp
// RMQ mode filling related functions
// this function should only be called once after we know the number of SAMPLE

#include "RMQmode.h"
#include "Global.h"
#include <unordered_map>
#include <math.h>
#include <string>
#include <iostream>


using namespace std;


// type: n^2 submode filling
// input: (i, j) i<>j, the differential lists: "010101110..."
// output: the index in which there is a minimum value among list[>=i, <=j]
// note: the index is the position in the previous block
int Blockget_min(int i, int j, string key)
{
	int index = i;
	int initial = 0;
	int actual = initial;

	for(int n = i + 1; n < j + 1; n++)
	{
		if(key.at(n-1) == '0')
		{
			actual -= 1;
		}
		else
		{
			actual += 1;
		}

		if(actual < initial)
		{
			initial = actual;
			index = n;
		}
	}

	return index;
}


// type: recursive mode_fill function
// input: the number of bits in this mode
// output: the mode directory for the mode with this length
void Blockmode_fill(string key, int N)
{
	if(N != 2)  // N = 2 is the last possible bit
	{
		key.push_back('0');
		Blockmode_fill(key, N-1);
		key.back() = '1';
		Blockmode_fill(key, N-1);
	}
	else
	{
		key.push_back('0');
		
		// fill this mode
		unordered_map<string, int> temp;
		MODE_BLOCK[key] = temp;

		int total = key.length() + 1;  // this is the BLOCK_SIZE
		for(int i = 0; i < total; i++)
			for(int j = i; j < total; j++)
			{
				string query;
				query.push_back(i + 48);  // transfer to ASCII character
				query.push_back('-');
				query.push_back(j + 48);  // transfer to ASCII character

				MODE_BLOCK[key][query] = Blockget_min(i, j, key);
			}

		key.back() = '1';

		// fill this mode
		unordered_map<string, int> temp1;
		MODE_BLOCK[key] = temp1;

		total = key.length() + 1;  // this is the BLOCK_SIZE
		for(int i = 0; i < total; i++)
			for(int j = i; j < total; j++)
			{
				string query;
				query.push_back(i + 48);  // transfer to ASCII character
				query.push_back('-');
				query.push_back(j + 48);  // transfer to ASCII character

				MODE_BLOCK[key][query] = Blockget_min(i, j, key);
			}
	}
	return;
}


// the same function with the above one, but specially for the BLOCK_MODE_LAST
void Blockmode_last_fill(string key, int N)
{
	if(N != 2)  // N = 2 is the last possible bit
	{
		key.push_back('0');
		Blockmode_last_fill(key, N-1);
		key.back() = '1';
		Blockmode_last_fill(key, N-1);
	}
	else
	{
		key.push_back('0');
		
		// fill this mode
		unordered_map<string, int> temp;
		MODE_BLOCK_LAST[key] = temp;

		int total = key.length() + 1;  // this is the BLOCK_SIZE
		for(int i = 0; i < total; i++)
			for(int j = i; j < total; j++)
			{
				string query;
				query.push_back(i + 48);  // transfer to ASCII character
				query.push_back('-');
				query.push_back(j + 48);  // transfer to ASCII character

				MODE_BLOCK_LAST[key][query] = Blockget_min(i, j, key);
			}

		key.back() = '1';

		// fill this mode
		unordered_map<string, int> temp1;
		MODE_BLOCK_LAST[key] = temp1;

		total = key.length() + 1;  // this is the BLOCK_SIZE
		for(int i = 0; i < total; i++)
			for(int j = i; j < total; j++)
			{
				string query;
				query.push_back(i + 48);  // transfer to ASCII character
				query.push_back('-');
				query.push_back(j + 48);  // transfer to ASCII character

				MODE_BLOCK_LAST[key][query] = Blockget_min(i, j, key);
			}
	}
	return;
}


// RMQ mode filling function
void RMQmode()
{
	int N = 4 * SAMPLE - 3;
	BLOCK_SIZE = int((log(N)/log(2))/2) + 1;

	// TODO check whether or not there is no last block??!! --> no bothering
	if(( (N*1.0/BLOCK_SIZE) - int(N*1.0/BLOCK_SIZE) ) == 0.0)
	{
		BLOCK_NUM = int(N*1.0/BLOCK_SIZE);
		BLOCK_SIZE_LAST = BLOCK_SIZE;
	}
	else
	{
		BLOCK_NUM = int(N*1.0/BLOCK_SIZE) + 1;
		BLOCK_SIZE_LAST = N - BLOCK_SIZE*(BLOCK_NUM-1);

		// we should avoid that the last block may have only one element in
		if(BLOCK_SIZE_LAST == 1)
		{
			//BLOCK_NUM -= 1;
			//BLOCK_SIZE_LAST = BLOCK_SIZE + 1;
			BLOCK_SIZE_LAST += 1;
		}
	}

	// fill all the block modes
	string key;
	// TODO when the BLOCK_SIZE is too small, it may not work (due to the recursive Blockmode_fill)
	Blockmode_fill(key, BLOCK_SIZE);

	key.clear();
	Blockmode_last_fill(key, BLOCK_SIZE_LAST);

	return;
}

// end of RMQmode.cpp