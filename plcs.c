/*
 * plcs.c
 *
 *  Created by Yanhua Liu for CS820 assignment 1
 *  Created on: Sep 5, 2013
 *  This program is executed as :
 *  plcs [options] search_string [list of input file names]
 *  It search the "search_string" in "list of input file names"
 *  according to "options"
 *  History:
 *  Few option parameter bugs fixed
 */
#include "ospenv.h" /* this header defines POSIX, ISOC, XOPEN and EXTENSIONS */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "command_util.h"
#include "global.h"
#include "plcsIO.h"

/*
 * The following parameters are made global
 * they are set via switches and interfaces
 * with plcsIO and is externed by global.h
 * any usage of these global prameters
 * must include "global.h"
 */

unsigned int options_flags; /* work with switch options_flags*/
int shift_table[MAX_ASCII]; /* shift table*/
char *any_line_buffer; /* buffer to store lines no matter what*/
int line_buffer_size; /* work with -l number*/
int max_line_number; /* work with -m number*/
int column_number; /* work with -n number*/

/*main function*/
int main(int argc, char *argv[]) {

	int c; /* switch character */
	int state, monitor_state; /* Error state during switch processing*/
	char *search_pattern; /* the search pattern*/
	char *temp_str; /* temporary string*/
	FILE *fptr; /* stream*/
	/* initialize all the options to their default values */
	options_flags = CASE_SENSITIVE; /* default is that inver printing OFF */
	line_buffer_size = DEFAULT_LINE_BUFFER;
	max_line_number = -1;
	column_number = -1;
	state = 1;
	any_line_buffer = NULL;
	search_pattern = NULL;
	monitor_state = 0;
	/* process all the command line switches */
	opterr = 0; /* prevent getopt() from printing error messages */

	while ((c = getopt(argc, argv, OPTIONS)) != -1) {
		switch (c) {
		case 'h': /* print help info */
			printf("-h: this is help information\n");
			printf("-b: select lines only matched at begin (default OFF)\n");
			printf("-e: select lines only matched at end (default OFF)\n");
			printf("-i: turn case sensitive OFF (default ON)\n");
			printf("-l512: set the size of buffer to 512\n"
					"\t(default %d and must less than %d)\n",
					DEFAULT_LINE_BUFFER, MAX_LINE_BUFFER);
			printf("-p: include the real path of file\n");
			printf("-l25: Segmenting a line buffer into sublines which contains 25 chars\n");
			printf("-m50: set the maximum number of lines to\n "
					"\tbe processes to 50 \n");
			printf("-n8: set the column of format number to 8 (max is 16)\n");
			break;
		case 'b': /* turn match at begin on */
			options_flags |= AT_BEGIN;
			break;
		case 'e': /* turn match at end on */
			options_flags |= AT_END;
			break;
		case 'i': /* turn case sensitive off */
			options_flags &= ~CASE_SENSITIVE;
			break;
		case 'p': /* include the real path*/
			options_flags |= SHOW_PATH;
			break;
		case 'v': /*invert the selection*/
			options_flags |= INVERSE_PRINT;
			break;
		case 'l': /* set buffer size */
			monitor_state = scan_switch_number(c, &line_buffer_size);
			if (line_buffer_size <= 0 || line_buffer_size >= MAX_LINE_BUFFER) {
				fprintf(stderr, "Illegal numeric value \"%d\" for switch -l\n",
						line_buffer_size);
				line_buffer_size = DEFAULT_LINE_BUFFER;
				state = -1;
			} else if (monitor_state == -1) {
				state = -1;
				monitor_state = 0;
			}
			break;
		case 'm': /* set maximum lines to be processed */
			monitor_state = scan_switch_number(c, &max_line_number);
			if (max_line_number <= 0) {
				fprintf(stderr, "Illegal numeric value \"%d\" for switch -m\n",
						max_line_number);
				max_line_number = -1;
				state = -1;
				monitor_state = 0;
			} else if (monitor_state == -1) {
				state = -1;
				monitor_state = 0;
			}
			break;
		case 'n': /* set column number to print line number */
			monitor_state = scan_switch_number(c, &column_number);
			if (column_number >= MAX_COLS || column_number<=0) {
				fprintf(stderr, "Illegal numeric value \"%d\" for switch -n\n",
						column_number);
				column_number = -1;
				state = -1;
				monitor_state = 0;
			} else if (monitor_state == -1) {
				state = -1;
				monitor_state = 0;
			}
			break;
		case ':':
			fprintf(stderr, "Missing parameter to switch '%c'\n", optopt);
			state = -1;
			break;
		case '?':
			fprintf(stderr, "Illegal switch '%c'\n", optopt);
			state = -1;
			break;
		} /* switch */
	}

	if (state == -1)
		return EXIT_FAILURE;

	/* now process the search_string given in command*/
	if (optind < argc) {
		if ((search_pattern = (char*) malloc(strlen(argv[optind]) + 1)) == NULL) {
			perror("Allocate space to search_pattern\n");
			return EXIT_FAILURE;
		}
		strcpy(search_pattern, argv[optind]);
		++optind;
	} else {
		/*if there is no search_string, issue error and exit*/
		fprintf(stderr, "search_string is not provided as an argument\n");
		return 1;
	}

	/*if both -b and -e are not switched on, build the shit table*/
	/*for BM algorithm to implement*/
	if (!(options_flags & AT_BEGIN) && !(options_flags & AT_BEGIN)
			&& strlen(search_pattern))
		build_shift_table(shift_table, search_pattern,
				!(options_flags & CASE_SENSITIVE));
	/* allocate memory to buffer*/
	if ((any_line_buffer = (char*) malloc(line_buffer_size + 1)) == NULL) {
		perror("Memory Allocation to Line Buffer\n");
		return 1;
	}

	/*
	 * if there is no argument in list of files
	 * directly go to stdin
	 */
	if (optind >= argc) {
		search_stream(stdin, NULL, search_pattern);
		return 0;
	}

	/* process the list of files*/
	for (; optind < argc; optind++) {
		temp_str = argv[optind];
		if (strcmp(temp_str, STREAM_REDIRECT) == 0)
		/* "-" redirect the io to stdin*/
				{
			search_stream(stdin, NULL, search_pattern);
		} else {
			if ((fptr = fopen(temp_str, "r")) == NULL) {
				perror(temp_str);
			} else {
				search_stream(fptr, temp_str, search_pattern);
				fclose(fptr);
			}
		}
	}

	/* clean the allocated memory*/
	if (search_pattern != NULL)
		free(search_pattern);
	if (any_line_buffer != NULL)
		free(any_line_buffer);

	return EXIT_SUCCESS;

} /* main */

