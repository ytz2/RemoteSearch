/*
 * Yanhua Liu (ytz2) CS820
 *  print_time.h - header file for
 *		  program to print out time differences in microseconds
 */

#ifndef PRINT_TIME_H
#define PRINT_TIME_H

#include "ospenv.h"
#include <unistd.h>
#include <stdio.h>
#if defined(_POSIX_TIMERS) && _POSIX_TIMERS != -1

	/* use the newer Posix high-resolution time facilities */
	/* must be linked on all machines with -lrt */

	#include <time.h>

	#define PER_SECOND	1000000000
	#define time_type	struct timespec
	#define fraction_field	tv_nsec
	#define get_time(where)	clock_gettime(CLOCK_REALTIME, where)

#else

	/* use the older (BSD) high-resolution time facilities */

	#include <sys/time.h>

	#define PER_SECOND	1000000
	#define time_type	struct timeval
	#define fraction_field	tv_usec
	#define get_time(where)	gettimeofday(where, NULL)

#endif



/*
 * get the time difference in double between two time value
 */
extern double
time_diff( time_type *before, time_type *done);

#endif

