/*
 * plcsIO.c
 *
 *  Created by Yanhua Liu for CS820 assignment 1
 *  Created on: Sep 6, 2013
 *  This Header files defines function utilities
 *  to implement IO reading & printing functions
 *  used in plcs
 *
 *  History:
 *  1 Corrected the maximum line no bug definition
 *  2 Do not print line number if -n is not set
 *  3 Add a space to the realpath
 *
 *  HW2:
 *  1 make the any_line_buffer to be thread local storage
 *
 */

#include "plcsIO.h"
#include "thread_share.h"
/*
 * search_str is a wrapper function wraps the options searches
 * defined in homework. -b -e -i and -v are handled
 */


int search_str(char* buffer, search *mysearch) {
	int result, insensitive;
	unsigned int options_flags;
	char *search_str;
	options_flags=mysearch->options_flags;
	search_str=mysearch->search_pattern;
	result = 0;
	insensitive = 0;

	/* check if it is null */
	if (buffer == NULL || search_str == NULL) {
		fprintf(stderr, "Null String in search_str function\n");
		exit(1);
	}
	/* check if -i is set, if on set insensitive as 1 else 0 */
	insensitive = (options_flags & CASE_INSENSITIVE);

	if ((options_flags & AT_BEGIN) && !(options_flags & AT_END))
		result = search_begin(buffer, search_str, insensitive);
	/* check if -b set and -e set */
	else if (!(options_flags & AT_BEGIN) && (options_flags & AT_END))
		result = search_end(buffer, search_str, insensitive);
	/* both -b and -e are set */
	else if ((options_flags & AT_BEGIN) && (options_flags & AT_END))
		result = exact_match(buffer, search_str, insensitive);
	/* else return the normal search */
	else
		result = boyer_moore(buffer, search_str, mysearch->shift_table, insensitive);

	return (options_flags & INVERSE_PRINT ? (!result) : result);
}/*search_str*/


/*
 * send the stderr msg to client side
 */
void send_err_line(search *mysearch,char *format,...)
{
	char *err_buffer;
	unsigned int len;
	pthread_once(&init_done, thread_init);
	err_buffer = pthread_getspecific(err_buffer_key);
	/* allocate memory to buffer*/
	if (err_buffer == NULL) {
		err_buffer = (char*) malloc(MAX_TCP_ERR);
		pthread_setspecific(err_buffer_key, err_buffer);
	}
	va_list argptr;
	va_start(argptr, format);
	vsprintf(err_buffer, format, argptr);
	va_end(argptr);
	len=strlen(err_buffer);
	err_buffer[len]='\n';
	err_buffer[++len]='\0';
	pthread_mutex_lock(&(mysearch->lock));
		/* send the fourth message */
	if (our_send_message(mysearch->client_fd, OUTPUT_ERR,len+1,err_buffer) != 0)
	{
		fprintf(stderr,"Fail to send %s\n",err_buffer);
		pthread_mutex_unlock(&(mysearch->lock));
		return;
	}
	pthread_mutex_unlock(&(mysearch->lock));
}

/* send the std_output from the server side to the client side */
void send_print_line(int lineno, char* realpath, char* str,int column_number,search *mysearch) {

	const char *temp1, *temp2;
	char *out_buffer;
	unsigned int len;
	out_buffer=NULL;
	pthread_once(&init_done, thread_init);
	out_buffer = pthread_getspecific(out_buffer_key);
		/* allocate memory to buffer*/
	if (out_buffer == NULL) {
		out_buffer = (char*) malloc(MAX_TCP_STD);
		pthread_setspecific(out_buffer_key, out_buffer);
	}
	if (realpath == NULL) {
		temp1 = "";
		temp2 = "";
	} else {
		temp1 = realpath;
		temp2 = ": ";
	}
	if (column_number > 0)
		sprintf(out_buffer,"%s%s%*d: %s", temp1, temp2, column_number, lineno, str);
	else
		sprintf(out_buffer,"%s%s%s",temp1, temp2, str);

	len=strlen(out_buffer);
	out_buffer[len]='\n';
	out_buffer[++len]='\0';
	/* the interface provided by tcpio is not atomic, add a lock */
	pthread_mutex_lock(&(mysearch->lock));
	/* send the fourth message */
	if (our_send_message(mysearch->client_fd, OUTPUT_STD,len+1,out_buffer) != 0)
	{
		fprintf(stderr,"Fail to send %s\n",out_buffer);
		pthread_mutex_unlock(&(mysearch->lock));
		return;
	}
	pthread_mutex_unlock(&(mysearch->lock));

}/*print_line*/
/*
 * print_line
 * print the str to standard output as the specified cols
 * will write realpath: lineno: str or if realpath is NULL
 * write: lineno: str
 */
void print_line(int lineno, char* realpath, char* str,int column_number) {
	const char *temp1, *temp2;
	if (realpath == NULL) {
		temp1 = "";
		temp2 = "";
	} else {
		temp1 = realpath;
		temp2 = ": ";
	}
	if (column_number > 0)
		printf("%s%s%*d: %s\n", temp1, temp2, column_number, lineno, str);
	else
		printf("%s%s%s\n", temp1, temp2, str);
	fflush(stdout);
}/*print_line*/

/*
 * search_stream
 * accept stream ptr either file or stdin
 * read each line specified by -p switch
 * or default value into any_line_buffer
 * search and print it
 */
void search_stream(FILE *fptr, char* filename, search *mysearch) {
	int lineno;
	int output_lineno;
	char* rptr, *memptr, *any_line_buffer;
	/*initialize the rptr*/
	rptr = NULL;
	memptr = NULL;
	output_lineno = 1;
	pthread_once(&init_done, thread_init);
	any_line_buffer = pthread_getspecific(line_buffer_key);
	/* allocate memory to buffer*/
	if (any_line_buffer == NULL) {
		any_line_buffer = (char*) malloc(mysearch->line_buffer_size + 1);
		pthread_setspecific(line_buffer_key, any_line_buffer);
	}

	/*
	 * check if it is stdin, if not and show_path is set
	 * get the realpath for printing purpose
	 */
	if ((fptr != stdin) && (mysearch->options_flags & SHOW_PATH))
		rptr = get_realpath(filename, &memptr);

	/*process each line in the input*/errno = 0;
	for (lineno = 1;
			!feof(fptr)
					&& fgets(any_line_buffer, mysearch->line_buffer_size + 1, fptr)
							!= NULL; lineno++) {
		if (mysearch->max_line_number >= 0 && output_lineno > mysearch->max_line_number)
			break;
		if (errno != 0 || ferror(fptr)) {
			/*error check*/
			if (fptr == stdin)
				perror("stdin");
			else
				perror(filename);
			clearerr(fptr);
			break;
		}
		trim_line(any_line_buffer);
		/*if get the match, print the line*/
		if (search_str(any_line_buffer, mysearch)) {
			/*if it is on the server side*/
			if (mysearch->client_fd>0)
			{
				send_print_line(lineno, rptr, any_line_buffer,mysearch->column_number,mysearch);
			}
			else /* on the client side */
				print_line(lineno, rptr, any_line_buffer,mysearch->column_number);
			output_lineno++;
		}
	}
	/*free the real path memory*/
	if (memptr != NULL)
		free(memptr);
	/*reset the stream for further redirection*/
	rewind(fptr);
}/*search_stream*/

/*
 * search_file
 * accept a filename and a search string to perform a search
 * but add another flag to indicate it is in sub directory
 * tow work well with -q, the flag is not passed to search_stream
 * since when dealing with subdirectories, the stat will follow all
 * the way down and if any problem occurs, stat can issue error
 */
void search_file(char* filename, search *mysearch, int flag) {
	FILE *fptr;
	if ((fptr = fopen(filename, "r")) == NULL) {
		if (flag == 0 || (flag == 1 && !(mysearch->options_flags & NO_ERR_MSG)))
			{
				perror(filename);
				if (mysearch->client_fd>0)
					send_err_line(mysearch,"%s: %s",filename,strerror(errno));
			}
	} else {
		search_stream(fptr, filename, mysearch);
		fclose(fptr);
	}

}

/*
 * wrapper function build the shift talbe
 */
void build_shifttable(search *mysearch)
{
	/*if both -b and -e are not switched on, build the shit table*/
	/*for BM algorithm to implement*/
	if ((mysearch->shift_table=(int*)malloc(MAX_ASCII*sizeof(int)))==NULL)
	{
		perror("malloc: ");
		exit(1);
	}
	if (!(mysearch->options_flags & AT_BEGIN) && !(mysearch->options_flags & AT_END)
			&& strlen(mysearch->search_pattern))
		build_shift_table(mysearch->shift_table, mysearch->search_pattern,
				(mysearch->options_flags & CASE_INSENSITIVE));

}
