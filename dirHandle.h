/*
 * dirHandle.h
 * For CS820 Assignment 2
 * Created on: Oct 17, 2013
 * Author: Yanhua Liu
 *
 *  This header file defines the history stack
 *  to work with directory search, the directory
 *  search functions
 */

#ifndef DIRHANDLE_H_
#define DIRHANDLE_H_

#include "ospenv.h"/* this header defines POSIX, ISOC, XOPEN and EXTENSIONS */
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include "rpath.h"
/* define the element structure for history stack */

typedef struct NODE
{
	unsigned long counter; // counter of reference to itself
	char *path; //realpath
	DIR *dir; //the dir object
	struct NODE *prev; // link to its previous node
} Node;

typedef struct STACK
{
	Node *head;
	pthread_rwlock_t s_lock;
} stack;

/*
 * make a history stack node
 */
Node* make_node(char *dname)
{
	Node* temp;
	DIR *dir;
	char* rptr,*memptr;

	errno=0;
	if((dir=opendir(dname))==NULL)
	{
		perror("Opendir");
		return NULL;
	}
	if ((rptr = get_realpath(dname,&memptr))==NULL)
	{
		perror("realpath");
		if (memptr)
			free(memptr);
		return NULL;
	}

	if ((temp=(Node*)malloc(sizeof(Node)))==NULL)
	{
		perror("malloc()");
		return NULL;
	}
	temp->counter=1;
	temp->dir=dir;
	temp->path=memptr;
	temp->prev=NULL;
	return temp;
}

/*
 * Initialize a stack
 */
int stack_init(stack *st);

/*
 * push a node on the "top" of the tree-like stack
 */
int stack_push(stack *st,Node *prev, Node *current);

/*
 * "pop" a node, decrement the counter once a job get
 * finished, the "pop" will happen only when counter==0
 */
int stack_job_done(stack *st,Node *current);


/*
 *  find the realpath which used to appear in the tree
 */
Node* stack_find_history(stack *st,Node* current,char *path);

/*
 * recursively walk the directory
 */
int walk_recur(char* dname, int depth);
#endif /* DIRHANDLE_H_ */
