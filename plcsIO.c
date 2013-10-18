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
 */

#include "plcsIO.h"

/*
 * search_str is a wrapper function wraps the options searches
 * defined in homework. -b -e -i and -v are handled
 */

int search_str(char* buffer, char* search_str) {

	int result, insensitive;
	result = 0;
	insensitive = 0;

	/* check if it is null */
	if (buffer == NULL || search_str == NULL ) {
		fprintf(stderr, "Null String in search_str function\n");
		exit(1);
	}
	/* check if -i is set, if on set insensitive as 1 else 0 */
	insensitive = !(options_flags & CASE_SENSITIVE);

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
		result = boyer_moore(buffer, search_str, shift_table, insensitive);

	return (options_flags & INVERSE_PRINT ? (!result) : result);
}/*search_str*/

/*
 * print_line
 * print the str to standard output as the specified cols
 * will write realpath: lineno: str or if realpath is NULL
 * write: lineno: str
 */
void print_line(int lineno, char* realpath, char* str) {
	const char *temp1, *temp2;
	if (realpath == NULL ) {
		temp1 = "";
		temp2 = "";
	} else {
		temp1 = realpath;
		temp2 = ": ";
	}
	if (column_number>0)
		printf("%s%s%*d: %s\n", temp1, temp2, column_number, lineno, str);
	else
		printf("%s%s%s\n", temp1, temp2, str);
}/*print_line*/



/*
 * search_stream
 * accept stream ptr either file or stdin
 * read each line specified by -p switch
 * or default value into any_line_buffer
 * search and print it
 */
void search_stream(FILE *fptr, char* filename, char* objstr) {
	int lineno;
	int output_lineno;
	char* rptr, *memptr;
	/*initialize the rptr*/
	rptr = NULL;
	memptr=NULL;
	output_lineno=1;
	/*
	 * check if it is stdin, if not and show_path is set
	 * get the realpath for printing purpose
	 */
	if ((fptr != stdin) && (options_flags & SHOW_PATH))
		rptr = get_realpath(filename,&memptr);

	/*process each line in the input*/
	errno=0;
	for (lineno = 1;!feof(fptr) && fgets(any_line_buffer, line_buffer_size + 1, fptr)!=NULL ; lineno++) {
		if (max_line_number>=0 && output_lineno>max_line_number)
			break;
		if (errno!=0 || ferror(fptr)) {
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
		if (search_str(any_line_buffer, objstr)){
			print_line(lineno, rptr, any_line_buffer);
			output_lineno++;
		}
	}
	/*free the real path memory*/
	if (memptr!=NULL)
		free(memptr);
	/*reset the stream for further redirection*/
	rewind(fptr);
}/*search_stream*/
