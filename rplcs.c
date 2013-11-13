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
#include "plcsIO.h"
#include "search_given.h"
#include "global.h"

pthread_key_t line_buffer_key;
pthread_key_t out_buffer_key;
pthread_key_t err_buffer_key;

#ifdef __sun__
pthread_once_t init_done= {PTHREAD_ONCE_INIT}; // once key
#else
pthread_once_t init_done = PTHREAD_ONCE_INIT; // once key
#endif

/*
 * The following parameters are made global
 * they are set via switches and interfaces
 * with plcsIO and is externed by global.h
 * any usage of these global prameters
 * must include "global.h"
 */

/*main function*/
int main(int argc, char *argv[]) {

	search *mysearch; /*ptr to a structure holding the search request */
	remote *rmt;
	Client_para *para;
	pthread_t id;
	int err;

	/*initialization*/
	err=0;
	mysearch=NULL;
	init_search(&mysearch);

	/* process all the command line switches */
	opterr = 0; /* prevent getopt() from printing error messages */
	scan_opt_search(argc,argv,mysearch); /*move the getopt to command_util.h to shorten the main */
	/*build the shift table*/
	build_shifttable(mysearch);
	/*
	 * if there is no argument in list of files
	 * directly go to stdin
	 */
	if (optind >= argc) {
		search_stream(stdin, NULL, mysearch);
		return 0;
	}

	/* process the list of files*/
	for (; optind < argc; optind++) {
		/* if it a remote search */
		if ((rmt=scan_remote_search(argv[optind]))!=NULL)
		{
			fprintf(stderr,"%s %s %s\n",rmt->node,rmt->port,rmt->name);
			if ((para=(Client_para*)malloc(sizeof(Client_para)))==NULL)
			{
				perror("malloc");
				continue;
			}
			para->mysearch=mysearch;
			para->rmt=rmt;
			/* spawn a thread the perform remote search*/
			err = pthread_create(&id, NULL, client_agent, (void*)para);
			if (err!=0)
			{
				fprintf(stderr,"Pthread_create of client\n");
				free(para);
				continue;
			}
			continue;
		}
		if (strcmp(argv[optind], STREAM_REDIRECT) == 0)
			/* "-" redirect the io to stdin*/
		{
			search_stream(stdin, NULL, mysearch);
			continue;
		}
		search_given(argv[optind],mysearch);
	}
	pthread_exit(NULL);

	/* it has no real meaning, just to depreciate the compiler warning*/
	return 0;
} /* main */

