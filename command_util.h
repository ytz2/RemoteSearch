/*
 * This header file defines helper types
 * in network communication, dir handling
 * and function calling parameters.
 * Utility functions are listed as below
 * command_util.h
 *Yanhua Liu (ytz2) CS820
 *  Created by Yanhua Liu for CS820 assignment 3
 *  Created on: Sep 5, 2013
 *  This header file provide the utilities to
 *  implement the getopt interface
 */

#ifndef COMMAND_UTIL_H_
#define COMMAND_UTIL_H_
#include "ospenv.h"/* this header defines POSIX, ISOC, XOPEN and EXTENSIONS */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <arpa/inet.h>
/* predefine parameters to work with getopt interface*/

#define OPTIONS ":hbeipvafql:m:n:d:t:"
#define WHITE_SPACE " \n\r\t\v\f"
#define WHITE_SPACE " \n\r\t\v\f"
#define DOT_ACCESS          0x01  /* -a switch */
#define AT_BEGIN            0x02  /* -b switch */
#define AT_END              0x04  /* -e switch */
#define NOT_FOLLOW_LINK     0x08  /* -f switch */
#define CASE_INSENSITIVE    0x10  /* -i switch */
#define SHOW_PATH           0x20  /* -p switch */
#define NO_ERR_MSG          0x40  /* -q switch */
#define INVERSE_PRINT       0x80  /* -v default is */

/* magic number definition */
#define DEFAULT_LINE_BUFFER 255
#define MAX_LINE_BUFFER     4096
#define MAX_COLS            16
#define MAX_FILES           1024
#define STREAM_REDIRECT     "-"

#define PORT_MAX 5
#ifdef HOST_NAME_MAX
#define HOSTMAX HOST_NAME_MAX
#else
#define HOSTMAX 1024
#endif

#define REMOTE_NAME_MAX 1024
#define NFLAGS 6
#define MAX_SEARCH_STR 1024

#define MAX_TCP_STD 4096*2
#define MAX_TCP_ERR MAX_TCP_STD
#define RIGHT_JUST 55

#ifndef MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif

typedef struct Message_Six {
	unsigned int link_ignored; /*total link ignored*/
	unsigned int dir_opened; /*total dir opened*/
	unsigned int loop_avoided; /* total loop avoided */
	unsigned int dir_pruned; /* dir get pruend by -d */
	unsigned int max_depth; /*max descent */
	unsigned int dot_caught; /* dot not ignored */
	unsigned int thread_created; /* total thread created */
	unsigned int thread_not_created; /* thread not created due to -t */
	unsigned int max_alive; /* max simutaneous alive thread*/
	unsigned int err_quiet; /* msg due to -q */
	unsigned int lines_matched; /* lines get matched */
	unsigned int lines_read; /*total lines get read*/
	unsigned int file_read;
	unsigned int bytes_read;
} Statistics;

/*wrap the command parameters to a structure to faciliate
 * the assignment 3
 */
typedef struct Search_info {
	unsigned int options_flags; /* work with switch options_flags*/
	int *shift_table; /* shift table*/
	int line_buffer_size; /* work with -l number*/
	int max_line_number; /* work with -m number*/
	int column_number; /* work with -n number*/
	char *search_pattern; /* the search pattern*/
	int thread_limits; /* work with -t number*/
	int max_dir_depth; /* work with -d number */
	int client_fd; /* to work with the server */
	//int thread_done; /* to work on server side*/
	unsigned int stk_count;
	int alive_threads; /* total alive threads */
	Statistics statistics; /* structure to contain the statistics */
	pthread_cond_t ready;
	pthread_mutex_t lock;
} search;

typedef struct Remote {
	char node[HOSTMAX + 1];
	char port[PORT_MAX + 1];
	char name[REMOTE_NAME_MAX + 1];
} remote; /* node:port/name */

typedef struct Client_parameters {
	remote* rmt;
	search* mysearch;
} Client_para;

typedef struct Message_one {
	unsigned int options_flags; /* work with switch options_flags*/
	int max_dir_depth; /* work with -d number */
	int line_buffer_size; /* work with -l number*/
	int max_line_number; /* work with -m number*/
	int column_number; /* work with -n number*/
	int thread_limits; /* work with -t number*/
} msg_one;
/* define the element structure for history stack */
typedef struct STACK stack; //pre declaration
typedef struct NODE {
	unsigned long counter; // counter of reference to itself
	char *path; //realpath
	DIR *dir; //the dir object
	struct NODE *prev; // link to its previous node
	int depth;
	stack *stk;
	Statistics statistics; /* statistics to report */
	char *full_path;
} Node;

/*
 * the stack type is made independent of Node
 * in order to provide a better interface
 */
struct STACK {
	/*the head or root is useless, but leave it here in case useful tomorrow*/
	Node *head;
	/*
	 * the stack is protected by rwlock, since most of time is not doing push
	 * but doing the searching whether a realpath used to appear
	 */
	pthread_rwlock_t s_lock;
	/*
	 * the attribute will be set to be detached to make sure onece
	 * a thread finished, the resource of that thread releases in time.
	 */
	pthread_attr_t attr;
	Statistics statistics;
	search *mysearch;
};

/* Function Utilities*/

/* Scans the string pointed to by optarg and tries to convert it to a number.
 * Returns 0 if successful (and stores the number in result),
 *	  -1 on any error (prints an error message and leaves result unchanged)
 */

int scan_switch_number(int switch_char, int *result);

/* If buffer ends with a new-line character, remove that character.
 * Returns number of characters remaining in buffer.
 */
int
trim_line(char *buffer);

/* print the flag value, this is used to debug*/
void print_flag(unsigned int flags, unsigned int this_one, char *name);

/*initialize the search request*/
void init_search(search **mysearch);
/*destroy the search*/
void destroy_search(search *mysearch);
/* wrapper of getopt function */
void scan_opt_search(int argc, char *argv[], search *mysearch);

/* check if the list of file contain the : to indicate a remote search */
remote* scan_remote_search(char *input, int *flag);

/*print the statistics*/
void print_stat(FILE* fptr, Statistics *statistics, double tdiff);

/*update the statistics within dir */
void update_statistics(Statistics *root, Statistics *node);
/*update all the statistics*/
void update_statistics_sock(Statistics *root, Statistics *node);
/*convert to network endian for sending*/
void trans_stat2send(Statistics *st);
/*convert from network endian to achitecture endian*/
void trans_stat2recv(Statistics *st);
/*
 * get the full path from the realpath and d_name
 */
char* get_fullpath(char* rpath, char *fname, int flag);


#endif /* COMMAND_UTIL_H_ */
