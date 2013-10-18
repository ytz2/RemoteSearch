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
 * This is function in Steven's book page 50
 * it returns the allocated buffer and the allocated
 * buffer size in bytes
 */
char* path_alloc(size_t *sizep)
{

	/*
	 * firstly do some compile time check
	 */
#ifdef PATH_MAX
	long pathmax=PATH_MAX;
#else
	long pathmax=0;
#endif
	char *ptr;
	size_t size;
	/*
	 * if not defined in compile time, running time check
	 */
	if (pathmax==0)
	{
		errno=0;
		if((pathmax=pathconf("/",_PC_PATH_MAX))<0)
		{
			if (errno==0)
				pathmax= PATH_MAX_GUESS;
			else
				fprintf(stderr, "PATH_CONF: %s\n", strerror(errno));
		}
		else
			pathmax++; // the pathmax is got relative to root,+1
	}
	size=pathmax;
	errno=0;
	if ((ptr=malloc(size))==NULL)
		fprintf(stderr, "malloc() for path buffer: %s\n", strerror(errno));
	if (sizep!=NULL)
		*sizep=size;
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
char* get_realpath(char* filename,char **memptr) {

	char *realptr;
	realptr = NULL;

	if ((*memptr=path_alloc(NULL))==NULL )
		fprintf(stderr, "malloc(): %s %s", strerror(errno), filename);
	else {
		realptr = realpath(filename, *memptr);
		if (realptr == NULL ) {
			fprintf(stderr, "%s %s", filename, strerror(errno));
		}
	}
	return realptr;
}/*get_realpath*/
