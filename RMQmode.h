// RMQmode.h
// to find the RMQ +-1 mode in the beginning of the program (right after we have the number of chromosomes)

#ifndef RMQ_MODE_H
#define RMQ_MODE_H

#include <string>


using namespace std;


int Blockget_min(int, int, string);

void Blockmode_fill(string, int);

void Blockmode_last_fill(string, int);

void RMQmode();  // use SAMPLE to initialize all above

#endif

// end of RMQmode.h