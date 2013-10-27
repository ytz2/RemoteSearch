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
 *  HW2
 *  The any_line_buffer is moved to plcsIO.h to make it thread safe
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
#include "dirHandle.h"
/*
 * The following parameters are made global
 * they are set via switches and interfaces
 * with plcsIO and is externed by global.h
 * any usage of these global prameters
 * must include "global.h"
 */

unsigned int options_flags; /* work with switch options_flags*/
int shift_table[MAX_ASCII]; /* shift table*/
int line_buffer_size; /* work with -l number*/
int max_line_number; /* work with -m number*/
int column_number; /* work with -n number*/
char *search_pattern; /* the search pattern*/
int thread_limits; /* work with -t number*/
int max_dir_depth; /* work with -d number */
/*
 * use this to store the generated history stacks in level 0
 * the reason to keep that is I have to keep the history stacks
 * in heap for unfinished threads, when the last pthread_exit is
 * called, use atexit to clean them
 */
stack* stacks[MAX_STACKS];

/*very simple operations of stack*/
int stacks_top=-1;
void push_stacks(stack *val);
void clear_stacks();
// function to clean up the string bufferand will be called by atexit()
void exit_func1();
// function to clear up the stacks
void exit_func2();

/*main function*/
int main(int argc, char *argv[]) {

	int c; /* switch character */
	int state, monitor_state; /* Error state during switch processing*/
	char *temp_str; /* temporary string*/
	struct stat info; /* to test if a given name a file,directory of link */
	stack *temp_stk; /*a temp var to work with history stack */
	Node *temp_node; /* a temp var to work with hsitory stack node */
	/* initialize all the options to their default values */
	options_flags = CASE_SENSITIVE; /* default is that inver printing OFF */
	line_buffer_size = DEFAULT_LINE_BUFFER;
	max_line_number = -1;
	column_number = -1;
	thread_limits=-1;
	max_dir_depth=-1;
	state = 1;
	search_pattern = NULL;
	monitor_state = 0;
	/* process all the command line switches */
	opterr = 0; /* prevent getopt() from printing error messages */

	while ((c = getopt(argc, argv, OPTIONS)) != -1) {
		switch (c) {
		case 'h': /* print help info */
			printf("Usage: plcs [options] search_string [list of input file/directory names]\n");
			printf("Search search_string in Provided File,Directory or Standard input\n");
			printf("Example: ./plcs hello main.c\n");
			printf("-h: this is help information\n");
			printf("-b: select lines only matched at begin (default OFF)\n");
			printf("-e: select lines only matched at end (default OFF)\n");
			printf("-i: turn case sensitive OFF (default ON)\n");
			printf("-v: turn inverse search ON (default OFF)\n");
			printf("-l512: set the size of buffer to 512\n"
					"\t(default %d and must less than %d)\n",
					DEFAULT_LINE_BUFFER, MAX_LINE_BUFFER);
			printf("-p: include the real path of file\n");
			printf("-m50: set the maximum number of lines to\n "
					"\tbe processes to 50 \n");
			printf("-n8: set the column of format number to 8 (max is 15)\n");
			printf("-a: process the files or directories in sub directories starts with '.'\n");
			printf("-f: do not follow the symbolic link\n");
			printf("-q: silent the error report when processing sub directories\n");
			printf("-d5: the recursive descent depth into directories is limited to 5\n");
			printf("-t3: to limit to 3 threads when processing sub directories\n");
			return 0;
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
		case 'f': /*invert the selection*/
			options_flags |= NOT_FOLLOW_LINK;
			break;
		case 'a': /*allow .dir .file search*/
			options_flags |= DOT_ACCESS;
			break;
		case 'v': /*invert the selection*/
			options_flags |= INVERSE_PRINT;
			break;
		case 'q': /*silent error message in subdir*/
			options_flags |= NO_ERR_MSG;
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
		case 't': /* set the line depth */
			monitor_state = scan_switch_number(c, &thread_limits);
			if (thread_limits<0) {
				fprintf(stderr, "Illegal numeric value \"%d\" for switch -d\n",
						thread_limits);
				thread_limits = -1;
				state = -1;
				monitor_state = 0;
			} else if (monitor_state == -1) {
				state = -1;
				monitor_state = 0;
			}
			break;
		case 'd': /* set column number to print line number */
			monitor_state = scan_switch_number(c, &max_dir_depth);
			if (max_dir_depth<0) {
				fprintf(stderr, "Illegal numeric value \"%d\" for switch -n\n",
						max_dir_depth);
				max_dir_depth = -1;
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
	atexit(exit_func1);
	/*if both -b and -e are not switched on, build the shit table*/
	/*for BM algorithm to implement*/
	if (!(options_flags & AT_BEGIN) && !(options_flags & AT_BEGIN)
			&& strlen(search_pattern))
		build_shift_table(shift_table, search_pattern,
				!(options_flags & CASE_SENSITIVE));

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
			continue;
		}
		errno=0;
		/*get the file info */
		if (stat(temp_str, &info) == -1) {
			perror(temp_str);
			continue;
		}
		/* if it directory */
		if (S_ISDIR(info.st_mode))
		{
			/*
			 * if -d is set to 0, do not process directory
			 */
			if (max_dir_depth==0)
				fprintf(stderr,"%s is detected to exceed the search limit %d\n",
						temp_str, max_dir_depth);
			else
			{
				/*allocate space to a history stack*/
				if ((temp_stk=malloc(sizeof(stack)))==NULL)
				{
					perror("malloc():");
					continue;
				}
				else
				{
					/* initialize the stack and push it on stacks*/
					stack_init(temp_stk);
					push_stacks(temp_stk);
					/* make a node */
					if((temp_node=make_node(temp_str,1,temp_stk))==NULL)
					{
						fprintf(stderr,"Error in make_node\n");
						continue;
					}
					/*push the root node to the stack and kick it off*/
					stack_push(temp_stk,temp_node,NULL);
					walk_to_next(temp_node);
				}
			}
		}
		search_file(temp_str,search_pattern,0);
	}
	atexit(exit_func2);
	pthread_exit(NULL);
} /* main */

// function to clean up
void exit_func1()
{
	if (search_pattern != NULL)
		free(search_pattern);
}

// function to clear up the stacks
void exit_func2()
{
	clear_stacks();
}
/* push a stack* to the stack of stacks*/
void push_stacks(stack *val)
{
	stacks[++stacks_top] = val;
}
/* clean the stacks*/
void clear_stacks()
{
	int d;
	/* if no subdirs, no need to clean */
	if(stacks_top==-1)
		return;
	for(d=stacks_top;d>=0;d--)
	{
		stack_destroy(stacks[d]);
		free(stacks[d]);
	}
}
