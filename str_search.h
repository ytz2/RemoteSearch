/*
 * str_search.h
 *
 *  Created by Yanhua Liu for CS820 assignment 1
 *  Created on: Sep 5, 2013
 *  This Header files defines function utilities
 *  to implement B-M algorithm and other supporting
 *  functions
 */

#ifndef STR_SEARCH_H_
#define STR_SEARCH_H_
#include "ospenv.h" /* this header defines POSIX, ISOC, XOPEN and EXTENSIONS */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>
#include "command_util.h"
#define MAX_ASCII 256
/*
 * accept an ascii table contains MAX_ASCII elements
 * fill the table with relative shift
 */
void build_shift_table(int* table, char* search_string, unsigned int flag);

/*
 * compare two char array if same return 1 else 0
 */
int cmp_char_arr(char* a, char *b, int len, unsigned int flag);

/*
 * Implement the Boyer-Moore Algorithm to check
 * if a given string a substring.
 * For simplicity, we have built the shift table
 * buffer: the object string to be searched
 * search_string: the substring
 * table: the already built shift table
 */
int boyer_moore(char* buffer, char* search_string, int *table,
		unsigned int flag);

/*search the search_string in buffer at begin/end*/
int search_begin(char* buffer, char* search_string, unsigned int flag);
int search_end(char* buffer, char* search_string, unsigned int flag);

/*the search_string exactly match the buffer string*/
int exact_match(char* buffer, char *search_string, unsigned int flag);

#endif /* STR_SEARCH_H_ */
