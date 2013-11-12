/* client.h - header for client.c */

#ifndef _CLIENT_H
#define _CLIENT_H
#include "ospenv.h"
#include "command_util.h"
#include "tcpblockio.h"
#include "thread_share.h"
void
client(remote *rmt,search *mysearch);

void *client_agent(void *para);
#endif
