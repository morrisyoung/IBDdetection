// Naive_parser.h

#ifndef NAIVE_PARSER_H
#define NAIVE_PARSER_H


// input: the tree
// output: coordinate; updated all-pair tMRCAs table
void parser(char *, double *);

// get the SAMPLE size automatically
long int parser_sample(char *);

void naive_update(long int, char *, long int *, double *);

void stdin_update(long int, char *, long int *, double *);

double tMRCA_find(char *, long int, long int);

#endif

// end of Naive_parser.h