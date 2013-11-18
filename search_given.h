/*
 * Yanhua Liu (ytz2) CS820
 * general_search.h
 *
 *  Created on: Nov 11, 2013
 *      Author: yanhualiu
 */

#ifndef SEARCH_GIVEN_H_
#define SEARCH_GIVEN_H_
#include "ospenv.h" /* this header defines POSIX, ISOC, XOPEN and EXTENSIONS */
#include "command_util.h"
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include "str_search.h"
#include "rpath.h"
#include <pthread.h>
#include "dirHandle.h"
#include "plcsIO.h"

/*
 * a more general search to search a given name from level 0
 */

void search_given(char *given, search *mysearch);

#endif /* GENERAL_SEARCH_H_ */
