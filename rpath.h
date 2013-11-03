/* Yanhua Liu
 * For CS820 Assignment 2
 * rpath.h
 *
 *  Created on: Oct 18, 2013
 *      Author: Yanhua Liu
 *
 * Note: this is a edition of
 * Assignment 1 version for the convenience
 * of assignment 2
 */

#ifndef RPATH_H_
#define RPATH_H_

#include "ospenv.h" /* this header defines POSIX, ISOC, XOPEN and EXTENSIONS */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#define PATH_MAX_GUESS 8192

/*
 * This function is edited from the version in Steven's book page 50
 * it returns the allocated buffer and the allocated
 * buffer size in bytes, in order to turn off the compiling warning,
 * I moved the global variable inside. In addtion, since we have defined
 * the cstd and posix version, we do not need to worry too much
 */
char* path_alloc(size_t *sizep);

/*
 * get_realpath
 * return the realpath of a file
 */
char* get_realpath(char* filename, char** memptr);

#endif /* RPATH_H_ */
