/*
 * Yanhua Liu (ytz2) CS820
 * rplcsd.c	- a server main program running under unix
 *		  that uses the socket interface to tcp or udp
 *
 * there are two optional command line parameters:
 *
 *	1	the port number on which the server should listen
 *	2	the name of the interface on which the server should listen
 */


#include "ospenv.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "server.h"
#include "global.h"


pthread_key_t line_buffer_key;
pthread_key_t out_buffer_key;
pthread_key_t err_buffer_key;
#ifdef __sun__
pthread_once_t init_done= {PTHREAD_ONCE_INIT}; // once key
#else
pthread_once_t init_done = PTHREAD_ONCE_INIT; // once key
#endif

int
main(int argc, char *argv[])
{
	char *server_port;
	char *interface_name;
	argv++;				/* point at first parameter (if any) */
	argc--;

	/* get the server's port number from optional first parameter */
	if (argc == 0) {
		server_port = NULL;
	} else {
		server_port = *argv++;	/* point at second parameter (if any)*/
		argc--;
	}

	if (argc == 0) {
		interface_name = NULL;
	} else {
		interface_name = *argv++;/* point at third parameter (if any)*/
		argc--;
	}
	/* now let the common listener do the real work */
	listener(server_port, interface_name);

	exit(EXIT_SUCCESS);
}	/* main */

/* vi: set autoindent tabstop=8 shiftwidth=8 : */
