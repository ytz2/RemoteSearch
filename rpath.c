/*
 * * Yanhua Liu
 * For CS820 Assignment 2
 * rpath.c
 *
 *  Created on: Oct 18, 2013
 *      Author: yhliu
 */

#include "rpath.h"

/*
 * This is edited version of the function in Steven's book page 50
 * it returns the allocated buffer and the allocated
 * buffer size in bytes
 */
char* path_alloc(size_t *sizep) {

	/*
	 * firstly do some compile time check
	 */
#ifdef PATH_MAX
	long pathmax = PATH_MAX;
#else
	long pathmax=0;
#endif
	char *ptr;
	size_t size;
	/*
	 * if not defined in compile time, running time check
	 */
	if (pathmax == 0) {
		errno = 0;
		if ((pathmax = pathconf("/", _PC_PATH_MAX)) < 0) {
			if (errno == 0)
				pathmax = PATH_MAX_GUESS;
			else
				fprintf(stderr, "PATH_CONF: %s\n", strerror(errno));
		} else
			pathmax++; // the pathmax is got relative to root,+1
	}
	size = pathmax;
	errno = 0;
	if ((ptr = malloc(size)) == NULL)
		fprintf(stderr, "malloc() for path buffer: %s\n", strerror(errno));
	if (sizep != NULL)
		*sizep = size;
	// I leave the recovery to be handled in realpath function
	return ptr;
}

/*
 * get_realpath
 * return the realpath of a file
 * comparing with the example given in course
 * the memptr is provided outside
 * the memptr must be checked outside to free!!!
 */
char* get_realpath(char* filename, char **memptr, Node *current) {

	char *realptr;
	search * mysearch;
	mysearch=NULL;
	realptr = NULL;
	if (current)
		mysearch=current->stk->mysearch;

	if ((*memptr = path_alloc(NULL)) == NULL)
	{
		if (current==NULL || (current!=NULL && !(mysearch->options_flags & NO_ERR_MSG)))
			{
				fprintf(stderr, "malloc(): %s %s\n", strerror(errno), filename);
				send_err_line(mysearch,"malloc(): %s %s\n", strerror(errno), filename);
			}
		else if (current && (mysearch->options_flags & NO_ERR_MSG))
		{
			(current->statistics).err_quiet++;
		}
	}
	else {
		realptr = realpath(filename, *memptr);
		if (realptr == NULL) {
			fprintf(stderr, "%s: %s\n", filename, strerror(errno));
		}
	}
	return realptr;
}/*get_realpath*/
