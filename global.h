/*
 * global.h
 *
 *  Created by Yanhua Liu for CS820 assignment 1
 *  Created on: Sep 6, 2013
 *  This header file externs the global variables
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_
#include "ospenv.h" /* this header defines POSIX, ISOC, XOPEN and EXTENSIONS */
extern unsigned int options_flags; /* work with switch options_flags */
extern int shift_table[MAX_ASCII]; /* shift table */
extern int line_buffer_size; /* work with -l number */
extern int max_line_number; /* work with -m number */
extern int column_number; /* work with -n mumber */
extern char *search_pattern; /* buffer to store the search string */
extern int thread_limits; /* work with -t number*/
extern int max_dir_depth; /* work with -d number */
#endif /* GLOBAL_H_ */
