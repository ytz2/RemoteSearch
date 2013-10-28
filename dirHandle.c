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
 * debuging purpose
 * usage: dbg(__func__,msg,__LINE__)
 *        or dbg(__func__,node->path,node->counter)
 * leave here for assignment 3, no use in current project
 * and leave it here.
 */

/*static void dbg(const char* func, const char* path, int count) {
	fprintf(stderr, "%s: %s is %d \n", func, path, count);
	fflush(stderr);
}*/

/*global lock for counter*/
static pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;
/* global parameter to count how many alive threads in use */
static long alive_threads = 0;

/*
 * make a history stack node
 */
Node* make_node(char *dname, int depth, stack *stk) {
	Node* temp;
	DIR *dir;
	char* rptr, *memptr;

	errno = 0;
	rptr=NULL;
	memptr=NULL;

	/* use full path name to get real path */
	if ((rptr = get_realpath(dname, &memptr)) == NULL) {
		/* error report has been done within get_realpath */
		if (memptr)
			free(memptr);
		return NULL;
	}

	/* open the direcotry with full path */
	if ((dir = opendir(rptr)) == NULL) {
		if (!(options_flags & NO_ERR_MSG))
			perror(rptr);
		return NULL;
	}

	/* give space to a node */
	if ((temp = (Node*) malloc(sizeof(Node))) == NULL) {
		if (!(options_flags & NO_ERR_MSG))
			perror("malloc()");
		return NULL;
	}

	/* initialize the node */
	temp->counter = 1; // set to 1 at the first time to be made
	temp->dir = dir; // the dir object about the path
	temp->path = memptr; // realpath of the dir
	temp->prev = NULL; // will be pointed to its parent dir
	temp->depth = depth;
	temp->stk = stk;
	return temp;
}

/*
 * do some housekeeping about a node
 */
void clear_node(Node* node) {
	if (node->dir)
		closedir(node->dir);
	if (node->path)
		free(node->path);
	free(node);
}
/*/
 * Initialize a stack
 */
int stack_init(stack *st) {
	int err;
	st->head = NULL;
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
		if (!(options_flags & NO_ERR_MSG))
			fprintf(stderr, "rwlock_wrlock: %s\n", strerror(err));
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
		if (!(options_flags & NO_ERR_MSG))
			fprintf(stderr, "rwlock_unlock: %s\n", strerror(err));
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
		if (!(options_flags & NO_ERR_MSG))
			fprintf(stderr, "rwlock_wrlock: %s\n", strerror(err));
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
			tofree = temp;
			temp = temp->prev;
			clear_node(tofree); // delete the node
		} else
			/* if counter is not 0, it means its job has not been finished*/
			break;
	}

	if ((err = pthread_rwlock_unlock(&st->s_lock)) != 0) {
		if (!(options_flags & NO_ERR_MSG))
			fprintf(stderr, "rwlock_unlock: %s\n", strerror(err));
		return err;
	}
	return 0;
}

/*
 *  find the realpath which used to appear in the tree
 */
Node* stack_find_history(stack *st, Node* current, char *path, char *fullpath) {
	int err;
	Node *temp, *iter;
	if ((err = pthread_rwlock_rdlock(&st->s_lock)) != 0) {
		if (!(options_flags & NO_ERR_MSG))
			fprintf(stderr, "rwlock_rdlock: %s\n", strerror(err));
		return NULL;
	}

	/* loop to find whether a given path in the history stack */
	for (temp = current; temp != NULL; temp = temp->prev) {
		if (strcmp(temp->path, path) == 0)
			break;
	}

	if (temp != NULL && !(options_flags & NO_ERR_MSG)) {
		fprintf(stderr, "%d depth: %s detect a loop\n", current->depth + 1, fullpath);
		/*
		 * go back to print all the loop elements in the loop branch
		 * in the history stack
		 */
		for (iter = current; iter != temp->prev; iter = iter->prev) {
			fprintf(stderr, "%*c%d depth: %s\n", 3 * iter->depth, ' ',
					iter->depth, iter->path);
			fflush(stderr);
		}
	}
	if ((err = pthread_rwlock_unlock(&st->s_lock)) != 0) {
		if (!(options_flags & NO_ERR_MSG))
			fprintf(stderr, "rwlock_unlock: %s\n", strerror(err));
		return NULL;
	}
	return temp;
}

/*
 * get the full path from the realpath and d_name
 */
char* get_fullpath(char* rpath, char *fname) {
	/*
	 * basic string operations
	 * give a buffer, copy the realpath
	 * realpath+'/'+filename=full_path
	 */
	char *fullpath;
	int name_max;
	int len;
	len = strlen(rpath);
	name_max = pathconf(rpath, _PC_NAME_MAX);
	if (name_max <= 0)
		name_max = 4096; /* arbitrarily large */
	/* rapth+/+name_max+'/0'=strlen(rapth)+name_max+2, for safety+5*/
	if ((fullpath = malloc(len + name_max + 5)) == NULL) {
		if (!(options_flags & NO_ERR_MSG))
			perror("malloc()");
		return NULL;
	}
	strncpy(fullpath, rpath, len);
	fullpath[len] = '/';
	fullpath[++len] = '\0';
	strcat(fullpath, fname);
	return fullpath;
}

/*
 * test if the symlink pointing to a dir
 * if it is, return bool 1
 * else false 0,failure -1
 */
int is_sym_dir(char* full_name) {
	struct stat st;
	int val;
	errno = 0;
	/*
	 * use the stat to follow the link
	 */
	if (stat(full_name, &st) == -1) {
		/* if the link does not exist, issue error*/
		if (!(options_flags & NO_ERR_MSG)) {
			fprintf(stderr, "stat(): %s %s\n", full_name, strerror(errno));
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
	/* get the current directory info*/
	depth = current->depth;
	stk = current->stk;
	dir = current->dir;
	current_path = current->path;

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
			if (!(options_flags & NO_ERR_MSG))
				perror("readdir_r");
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
		if (!(options_flags & DOT_ACCESS) && result->d_name[0] == '.')
			continue;

		/* generate the full path of subdirectory or files
		 * hmmm don't forget to free full_path when any condition
		 * cause to neglect one step
		 */
		full_path = get_fullpath(current_path, result->d_name);

		/* get the stat info of subdir/file*/
		if (lstat(full_path, &st) == -1) {
			if (!(options_flags & NO_ERR_MSG))
				perror(full_path);
			free(full_path);
			continue;
		}
		/* deal with symlink -f*/
		if (S_ISLNK(st.st_mode)) {
			/* if -f is set, do not follow link */
			if (options_flags & NOT_FOLLOW_LINK) {
				if (!(options_flags & NO_ERR_MSG))
					fprintf(stderr, "Symlink: %s\n", full_path);
				free(full_path);
				continue;
			}
			/* use stat to follow the symlink to test whether the symlink to
			 * dir or file exist
			 */
			sym_link_flag = is_sym_dir(full_path);
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
			if (max_dir_depth >= 0 && depth == max_dir_depth) {
				if (!(options_flags & NO_ERR_MSG)) {
					fprintf(stderr,
							"%s is detected to exceed the search limit %d\n",
							full_path, max_dir_depth);
				}
				free(full_path);
				continue;
			}
			/* make a node contains info */
			if ((next = make_node(full_path, depth + 1, stk)) == NULL) {
				free(full_path);
				continue;
			}
			/* check if it used to appear in the history stack*/
			if ((prev = stack_find_history(stk, current, next->path, full_path))
					!= NULL) {
				clear_node(next);
				free(full_path);
				continue;
			}
			/* push the next node to the stack on the top of current node*/
			if ((err = stack_push(stk, current, next)) != 0) {
				clear_node(next);
				free(full_path);
				continue;
			}
			/* recursively search */
			walk_to_next(next);
			free(full_path);
			fflush(stderr);
			continue;
		}
		search_file(full_path, search_pattern, 1);
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
	long err = 0;

	err = walk_recur((Node*) para);

	fflush(stderr);

	/*decrement the counter when firstly enter*/
	if (thread_limits > 0) {
		pthread_mutex_lock(&counter_lock);
		alive_threads--;
		pthread_mutex_unlock(&counter_lock);
	}

	pthread_exit((void*) err);
	return (void*) err;
}
/*
 * another wrapper function, it will
 * test if alive threads, if the alive threads
 * number is less than the limit, then do thread_create
 * else, use walk_recur
 */
int walk_to_next(Node* next) {
	pthread_t id; // thread id
	int err;
	err = 1;

	/*if the thread number limit is 0, use main thread*/
	if (thread_limits == 0) {
		return walk_recur(next);
	}

	/* if -t is not defined, create a thread if possible */
	if (thread_limits < 0)
	{
		err = pthread_create(&id, &next->stk->attr, search_dir, (void*) next);
		if (err!=0 && !(options_flags & NO_ERR_MSG))
			perror("Pthread");
	}
	else // thread_limits>0
	{
		/*lock the counter*/
		pthread_mutex_lock(&counter_lock);
		/* check if the alive threads is below the thread limits */
		if (alive_threads < thread_limits)
		{
			err = pthread_create(&id, &next->stk->attr, search_dir,
					(void*) next);
			if (err!=0 && !(options_flags & NO_ERR_MSG))
			{
				perror("Pthread");
			}
		}
		/* if thread creation succeed, increment the alive threads #*/
		if (err == 0)
			alive_threads++;
		pthread_mutex_unlock(&counter_lock);
	}

	/* if thread creation failed, issue an error and continue with the current thread */
	if (err != 0)
		return walk_recur(next);

	return 0;
}
