/* send_recv.h */

#ifndef _SEND_RECV_H
#define _SEND_RECV_H

#include "ospenv.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/tcp.h>			/* for TCP_NODELAY */
#include <pthread.h>				/* for threads */

#include "tcpblockio.h"
#include "no_sigpipe.h"
enum header_types {OPTION_PARAMETER=1, TO_SEARCH, REMOTE_NAME,OUTPUT_STD,OUTPUT_ERR,STATISTICS};

/* fixed-size message header, all fields in network byte order */
struct our_header {
	unsigned int	type;		/* type of this message */
	unsigned int	length;		/* number of bytes following header */
};


/* writes a fixed-size header followed by variable length data
 *
 * fills in header with values from type, length and info
 * writes length bytes of variable length data from user_data
 *
 * returns 0 if all ok, else -1 on any error
 */
extern int
our_send_message(int fd, enum header_types type, unsigned int length,
		  void *user_data);


/* reads a fixed-size header followed by variable length data
 *
 * fills in type, length and info with values from the header
 * reads up to *length bytes of variable length data into user_data
 *
 * returns number of bytes of user_data read, or -1 on error
 */
extern int
our_recv_message(int fd, enum header_types *type, unsigned int *length,
		 void *user_data);

#endif
