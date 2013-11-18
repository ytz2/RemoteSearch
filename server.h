/*
 * Yanhua Liu (ytz2) CS820
 *  server.h - header for server.c
 */

#ifndef _SERVER_H
#define _SERVER_H
#include "ospenv.h"
#include "command_util.h"
#include "send_recv.h"
#include "plcsIO.h"
#include "search_given.h"
#include "thread_share.h"
#include "print_time.h"
void
listener(char *server_port, char *interface_name);

/*once receive the search parameters from message 1
 * we should print out the agent fd, threadid and options
 */
void print_search_para(int fd,search *mysearch);
#endif
