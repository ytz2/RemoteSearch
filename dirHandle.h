/*
 * Yanhua Liu (ytz2) CS820
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
#include "command_util.h"
#include "plcsIO.h"
#include "thread_share.h"

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
Node* stack_find_history(stack *st, Node* current, Node *next);


/*
 * Walk through a dir from the current node
 */
int walk_recur(Node* current);

/*
 * test if the symlink pointing to a dir
 * if it is, return bool 1
 * else false 0
 */
int is_sym_dir(char* full_name, search *mysearch, Node *current);

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
