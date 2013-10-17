/*
 * plcsIO.h
 *
 *  Created by Yanhua Liu for CS820 assignment 1
 *  Created on: Sep 6, 2013
 *  This Header files defines function utilities
 *  to implement IO reading & printing functions
 *  used in plcs
 */

#ifndef PLCSIO_H_
#define PLCSIO_H_
#include "ospenv.h" /* this header defines POSIX, ISOC, XOPEN and EXTENSIONS */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include "command_util.h"
#include "str_search.h"
#include "global.h"

/*
 * search_str is a wrapper function wraps the options searches
 * defined in homework. -b -e -i and -v are handled
 */

int search_str(char* buffer, char* search_str);

/*
 * print_line
 * print the str to standard output as the specified cols
 * will write realpath: lineno: str or if realpath is NULL
 * write: lineno: str
 */
void print_line(int lineno, char* realpath, char* str);

/*
 * get_realpath
 * return the realpath of a file
 */
char* get_realpath(char* filename, char** memptr);

/*
 * search_stream
 * accept stream ptr either file or stdin
 * read each line specified by -p swithc
 * or default value into any_line_buffer
 * search and print it
 */
void search_stream(FILE *input, char* filename, char* searhc_str);

#endif /* PLCSIO_H_ */
