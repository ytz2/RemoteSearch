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
#include "global.h"
#include "command_util.h"
#include "plcsIO.h"

/* define the element structure for history stack */
typedef struct STACK stack; //pre declaration
typedef struct NODE {
	unsigned long counter; // counter of reference to itself
	char *path; //realpath
	DIR *dir; //the dir object
	struct NODE *prev; // link to its previous node
	int depth;
	stack *stk;
} Node;

/*
 * the stack type is made independent of Node
 * in order to provide a better interface
 */
struct STACK {
	/*the head or root is useless, but leave it here in case useful tomorrow*/
	Node *head;
	/*
	 * the stack is protected by rwlock, since most of time is not doing push
	 * but doing the searching whether a realpath used to appear
	 */
	pthread_rwlock_t s_lock;
	/*
	 * the attribute will be set to be detached to make sure onece
	 * a thread finished, the resource of that thread releases in time.
	 */
	pthread_attr_t attr;
};

/*
 * make a history stack node
 */
Node* make_node(char *dname, int depth, stack *stk);

/*
 * do some housekeeping about a node
 */
void clear_node(Node* node);

/*
 * Initialize a stack
 */
int stack_init(stack *st);

/*
 * destroy the stack
 */

int stack_destroy(stack *st);

/*
 * push a node on the "top" of the tree-like stack
 */
int stack_push(stack *st, Node *prev, Node *current);

/*
 * "pop" a node, decrement the counter once a job get
 * finished, the "pop" will happen only when counter==0
 */
int stack_job_done(stack *st, Node *current);

/*
 *  find the realpath which used to appear in the tree
 */
Node* stack_find_history(stack *st, Node* current, char *path, char *fullpath);

/*
 * get the full path from the realpath and d_name
 */
char* get_fullpath(char* rpath, char *fname);

/*
 * Walk through a dir from the current node
 */
int walk_recur(Node* current);

/*
 * test if the symlink pointing to a dir
 * if it is, return bool 1
 * else false 0
 */
int is_sym_dir(char* full_name);

/*
 * wrapper function for directory search
 * to let a thread call it
 */
void* search_dir(void *para);

/*
 * another wrapper function, it will
 * test if the alive threads
 * number is less than the limit, then do thread_create
 * else, use walk_recur
 */
int walk_to_next(Node* next);
#endif /* DIRHANDLE_H_ */
