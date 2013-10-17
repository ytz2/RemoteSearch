/*
 * command_util.h
 *
 *  Created by Yanhua Liu for CS820 assignment 1
 *  Created on: Sep 5, 2013
 *  This header file provide the utilities to
 *  implement the getopt interface
 */

#ifndef COMMAND_UTIL_H_
#define COMMAND_UTIL_H_
#include "ospenv.h"/* this header defines POSIX, ISOC, XOPEN and EXTENSIONS */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
/* predefine parameters to work with getopt interface*/

#define OPTIONS ":hbeipvl:m:n:"
#define WHITE_SPACE " \n\r\t\v\f"
#define AT_BEGIN            0x0001  /* -b switch */
#define AT_END              0x0002  /* -e switch */
#define CASE_SENSITIVE	    0x0004  /* -i switch */
#define SHOW_PATH           0x0008  /* -p switch */
#define INVERSE_PRINT       0x0010  /* -v default is */
/* magic number definition */
#define DEFAULT_LINE_BUFFER 255
#define MAX_LINE_BUFFER     4096
#define MAX_COLS            16
#define MAX_FILES           1024
#define STREAM_REDIRECT     "-"

/* Function Utilities*/

/* Scans the string pointed to by optarg and tries to convert it to a number.
 * Returns 0 if successful (and stores the number in result),
 *	  -1 on any error (prints an error message and leaves result unchanged)
 */

int scan_switch_number(int switch_char, int *result);

/* If buffer ends with a new-line character, remove that character.
 * Returns number of characters remaining in buffer.
 */
int
trim_line(char *buffer);

/* print the flag value, this is used to debug*/
void print_flag(unsigned int flags, unsigned int this_one, char *name);

#endif /* COMMAND_UTIL_H_ */
