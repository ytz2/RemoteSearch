/*
 * Yanhua Liu (ytz2) CS820
 *print_time.c - function to get time differences in microseconds */

#include "print_time.h"

/*
 * get the time difference in microsends precision
 *
 */
double time_diff(time_type *start, time_type *done) {
	int secs, fraction;
	double delta;

	secs = done->tv_sec;
	fraction = done->fraction_field;
	if (start != NULL) {
		secs -= start->tv_sec;
		fraction -= start->fraction_field;
		if (fraction < 0) {
			secs--;
			fraction += PER_SECOND;
		}
	}
	delta = ((double) secs) + ((double) fraction) / ((double) PER_SECOND);
	return delta;
}

/* vi: set autoindent tabstop=8 shiftwidth=8 : */
