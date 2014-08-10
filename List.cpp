// List.cpp
// function: the basic operations (initialization, appending, extending, cleaning) for the List

#include "List.h"
#include <stdio.h>      /* printf, scanf, NULL */
#include <stdlib.h>     /* malloc, free, rand */


using namespace std;


List_list * createList_list()
{
	List_list * lHead;
	lHead = (List_list *)malloc(sizeof(List_list));
	lHead -> start = NULL;
	lHead -> end = NULL;
	lHead -> length = 0;
	return(lHead);
}

List_int * createList_int()
{
	List_int * lHead;
	lHead = (List_int *)malloc(sizeof(List_int));
	lHead -> start = NULL;
	lHead -> end = NULL;
	lHead -> length = 0;
	return(lHead);
}

List_double * createList_double()
{
	List_double * lHead;
	lHead = (List_double *)malloc(sizeof(List_double));
	lHead -> start = NULL;
	lHead -> end = NULL;
	lHead -> length = 0;
	return(lHead);
}

// we only support appending an empty list (add length 1 of course) in this function
void appendList_list(List_list * list)
{
	Node_list * node;
	node = (Node_list *)malloc(sizeof(Node_list));
	node -> start = NULL;
	node -> next = NULL;

	List_int * list1;
	list1 = (List_int *)malloc(sizeof(List_int));
	list1 -> start = NULL;
	list1 -> end = NULL;
	list1 -> length = 0;

	node -> start = list1;

	if(list->start == NULL)
	{
		list -> start = node;
		list -> end = node;
		(list -> length)++;
	}
	else
	{
		((list -> end) -> next) = node;
		(list -> end) = node;
		(list -> length)++;
	}
	return;
}

// we only support to append an unsigned int to the last List_int in the List_list, because we only
// need this
void appendList_int(List_list * list, long int value)
{
	Node_int * node;
	node = (Node_int *)malloc(sizeof(Node_int));
	node -> element = value;
	node -> next = NULL;


	if(((list->end)->start)->start == NULL)
	{
		((list->end)->start)->start = node;
		((list->end)->start)->end = node;
		((list->end)->start)->length++;
	}
	else
	{
		(((list->end)->start)->end)->next = node;
		((list->end)->start)->end = node;
		((list->end)->start)->length++;
	}
	return;
}

void appendList_double(List_double * list, double value)
{
	Node_double * node;
	node = (Node_double *)malloc(sizeof(Node_double));
	node -> element = value;
	node -> next = NULL;

	if(list->start == NULL)
	{
		list -> start = node;
		list -> end = node;
		(list -> length)++;
	}
	else
	{
		((list -> end) -> next) = node;
		(list -> end) = node;
		(list -> length)++;
	}
	return;
}

void extendList_list(List_list * list)
{
	Node_list * node_list_last = list -> start;
	int i = 1;
	if(list->length > 2)
	{
		while(i < (list->length)-1)
		{
			node_list_last = node_list_last->next;
			i++;
		}
	}

	if(list->end->start->length == 0)  // not possible for this case
	{
		free(list->end->start);
		free(list->end);
		list->end = node_list_last;
		list->length -= 1;
	}
	else
	{
		if(node_list_last->start->length == 0)
		{
			free(node_list_last->start);
			node_list_last->start = list->end->start;
			free(list->end);
			list->end = node_list_last;
			list->length -= 1;
		}
		else
		{
			node_list_last->start->end->next = list->end->start->start;
			node_list_last->start->end = list->end->start->end;
			node_list_last->start->length = node_list_last->start->length + list->end->start->length;
			free(list->end->start);
			free(list->end);
			list->end = node_list_last;
			list->length -= 1;
		}
	}
	return;
}

void extendList_double(List_double * list, double value)
{
	Node_double * node_double_last = list -> start;
	int i = 1;
	if(list->length > 2)
	{
		while(i < (list->length)-1)
		{
			node_double_last = node_double_last->next;
			i++;
		}
	}

	if(node_double_last->element == 0)  // left leaf
	{
		node_double_last->element = list->end->element + value;
	}

	// right leaf
	free(list->end);
	list->end = node_double_last;
	list->length -= 1;
	return;
}

void cleanList(List_list * l1, List_double * l2)
{
	// by now, the start and end of these two lists are all pointing to the first element in those lists
	Node_int * node_int_last = l1->start->start->start;
	Node_int * node_int = node_int_last->next;
	free(node_int_last);
	while(node_int->next != NULL)  // we have more than 2 elements in this List_int
	{
		node_int_last = node_int;
		node_int = node_int_last->next;
		free(node_int_last);
	}
	free(node_int);
	free(l1->start->start);
	free(l1->start);  // the start and the end are all pointing to the same Node_list
	free(l1);

	free(l2->start);
	free(l2);
	return;
}

// end of List.cpp