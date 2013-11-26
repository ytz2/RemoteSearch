/*
 * Yanhua Liu (ytz2) CS820
 * general_search.c
 *
 *  Created on: Nov 11, 2013
 *      Author: yanhualiu
 */

/*
 * a more general search to search a given name from level 0
 */
#include "search_given.h"

/*
 * cases like: ./dir, ./dir/ ../dir/.. or ../dir../
 * are resolved in this function.
 * it will be parsed as realpath(../dir/)+ name(..)
 * Note to myself: ~ is automatically parsed by shell as
 * absolute path
 * scan a given directory name
 * if it start with / it is a absolute path and copy it
 * if it is a relative name get its parent's realpath
 * then followed by its name
 */
static char* scan_dir_name(char *given)
{
	char *full_path,*relative_path,*rptr,*memptr,*real_given;
	int i,slash,given_length;
	full_path=NULL;
	given_length=strlen(given);
	slash=-1;
	if (given[0]=='/') /*absolute path*/
	{
		if ((full_path=malloc(given_length+1))==NULL)
		{
			perror("malloc():");
		}else
		{
			strcpy(full_path,given);
		}
	}else /* it is a given directory not starting with '/'*/
	{
		/* scan from 1 since we know it is not / at 0 */
		for (i=1;i<given_length;i++)
		{
			/* record the last appeared slash /
			 * but do not record the slash appeared
			 * at the last character
			 */
			if (given[i]=='/' && i!=given_length-1)
				slash=i;
		}
		/* if there is no slash is just the realpath of '.' +/+ given_name*/
		if (slash==-1)
			{
				relative_path=".";
				real_given=given;
			}
		else /* now we need to parse its realtive path realpath and the real given name*/
		{
			if ((relative_path=malloc(slash+1))==NULL)
			{
				perror("malloc():");
			}else
			{
				strncpy(relative_path,given,slash);
				relative_path[slash]='\0';
			}
			/* parse the real given name */
			if ((real_given=malloc(given_length))==NULL)
			{
				perror("malloc():");
			}else
			{
				/*get the real given name after the parsed relative path name */
				for (i=slash+1;i<given_length;i++)
					real_given[i-slash-1]=given[i];
				real_given[i-slash-1]='\0';
			}
		}

		if ((rptr = get_realpath(relative_path, &memptr, NULL)) == NULL) {
			/* error report has been done within get_realpath */
			if (memptr) {
				free(memptr);
			}
		}
		else /* generate the full path of level 0 from realpath of '.' followed by given name*/
		{
			full_path = get_fullpath(memptr, real_given,1);
			free(memptr);
		}
	}
	return full_path;
}

/*
 * Search_given:
 * to perform search from level 0, only handle file and directory
 * but not stdin
 * */

void search_given(char *given, search *mysearch) {
	struct stat info;
	stack *temp_stk; /*a temp var to work with history stack */
	Node *temp_node; /* a temp var to work with hsitory stack node */
	char *full_path; /* temp string of full path */
	errno = 0;
	/*get the file info */
	if (stat(given, &info) == -1) {
		perror(given);
		if (mysearch->client_fd > 0)
			send_err_line(mysearch, "%s: %s", given, mystrerror(errno));
		return;
	}
	/* if it directory */
	if (S_ISDIR(info.st_mode)) {
		/* get the full path */
		full_path=scan_dir_name(given);
		if (full_path==NULL)
			return;
		/*
		 * if -d is set to 0, do not process directory
		 */
		if (mysearch->max_dir_depth == 0) {
			fprintf(stderr,
					"%s is detected to exceed the search limit %d\n", full_path,
					mysearch->max_dir_depth);
			if (mysearch->client_fd > 0)
				send_err_line(mysearch,
						"%s is detected to exceed the search limit %d",
						full_path, mysearch->max_dir_depth);
			free(full_path);
			return;
		} else {
			/*allocate space to a history stack*/
			if ((temp_stk = malloc(sizeof(stack))) == NULL) {
				perror("malloc():");
				free(full_path);
				return;
			}
			else {
				/* initialize the stack and push it on stacks*/
				temp_stk->mysearch = mysearch;
				stack_init(temp_stk);
				/* make a node */

				if ((temp_node = make_node(full_path, 1, temp_stk)) == NULL) {
					free(full_path);
					free(temp_stk);
					return;
				}
				/*push the root node to the stack and kick it off*/
				stack_push(temp_stk, temp_node, NULL);
				walk_to_next(temp_node);
			}
		}
		return;
	} else {
		search_file(given, mysearch, NULL);
	} /* search it as a file*/
	return;
}
