/*
 * Yanhua Liu (ytz2) CS820
 * command_util.c
 *
 *
 *  Created by Yanhua Liu for CS820
 *  Created on: Sep 5, 2013
 *  This header file provide the utilities to
 *  implement the getopt interface
 */

#include "command_util.h"

/* Scans the string pointed to by optarg and tries to convert it to an int number.
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

/*print the command options */
void print_flag(unsigned int flags, unsigned int this_one, char *name) {
	fprintf(stderr, "%s\t0x%02x\n", name, (flags & this_one) ? this_one : 0);
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
void init_search(search **my_search) {
	/*allocate space to search parameters*/
	if ((*my_search = (search*) malloc(sizeof(search))) == NULL) {
		perror("malloc:");
		exit(EXIT_FAILURE);
	}
	(*my_search)->options_flags = 0; /* default flag */
	(*my_search)->line_buffer_size = DEFAULT_LINE_BUFFER;
	(*my_search)->max_line_number = -1;
	(*my_search)->column_number = -1;
	(*my_search)->thread_limits = -1;
	(*my_search)->max_dir_depth = -1;
	(*my_search)->search_pattern = NULL;
	(*my_search)->shift_table = NULL;
	(*my_search)->client_fd = -1;
	//(*my_search)->thread_done=-1;
	(*my_search)->stk_count = 0;
	(*my_search)->alive_threads = 0;
	/* set the statistical table value to 0 */
	memset(&(*my_search)->statistics, 0, sizeof(Statistics));
	pthread_mutex_init(&((*my_search)->lock), NULL);
	pthread_cond_init(&((*my_search)->ready), NULL);
}
/*destroy the search*/
void destroy_search(search *mysearch) {
	/*free the search string*/
	if ((mysearch->search_pattern) != NULL)
		free(mysearch->search_pattern);
	/*free the search table */
	if (mysearch->shift_table)
		free(mysearch->shift_table);
	/* close fd if it is on the server side*/
	if ((mysearch->client_fd) > 0) {
		pthread_mutex_destroy(&(mysearch->lock));
		pthread_cond_destroy(&(mysearch->ready));
		close(mysearch->client_fd);
	}
	/* finally free the search object*/
	free(mysearch);
}

/* wrapper of getopt function */
void scan_opt_search(int argc, char *argv[], search *mysearch) {
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
					"Usage: rplcs [options] search_string [list of input file/directory/remote names]\n");
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
			if (mysearch->line_buffer_size
					<= 0|| mysearch->line_buffer_size >= MAX_LINE_BUFFER) {fprintf(stderr, "Illegal numeric value \"%d\" for switch -l\n",
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
		if ((mysearch->search_pattern = (char*) malloc(strlen(argv[optind]) + 1))
				== NULL) {
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
remote* scan_remote_search(char *buffer, int *flag) {
	char input[HOSTMAX + PORT_MAX + REMOTE_NAME_MAX + 1];
	int len, i, colon, slash;
	remote *rmt;
	colon = -1;
	slash = -1;
	strcpy(input, buffer);
	len = strlen(input);
	trim_line(input);
	/*record the : and / position */
	for (i = 0; i < len; i++) {
		/* record the first : */
		if (input[i] == ':') {
			if (colon == -1)
				colon = i;
		}
		/* record the first / */
		if (input[i] == '/') {
			if (slash == -1)
				slash = i;
		}
	}
	/* if there is a : then interprete it
	 * as a remote name, if no colon, or
	 * remote_name has not characters, wrong
	 */
	/* not a remote name */
	if (colon == -1) {
		*flag = 0;
		return NULL;
	} else
		*flag = 1; // yes, it is a remote name

	if (colon == 0) {
		fprintf(stderr, "%s has the wrong remote_name format\n", input);
		return NULL;
	}
	/* if there is no / or the / is the last char
	 * or there is no content between : and /
	 * or : and / position exceed our predefined size
	 *  */
	if (slash == -1|| slash==len-1 ||slash-colon==1||
	colon>HOSTMAX || (slash-colon-1)>PORT_MAX
	|| (len-slash)>REMOTE_NAME_MAX) {fprintf(stderr,"%s has the wrong remote_name format\n",input);
	return NULL;
}
	if ((rmt = (remote*) malloc(sizeof(remote))) == NULL) {
		perror("malloc");
		return NULL;
	}
	for (i = 0; i < colon; i++)
		rmt->node[i] = input[i];
	rmt->node[i] = '\0';
	for (i = colon + 1; i < slash; i++)
		rmt->port[i - colon - 1] = input[i];
	rmt->port[i] = '\0';
	for (i = slash + 1; i < len; i++)
		rmt->name[i - slash - 1] = input[i];
	rmt->name[i] = '\0';
	return rmt;
}

/* to work with print_stat, check void print_stat*/
void add_one_line(char* buff, char *info, unsigned int val) {
	char line_buff[RIGHT_JUST * 2];
	const char *fmt = "%*s   %u\n";
	sprintf(line_buff, fmt, RIGHT_JUST, info, val);
	strcat(buff, line_buff);
	;
}

/*print the statistics
 * Put every thing in a big buffer
 * to let the printing of statistics
 * together
 * */
void print_stat(FILE* fptr, Statistics *statistics, double tdiff) {
	char buffer[RIGHT_JUST * 2 * 17];
	char *info;
	char line_buff[RIGHT_JUST * 2];
	double rate;
	memset(buffer, 0, RIGHT_JUST*2*17);
	rate = 0.;
	fflush(stdout);
	sprintf(line_buff, "%*s   %s\n", RIGHT_JUST, "label", "statistics");
	strcat(buffer, line_buff);

	info = "Total soft links ignored due to -f";
	add_one_line(buffer, info, statistics->link_ignored);

	info = "Total directories opened successfully";
	add_one_line(buffer, info, statistics->dir_opened);

	info = "Total directory loops avoided";
	add_one_line(buffer, info, statistics->loop_avoided);

	info = "Total directory descents pruned by -d";
	add_one_line(buffer, info, statistics->dir_pruned);

	info = "Maximum directory descent depth";
	add_one_line(buffer, info, statistics->max_depth);

	info = "Total dot names not ignored due to -a";
	add_one_line(buffer, info, statistics->dot_caught);

	info = "Total descent threads created";
	add_one_line(buffer, info, statistics->thread_created);

	info = "Total descent threads pruned by -t";
	add_one_line(buffer, info, statistics->thread_not_created);

	info = "Maximum simultaneously active descent threads";
	add_one_line(buffer, info, statistics->max_alive);

	info = "Total errors not printed due to -q";
	add_one_line(buffer, info, statistics->err_quiet);

	info = "Total lines matched";
	add_one_line(buffer, info, statistics->lines_matched);

	info = "Total lines read";
	add_one_line(buffer, info, statistics->lines_read);

	info = "Total files read";
	add_one_line(buffer, info, statistics->file_read);

	info = "Total bytes read";
	add_one_line(buffer, info, statistics->bytes_read);

	info = "Total search time in seconds";
	sprintf(line_buff, "%*s   %.6f\n", RIGHT_JUST, info, tdiff);
	strcat(buffer, line_buff);
	if (tdiff != 0.)
		rate = ((double) statistics->bytes_read) / (1000000. * tdiff);
	info = "Processing rate in megabytes per second";
	sprintf(line_buff, "%*s   %.6f\n", RIGHT_JUST, info, rate);
	strcat(buffer, line_buff);
	fprintf(fptr, "%s", buffer);
	fflush(fptr);
}

/*
 * update the statistics from node to root
 * the fields about thread will not be updated
 *
 */

void update_statistics(Statistics *root, Statistics *node) {
	root->link_ignored += node->link_ignored;
	root->dir_opened += node->dir_opened;
	root->loop_avoided += node->loop_avoided;
	root->dir_pruned += node->dir_pruned;
	root->max_depth = MAX(root->max_depth,node->max_depth);
	root->dot_caught += node->dot_caught;
	/* the thread related have been directly updated to mysearch obj */
	root->err_quiet += node->err_quiet;
	root->lines_matched += node->lines_matched;
	root->lines_read += node->lines_read;
	root->file_read += node->file_read;
	root->bytes_read += node->bytes_read;
}

/*
 * update the root from node
 * all fields in statistics structure get updated
 */
void update_statistics_sock(Statistics *root, Statistics *node) {
	update_statistics(root, node);
	root->thread_created += node->thread_created;
	root->thread_not_created += node->thread_not_created;
	root->max_alive = MAX(root->max_alive,node->max_alive);
}

/*
 * translate the statistics structure to net endian
 */
void trans_stat2send(Statistics *st) {
	st->link_ignored = htonl(st->link_ignored);
	st->dir_opened = htonl(st->dir_opened);
	st->loop_avoided = htonl(st->loop_avoided);
	st->dir_pruned = htonl(st->dir_pruned);
	st->max_depth = htonl(st->max_depth);
	st->dot_caught = htonl(st->dot_caught);
	st->err_quiet = htonl(st->err_quiet);
	st->lines_matched = htonl(st->lines_matched);
	st->lines_read = htonl(st->lines_read);
	st->file_read = htonl(st->file_read);
	st->bytes_read = htonl(st->bytes_read);
	st->thread_created = htonl(st->thread_created);
	st->thread_not_created = htonl(st->thread_not_created);
	st->max_alive = htonl(st->max_alive);
}

/*
 * translate the statistics structure from net endian
 * to local endian
 */
void trans_stat2recv(Statistics *st) {
	st->link_ignored = ntohl(st->link_ignored);
	st->dir_opened = ntohl(st->dir_opened);
	st->loop_avoided = ntohl(st->loop_avoided);
	st->dir_pruned = ntohl(st->dir_pruned);
	st->max_depth = ntohl(st->max_depth);
	st->dot_caught = ntohl(st->dot_caught);
	st->err_quiet = ntohl(st->err_quiet);
	st->lines_matched = ntohl(st->lines_matched);
	st->lines_read = ntohl(st->lines_read);
	st->file_read = ntohl(st->file_read);
	st->bytes_read = ntohl(st->bytes_read);
	st->thread_created = ntohl(st->thread_created);
	st->thread_not_created = ntohl(st->thread_not_created);
	st->max_alive = ntohl(st->max_alive);
}
