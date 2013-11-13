/*
 * plcsIO.h
 *
 *  Created by Yanhua Liu for CS820 assignment 1
 *  Created on: Sep 6, 2013
 *  This Header files defines function utilities
 *  to implement IO reading & printing functions
 *  used in plcs
 *
 *  History:
 *  1. the get_realpath function is moved to rpath.h
 *  HW2:
 *  1 the any_line_buffer becomes a pthread_key
 *  2 add a wrapper function search_file for convenience
 */

#ifndef PLCSIO_H_
#define PLCSIO_H_
#include "ospenv.h" /* this header defines POSIX, ISOC, XOPEN and EXTENSIONS */
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include "command_util.h"
#include "str_search.h"
#include "rpath.h"
#include <pthread.h>
#include "dirHandle.h"
#include "send_recv.h"
#include "thread_share.h"
#include <stdarg.h>
/*
 * function to create the key
 */
void thread_init();

/*
 * search_str is a wrapper function wraps the options searches
 * defined in homework. -b -e -i and -v are handled
 */

int search_str(char* buffer, search *mysearch);

/*
 * print_line
 * print the str to standard output as the specified cols
 * will write realpath: lineno: str or if realpath is NULL
 * write: lineno: str
 */
void print_line(int lineno, char* realpath, char* str,int colum_number);

/*
 * search_stream
 * accept stream ptr either file or stdin
 * read each line specified by -p switch
 * or default value into any_line_buffer
 * search and print it
 */
void search_stream(FILE *input, char* filename, search *mysearch);


/*
 * search_file
 * accept a filename and a search string to perform a search
 * but add another flag to indicate it is in sub directory
 * tow work well with -q
 */
void search_file(char* filename, search *mysearch,int flag);

/*
 * wrapper function build the shift talbe
 */
void build_shifttable(search *mysearch);

/*
 * set err msg to client
 */
void send_err_line(search *mysearch,char *format,...);

#endif /* PLCSIO_H_ */
