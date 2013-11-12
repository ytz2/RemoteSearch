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
	fprintf(stderr,"0x%02x \t%s \n", (flags & this_one) ? this_one : 0,name);
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

/*initialize the search request*/
void init_search(search **my_search)
{
	/*allocate space to search parameters*/
	if ((*my_search=(search*)malloc(sizeof(search)))==NULL) {
		perror("malloc:");
		exit(EXIT_FAILURE);
	}
	(*my_search)->options_flags = 0; /* default flag */
	(*my_search)->line_buffer_size =MAX_LINE_BUFFER;
	(*my_search)->max_line_number = -1;
	(*my_search)->column_number = -1;
	(*my_search)->thread_limits = -1;
	(*my_search)->max_dir_depth = -1;
	(*my_search)->search_pattern = NULL;
	(*my_search)->shift_table=NULL;
	(*my_search)->client_fd=-1;
}
/*destroy the search*/
void destroy_search(search *mysearch)
{
	if((mysearch->search_pattern)!=NULL)
		free(mysearch->search_pattern);
	if (mysearch->shift_table)
		free(mysearch->shift_table);
	if ((mysearch->client_fd)>0)
	{
		close(mysearch->client_fd);
	}
	free(mysearch);
}

/* wrapper of getopt function */
void scan_opt_search(int argc,char *argv[],search *mysearch)
{
	int c; /* switch character */
	int state, monitor_state; /* Error state during switch processing*/
	state = 1;
	monitor_state = 0;
	/* process all the command line switches */
	opterr = 0; /* prevent getopt() from printing error messages */

	while ((c = getopt(argc, argv, OPTIONS)) != -1) {
		switch (c) {
		case 'h': /* print help info */
			printf(
					"Usage: plcs [options] search_string [list of input file/directory/remote names]\n");
			printf(
					"Search search_string in Provided File,Directory or Standard input\n");
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
			printf(
					"-a: process the files or directories in sub directories starts with '.'\n");
			printf("-f: do not follow the symbolic link\n");
			printf(
					"-q: silent the error report when processing sub directories\n");
			printf(
					"-d5: the recursive descent depth into directories is limited to 5\n");
			printf(
					"-t3: to limit to 3 threads when processing sub directories\n");
			exit(0);
		case 'b': /* turn match at begin on */
			mysearch->options_flags |= AT_BEGIN;
			break;
		case 'e': /* turn match at end on */
			mysearch->options_flags |= AT_END;
			break;
		case 'i': /* turn case sensitive off */
			mysearch->options_flags |= CASE_INSENSITIVE;
			break;
		case 'p': /* include the real path*/
			mysearch->options_flags |= SHOW_PATH;
			break;
		case 'f': /*invert the selection*/
			mysearch->options_flags |= NOT_FOLLOW_LINK;
			break;
		case 'a': /*allow .dir .file search*/
			mysearch->options_flags |= DOT_ACCESS;
			break;
		case 'v': /*invert the selection*/
			mysearch->options_flags |= INVERSE_PRINT;
			break;
		case 'q': /*silent error message in subdir*/
			mysearch->options_flags |= NO_ERR_MSG;
			break;
		case 'l': /* set buffer size */
			monitor_state = scan_switch_number(c, &mysearch->line_buffer_size);
			if (mysearch->line_buffer_size <= 0 || mysearch->line_buffer_size >= MAX_LINE_BUFFER) {
				fprintf(stderr, "Illegal numeric value \"%d\" for switch -l\n",
						mysearch->line_buffer_size);
				mysearch->line_buffer_size = DEFAULT_LINE_BUFFER;
				state = -1;
			} else if (monitor_state == -1) {
				state = -1;
				monitor_state = 0;
			}
			break;
		case 'm': /* set maximum lines to be processed */
			monitor_state = scan_switch_number(c, &mysearch->max_line_number);
			if (mysearch->max_line_number <= 0) {
				fprintf(stderr, "Illegal numeric value \"%d\" for switch -m\n",
						mysearch->max_line_number);
				mysearch->max_line_number = -1;
				state = -1;
				monitor_state = 0;
			} else if (monitor_state == -1) {
				state = -1;
				monitor_state = 0;
			}
			break;
		case 'n': /* set column number to print line number */
			monitor_state = scan_switch_number(c, &mysearch->column_number);
			if (mysearch->column_number >= MAX_COLS || mysearch->column_number <= 0) {
				fprintf(stderr, "Illegal numeric value \"%d\" for switch -n\n",
						mysearch->column_number);
				mysearch->column_number = -1;
				state = -1;
				monitor_state = 0;
			} else if (monitor_state == -1) {
				state = -1;
				monitor_state = 0;
			}
			break;
		case 't': /* set the line depth */
			monitor_state = scan_switch_number(c, &mysearch->thread_limits);
			if (mysearch->thread_limits < 0) {
				fprintf(stderr, "Illegal numeric value \"%d\" for switch -d\n",
						mysearch->thread_limits);
				mysearch->thread_limits = -1;
				state = -1;
				monitor_state = 0;
			} else if (monitor_state == -1) {
				state = -1;
				monitor_state = 0;
			}
			break;
		case 'd': /* set column number to print line number */
			monitor_state = scan_switch_number(c, &mysearch->max_dir_depth);
			if (mysearch->max_dir_depth < 0) {
				fprintf(stderr, "Illegal numeric value \"%d\" for switch -n\n",
						mysearch->max_dir_depth);
				mysearch->max_dir_depth = -1;
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
		exit(EXIT_FAILURE);

	/* now process the search_string given in command*/
	if (optind < argc) {
		if ((mysearch->search_pattern = (char*) malloc(strlen(argv[optind]) + 1)) == NULL) {
			perror("Allocate space to search_pattern\n");
			exit(EXIT_FAILURE);
		}
		strcpy(mysearch->search_pattern, argv[optind]);
		++optind;
	} else {
		/*if there is no search_string, issue error and exit*/
		fprintf(stderr, "search_string is not provided as an argument\n");
		exit(1);
	}
}



/* check if the list of file contain the : to indicate a remote search */
remote* scan_remote_search(char *input)
{
	int len,i,colon,slash;
	remote *rmt;
	len=strlen(input);
	colon=0;
	slash=0;
	/*record the : and / position */
	for(i=0;i<len;i++)
	{
		if(input[i]==':')
		{
			colon=i;
		}
		if (input[i]=='/')
		{
			slash=i;
		}
	}
	if (colon==0)
		return NULL;
	/* if no : or / or the / is the last char */
	if ( slash== 0 || slash==len-1 ||
			colon>HOSTMAX || (slash-colon-1)>PORT_MAX
			|| (len-slash)>REMOTE_NAME_MAX)
	{
		fprintf(stderr,"%s has the wrong format\n",input);
		return NULL;
	}
	if ((rmt=(remote*)malloc(sizeof(remote)))==NULL)
	{
		perror("malloc");
		return NULL;
	}
	for (i=0;i<colon;i++)
		rmt->node[i]=input[i];
	rmt->node[i]='\0';
	for (i=colon+1;i<slash;i++)
		rmt->port[i-colon-1]=input[i];
	rmt->port[i]='\0';
	for (i=slash+1;i<len;i++)
		rmt->name[i-slash-1]=input[i];
	rmt->name[i]='\0';

	return rmt;
}
