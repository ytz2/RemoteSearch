/*
 * dirHandle.c
 * Yanhua Liu
 * This file defines directory processing functions
 * For CS820 Assignment 2
 * Created on: Oct 17, 2013
 * Author: Yanhua Liu ytz2
 */

#include "dirHandle.h"

/*
 * make a history stack node
 */
Node* make_node(char *dname, int depth, stack *stk) {
	Node* temp;
	DIR *dir;
	char* rptr, *memptr,*full;
	search *mysearch;
	mysearch = stk->mysearch;
	errno = 0;
	rptr = NULL;
	memptr = NULL;
	full=dname;
	/* give space to a node */
	if ((temp = (Node*) malloc(sizeof(Node))) == NULL) {
		perror("malloc()");
		return NULL;
	}

	/* initialize the node */
	temp->counter = 1; // set to 1 at the first time to be made
	temp->prev = NULL; // will be pointed to its parent dir
	temp->depth = depth;
	temp->stk = stk;
	memset(&temp->statistics, 0, sizeof(Statistics));



	/* open the direcotry with full path */
	if ((dir = opendir(full)) == NULL) {
		if (!(stk->mysearch->options_flags & NO_ERR_MSG)) {
			/* if -q is not set */
			perror(full);
			if (mysearch->client_fd > 0)
				send_err_line(mysearch, "%s: %s", full, mystrerror(errno));
		} else {
			/* since it will be deleted, we directly update to mysearch*/
			pthread_mutex_lock(&(mysearch->lock));
			(mysearch->statistics).err_quiet++;
			pthread_mutex_unlock(&(mysearch->lock));
		}
		free(temp);
		return NULL;
	}

	/* use full path name to get real path */
	if ((rptr = get_realpath(full, &memptr, temp)) == NULL) {
		/* error report has been done within get_realpath */
		if (memptr) {
			free(memptr);
			free(temp);
			closedir(dir);
		}
		return NULL;
	}
	/* initialize the node */
	temp->full_path=dname; /*setup the full path */
	temp->counter = 1; // set to 1 at the first time to be made
	temp->dir = dir; // the dir object about the path
	temp->path = memptr; // realpath of the dir
	temp->prev = NULL; // will be pointed to its parent dir
	temp->depth = depth;
	temp->stk = stk;
	memset(&temp->statistics, 0, sizeof(Statistics));
	return temp;
}/* Node* make_node  */

/*
 * do some housekeeping about a node
 */
void clear_node(Node* node) {
	if (node->dir)
		closedir(node->dir);
	if (node->path)
		free(node->path);
	if (node->full_path)
		free(node->full_path);
	free(node);
}
/*/
 * Initialize a stack
 */
int stack_init(stack *st) {
	int err;
	st->head = NULL;
	memset(&(st->statistics), 0, sizeof(Statistics));

	/* increment the stack count to keep track of
	 * how many stack in heap
	 */
	pthread_mutex_lock(&(st->mysearch->lock));
	st->mysearch->stk_count++;
	pthread_mutex_unlock(&(st->mysearch->lock));

	// initialize the lock
	err = pthread_rwlock_init(&st->s_lock, NULL);
	if (err != 0)
		return (err);
	// initialize the thread attribute
	err = pthread_attr_init(&st->attr);
	if (err != 0)
		return (err);
	// set the attribute to be detached
	err = pthread_attr_setdetachstate(&st->attr, PTHREAD_CREATE_DETACHED);
	if (err != 0)
		return (err);
	return 0;
}
/*
 * destroy the stack
 */

int stack_destroy(stack *st) {
	int err;
	if ((err = pthread_attr_destroy(&st->attr)) != 0)
		return err;
	if ((err = pthread_rwlock_destroy(&st->s_lock)) != 0)
		return err;
	return 0;
}

/*
 * push a node on the "top" of the tree-like stack
 */
int stack_push(stack *st, Node *current, Node *next) {
	/* do some safe check */
	int err;
	if (current == NULL)
		return -1;

	if ((err = pthread_rwlock_wrlock(&st->s_lock)) != 0) {
		perror("rwlock");
		return err;
	}

	/* when the head is null set the current as head
	 * this handles the call stack_push(st,current,NULL)
	 * */
	if (st->head == NULL)
		st->head = current;
	else
		/* if one subdirectory added, the current counter increase*/
		/* link the next to the current node*/
	{
		current->counter++;
		next->prev = current;
	}

	if ((err = pthread_rwlock_unlock(&st->s_lock)) != 0) {
		perror("rwlock");
		return err;
	}
	return 0;
}

/*
 * pop a node on the "top" of the tree-like stack
 */
int stack_job_done(stack *st, Node *current) {
	int err;
	Node *temp, *tofree;
	temp = current;
	if ((err = pthread_rwlock_wrlock(&st->s_lock)) != 0) {
		perror("rwlock");
		return err;
	}
	// decrement the leaf counter

	/*
	 * from the current node to loop, if the counter of
	 * the current node is 0, clean the node and continue
	 * to check, else just stop
	 */

	while (temp != NULL) {
		/* decrement the counter of the current ptr
		 * this happens firstly for the leaf dir
		 */
		temp->counter--;

		/* if the counter is 0, trigger the avalanche */
		if (temp->counter == 0) {
			/* before clear the node, update the statistics */
			update_statistics(&(st->statistics), &(temp->statistics));
			tofree = temp;
			temp = temp->prev;
			clear_node(tofree); // delete the node
		} else
			/* if counter is not 0, it means its job has not been finished*/
			break;
	}

	if ((err = pthread_rwlock_unlock(&st->s_lock)) != 0) {
		perror("rwlock");
		return err;
	}
	if (temp == NULL)
		/*if it is the root */
	{
		/* before destroy the stack update the stack statistics
		 * to search object statistics
		 */
		pthread_mutex_lock(&(st->mysearch->lock));
		update_statistics(&(st->mysearch->statistics), &(st->statistics));
		st->mysearch->stk_count--;
		pthread_cond_signal(&(st->mysearch->ready));
		pthread_mutex_unlock(&(st->mysearch->lock));

		stack_destroy(st);
		free(st);
	}
	return 0;
}

/*
 *  find the realpath which used to appear in the tree
 */
Node* stack_find_history(stack *st, Node* current, Node *next) {
	int err;
	Node *temp, *iter;
	search *mysearch;
	char *path,*fullpath;
	int depth;
	path=next->path;
	fullpath=next->full_path;
	mysearch = st->mysearch;
	depth=next->depth;

	if ((err = pthread_rwlock_rdlock(&st->s_lock)) != 0) {
		perror("rwlock");
		return NULL;
	}

	/* loop to find whether a given path in the history stack */
	for (temp = current; temp != NULL; temp = temp->prev) {
		if (strcmp(temp->path, path) == 0)
			break;
	}

	if (temp != NULL) {
		{
			if (!(mysearch->options_flags & NO_ERR_MSG)) {
				fprintf(stderr, "Loop at level %d due to %s\n",
						depth, fullpath);
				fprintf(stderr, "%*clevel %d: %s\n", 3*depth, ' ',depth,
						path);
				if (mysearch->client_fd > 0){
					send_err_line(mysearch, "Loop at level %d due to %s",
							depth, fullpath);
					send_err_line(mysearch, "%*clevel %d: %s", 3*depth, ' ',depth,
							path);
				}

			} else
				(current->statistics).err_quiet++;
			/* update the avoided loop directory */
			(current->statistics).loop_avoided++;
		}
		/*
		 * go back to print all the loop elements in the loop branch
		 * in the history stack
		 */
		for (iter = current; iter != temp->prev; iter = iter->prev) {
			if (!(mysearch->options_flags & NO_ERR_MSG)) {
				fprintf(stderr, "%*clevel %d: %s\n", 3 * depth, ' ',iter->depth,
						iter->path);
				if (mysearch->client_fd > 0)
					send_err_line(mysearch, "%*clevel %d: %s", 3 * depth, ' ',iter->depth,
							iter->path);
				fflush(stderr);
			} else {
				(current->statistics).err_quiet++;
			}
		}
	}
	if ((err = pthread_rwlock_unlock(&st->s_lock)) != 0) {
		perror("rwlock");
		return NULL;
	}
	return temp;
}


/*
 * test if the symlink pointing to a dir
 * if it is, return bool 1
 * else false 0,failure -1
 */
int is_sym_dir(char* full_name, search *mysearch, Node *current) {
	struct stat st;
	int val;
	errno = 0;
	/*
	 * use the stat to follow the link
	 */
	if (stat(full_name, &st) == -1) {
		/* if the link does not exist, issue error*/
		if (!(mysearch->options_flags & NO_ERR_MSG)) {
			perror(full_name);
			if (mysearch->client_fd > 0)
				send_err_line(mysearch, "%s:%s", full_name, mystrerror(errno));
		} else {
			(current->statistics).err_quiet++;
		}
		val = -1;
	} else if (S_ISDIR(st.st_mode))
		val = 1;
	else
		val = 0;

	return val;
}

/*
 * recursively walk the directory
 */
int walk_recur(Node* current) {
	DIR *dir; //directory obj ptr
	char *current_path, *full_path; //realpath and fullpath
	struct dirent *entry; //directory entry
	struct dirent *result; //to work with readdir_r
	int err;
	Node *next; // next dir
	Node *prev; // previous dir in history stack
	int sym_link_flag; // if sym_link_flag is set, it means a link to an openable dir
	int depth; // current depth
	stack *stk; // history stack
	long name_max;
	struct stat st;
	unsigned int options_flags;
	search *mysearch;
	/* get the current directory info*/
	depth = current->depth;
	stk = current->stk;
	dir = current->dir;
	current_path = current->path; // this is the real path
	options_flags = stk->mysearch->options_flags;
	mysearch = stk->mysearch;
	/* update the depth for statistics directory */
	(current->statistics).max_depth = current->depth;

	/* use the realpath to get the max name_max */
	name_max = pathconf(current_path, _PC_NAME_MAX);
	if (name_max <= 0)
		name_max = 4096; /* arbitrarily large */
	if ((entry = malloc(sizeof(struct dirent) + name_max + 1)) == NULL) {
		if (!(options_flags & NO_ERR_MSG))
			fprintf(stderr, "No space for dirent buffer\n");
		return -1;
	}

	/* now loop directory */

	while (1) {
		if ((err = readdir_r(dir, entry, &result)) != 0) {
			if (!(options_flags & NO_ERR_MSG)) {
				perror("readdir_r");
				if (mysearch->client_fd > 0)
					send_err_line(mysearch, "%s: %s", "readdir_r",
							mystrerror(errno));
			} else {
				(current->statistics).err_quiet++;
			}
			break;
		}
		if (result == NULL) /* just hit EOF */
			break;

		// initialize a flag to indicate whether it is a sym_link
		sym_link_flag = -2;

		/* neglect the . and .. */
		if (!strcmp(result->d_name, ".") || !strcmp(result->d_name, ".."))
			continue;

		/*	handle the files or directories start with .	 */
		if (result->d_name[0] == '.') {
			if (!(options_flags & DOT_ACCESS))
				continue;
			else
				/* update the -a statistics directory */
				(current->statistics).dot_caught++;
		}

		/* get the fullpath of the file/directory*/
		full_path = get_fullpath(current_path, result->d_name,
				!(options_flags & NO_ERR_MSG));

		/* get the stat info of subdir/file*/
		if (lstat(full_path, &st) == -1) {
			if (!(options_flags & NO_ERR_MSG)) {
				perror(full_path);
				if (mysearch->client_fd > 0)
					send_err_line(mysearch, "%s: %s", full_path,
							mystrerror(errno));
			} else {
				(current->statistics).err_quiet++;
			}

			free(full_path);
			continue;
		}
		/* deal with symlink -f*/
		if (S_ISLNK(st.st_mode)) {
			/* if -f is set, do not follow link */
			if (options_flags & NOT_FOLLOW_LINK) {
				if (!(options_flags & NO_ERR_MSG)) {
					fprintf(stderr, "Symlink: %s\n", full_path);
					if (mysearch->client_fd > 0)
						send_err_line(mysearch, "Symlink: %s", full_path);
				} else {
					(current->statistics).err_quiet++;
				}

				(current->statistics).link_ignored++;
				free(full_path);
				continue;
			}
			/* use stat to follow the symlink to test whether the symlink to
			 * dir or file exist
			 */
			sym_link_flag = is_sym_dir(full_path, mysearch, current);
			/* if it does not exist, just go to process next one */
			if (sym_link_flag == -1) {
				free(full_path);
				continue;
			}
		}
		//sleep(1);
		/* if it is the directory or soft link to a directory */
		if (S_ISDIR(st.st_mode) || sym_link_flag == 1) {
			/* if the max_dir_depth is defined and it reached this limit*/
			if (stk->mysearch->max_dir_depth >= 0
					&& depth == stk->mysearch->max_dir_depth) {

				/* update the -d statistics directory */
				(current->statistics).dir_pruned++;
				if (!(options_flags & NO_ERR_MSG)) {
					fprintf(stderr,
							"%s is detected to exceed the search limit %d\n",
							full_path, stk->mysearch->max_dir_depth);
					if (mysearch->client_fd > 0)
						send_err_line(mysearch,
								"%s is detected to exceed the search limit %d",
								full_path, stk->mysearch->max_dir_depth);

				} else {
					(current->statistics).err_quiet++;
				}
				free(full_path);
				continue;
			}
			/* make a node contains info */
			if ((next = make_node(full_path, depth + 1, stk)) == NULL) {
				free(full_path);
				continue;
			}
			/* update the opened directory */
			(current->statistics).dir_opened++;
			/* check if it used to appear in the history stack*/
			if ((prev = stack_find_history(stk, current, next))
					!= NULL) {
				clear_node(next);
				//free(full_path);
				continue;
			}
			/* push the next node to the stack on the top of current node*/
			if ((err = stack_push(stk, current, next)) != 0) {
				clear_node(next);
				//free(full_path);
				continue;
			}

			/* recursively search */
			walk_to_next(next);
			//free(full_path);
			fflush(stderr);
			continue;
		}
		/*now it is the file */
		search_file(full_path, mysearch, current);
		free(full_path);
		fflush(stderr);
	}
	if (entry != NULL)
		free(entry);
	stack_job_done(stk, current);
	return 0;
}

/*
 * wrapper function for directory search
 */
void* search_dir(void *para) {
	long err;
	Node *next;
	search *mysearch;
	/*
	 * create the key
	 */
	pthread_once(&init_done, thread_init);
	err = 0;
	next = (Node*) para;
	mysearch = next->stk->mysearch;

	pthread_mutex_lock(&(mysearch->lock));
	/* update the descend thread created directly */
	(mysearch->statistics).thread_created++;
	pthread_mutex_unlock(&(mysearch->lock));

	err = walk_recur(next);
	fflush(stderr);

	pthread_mutex_lock(&(mysearch->lock));
	(mysearch->statistics).max_alive = MAX((mysearch->statistics).max_alive,
			(unsigned int)mysearch->alive_threads);
	mysearch->alive_threads--;
	pthread_mutex_unlock(&(mysearch->lock));

	pthread_exit((void*) err);
	return (void*) err;
}
/*
 * another wrapper function, it will
 * test if the alive threads
 * number is less than the limit, then do thread_create
 * else, use walk_recur
 */
int walk_to_next(Node* next) {
	pthread_t id; // thread id
	int err, thread_limits;
	search *mysearch;
	err = 1;
	mysearch = next->stk->mysearch;
	thread_limits = mysearch->thread_limits;
	/*if the thread number limit is 0, use main thread*/
	if (thread_limits == 0) {
		return walk_recur(next);
	}

	/* if -t is not defined, create a thread if possible */
	if (thread_limits < 0) {
		err = pthread_create(&id, &next->stk->attr, search_dir, (void*) next);
		if (err != 0)
			perror("Pthread");
		else if (err == 0) {
			/*lock the counter*/
			pthread_mutex_lock(&(mysearch->lock));
			mysearch->alive_threads++;
			pthread_mutex_unlock(&(mysearch->lock));
		}
	} else // thread_limits>0
	{
		/*lock the counter*/
		pthread_mutex_lock(&(mysearch->lock));
		/* check if the alive threads is below the thread limits */
		if (mysearch->alive_threads < thread_limits) {
			err = pthread_create(&id, &next->stk->attr, search_dir,
					(void*) next);
			if (err != 0) {
				perror("Pthread");
			}
		}
		/* if thread creation succeed, increment the alive threads #*/
		if (err == 0)
			//alive_threads++;
			mysearch->alive_threads++;
		else
			(mysearch->statistics).thread_not_created++;
		pthread_mutex_unlock(&(mysearch->lock));
	}

	/* if thread creation failed, issue an error and continue with the current thread */
	if (err != 0)
		return walk_recur(next);

	return 0;
}
