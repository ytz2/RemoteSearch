/*
 * general_search.c
 *
 *  Created on: Nov 11, 2013
 *      Author: yanhualiu
 */


/*
 * a more general search to search a given name from level 0
 */
#include "search_given.h"


void search_given(char *given, search *mysearch)
{
	struct stat info;
	stack *temp_stk; /*a temp var to work with history stack */
	Node *temp_node; /* a temp var to work with hsitory stack node */

	errno = 0;
	/*get the file info */
	if (stat(given, &info) == -1) {
		perror(given);
		if(mysearch->client_fd>0)
			send_err_line(mysearch,"%s: %s",given,strerror(errno));
		return;
	}
	/* if it directory */
	if (S_ISDIR(info.st_mode)) {
		/*
		 * if -d is set to 0, do not process directory
		 */
		if (mysearch->max_dir_depth == 0) {
			/*if -q is set */
			if (!(mysearch->options_flags & NO_ERR_MSG))
				{
					fprintf(stderr,
						"%s is detected to exceed the search limit %d\n",
						given, mysearch->max_dir_depth);
					if(mysearch->client_fd>0)
						send_err_line(mysearch,"%s is detected to exceed the search limit %d",
								given, mysearch->max_dir_depth);
				}
			else
			{
				/* update the search statistics */
				pthread_mutex_lock(&(mysearch->lock));
				(mysearch->statistics).err_quiet++;
				pthread_mutex_unlock(&(mysearch->lock));
			}
			return;
		} else {
			/*allocate space to a history stack*/
			if ((temp_stk = malloc(sizeof(stack))) == NULL) {
				perror("malloc():");
				return;
			} else {
				/* initialize the stack and push it on stacks*/
				temp_stk->mysearch=mysearch;
				stack_init(temp_stk);
				/* make a node */
				if ((temp_node = make_node(given, 1, temp_stk)) == NULL) {
					return;
				}
				/*push the root node to the stack and kick it off*/
				stack_push(temp_stk, temp_node, NULL);
				walk_to_next(temp_node);
			}
		}
		return;
	}
	else{
		search_file(given,mysearch, NULL);
	}
	return;
}
