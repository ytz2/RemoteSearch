/*
 * command_util.c
 *
 *
 *  Created by Yanhua Liu for CS820 assignment 1
 *  Created on: Sep 5, 2013
 *  This header file provide the utilities to
 *  implement the getopt interface
 */

#include "command_util.h"

/* Scans the string pointed to by optarg and tries to convert it to a number.
 * Returns 0 if successful (and stores the number in result),
 *	  -1 on any error (prints an error message and leaves result unchanged)
 */

int scan_switch_number(int switch_char, int *result) {
	int temp, retval;
	char *ptr;

	errno = 0;
	temp = strtol(optarg, &ptr, 10);
	if (errno != 0 || ptr == optarg
			|| strspn(ptr, WHITE_SPACE) != strlen(ptr)) {
		fprintf(stderr, "Illegal numeric value \"%s\" for switch -%c\n", optarg,
				switch_char);
		retval = -1;
	} else {
		*result = temp;
		retval = 0;
	}
	return retval;
} /* scan_switch_number */

void print_flag(unsigned int flags, unsigned int this_one, char *name) {
	printf("\t%s is %s\n", name, (flags & this_one) ? "ON" : "OFF");
} /* print_flag */

/* If buffer ends with a new-line character, remove that character.
 * Returns number of characters remaining in buffer.
 */
int trim_line(char *buffer) {
	int len;

	len = strlen(buffer);
	if (len > 0 && buffer[len - 1] == '\n')
		buffer[--len] = '\0';
	return len;
} /* trim_line */
