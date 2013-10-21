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
static void dbg(const char* func, const char* path, int count) {
	fprintf(stderr, "%s: %s is %d \n", func, path, count);
	fflush(stderr);
}
/*
 * make a history stack node
 */
Node* make_node(char *dname, int depth, stack *stk) {
	Node* temp;
	DIR *dir;
	char* rptr, *memptr;

	errno = 0;

	/* use full path name to get real path */
	if ((rptr = get_realpath(dname, &memptr)) == NULL ) {
		/* error report has been done within get_realpath */
		if (memptr)
			free(memptr);
		return NULL ;
	}

	/* open the direcotry with full path */
	if ((dir = opendir(rptr)) == NULL ) {
		if (!no_err_msg)
			perror(rptr);
		return NULL ;
	}

	/* give space to a node */
	if ((temp = (Node*) malloc(sizeof(Node))) == NULL ) {
		if (!no_err_msg)
			perror("malloc()");
		return NULL ;
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
/*/*
 * Initialize a stack
 */
int stack_init(stack *st) {
	int err;
	st->head = NULL;
	// initialize the lock
	err = pthread_rwlock_init(&st->s_lock, NULL );
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
	st->thread_counts = 0;
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
	if (current == NULL )
		return -1;

	int err;
	if ((err = pthread_rwlock_wrlock(&st->s_lock)) != 0) {
		if (!no_err_msg)
			fprintf(stderr, "rwlock_wrlock: %s\n", strerror(err));
		return err;
	}

	/* when the head is null set the current as head
	 * this handles the call stack_push(st,current,NULL)
	 * */
	if (st->head == NULL )
		st->head = current;
	else
	/* if one subdirectory added, the current counter increase*/
	/* link the next to the current node*/
	{
		current->counter++;
		next->prev = current;
	}

	if ((err = pthread_rwlock_unlock(&st->s_lock)) != 0) {
		if (!no_err_msg)
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
		if (!no_err_msg)
			fprintf(stderr, "rwlock_wrlock: %s\n", strerror(err));
		return err;
	}
	// decrement the leaf counter

	/*
	 * from the current node to loop, if the counter of
	 * the current node is 0, clean the node and continue
	 * to check, else just stop
	 */
	while (temp != NULL ) {
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
		if (!no_err_msg)
			fprintf(stderr, "rwlock_unlock: %s\n", strerror(err));
		return err;
	}
	return 0;
}

/*
 *  find the realpath which used to appear in the tree
 */
Node* stack_find_history(stack *st, Node* current, char *path) {
	int err;
	Node *temp;
	if ((err = pthread_rwlock_rdlock(&st->s_lock)) != 0) {
		if (!no_err_msg)
			fprintf(stderr, "rwlock_rdlock: %s\n", strerror(err));
		return NULL ;
	}

	/* loop to find whether a given path in the history stack */
	for (temp = current; temp != NULL ; temp = temp->prev) {
		if (strcmp(temp->path, path) == 0)
			break;
	}

	if ((err = pthread_rwlock_unlock(&st->s_lock)) != 0) {
		if (!no_err_msg)
			fprintf(stderr, "rwlock_unlock: %s\n", strerror(err));
		return NULL ;
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
	int len;
	fullpath = path_alloc(NULL );
	len = strlen(rpath);
	strcpy(fullpath, rpath);
	fullpath[len++] = '/';
	fullpath[len] = '\0';
	strcat(fullpath, fname);
	return fullpath;
}

/*
 * test if the symlink pointing to a dir
 * if it is, return bool 1
 * else false 0
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
		if (!no_err_msg)
			perror(full_name);
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
struct stat st;
int walk_recur(Node* current) {

	DIR *dir; //directory obj ptr
	char *current_path, *full_path; //realpath and fullpath
	struct dirent entry; //directory entry
	struct dirent *result; //to work with readdir_r
	int err;
	Node *next; // next dir
	Node *prev; // previous dir in history stack
	Node *temp; // helper node to loop stack
	int sym_link_flag; // if sym_link_flag is set, it means a link to an openable dir
	int depth; // current depth
	stack *stk; // history stack

	/* get the current directory info*/
	depth = current->depth;
	stk = current->stk;
	dir = current->dir;
	current_path = current->path;

	printf("%*s: thread id: %lu\n", 3 * depth + (int) strlen(current->path),
			current->path, pthread_self());
	/* now loop directory */
	for (err = readdir_r(dir, &entry, &result); result != NULL && err == 0;
			err = readdir_r(dir, &entry, &result)) {
		// initialize a flag to indicate whether it is a sym_link
		sym_link_flag = 0;
		/* neglect the . and .. */
		if (!strcmp(result->d_name, ".") || !strcmp(result->d_name, ".."))
			continue;

		/*	handle the files or directories start with .	 */
		if (dot_no_access && result->d_name[0] == '.')
			continue;

		/* generate the full path of subdirectory or files
		 * hmmm don't forget to free full_path when any condition
		 * cause to neglect one step
		 */
		full_path = get_fullpath(current_path, result->d_name);

		/* get the stat info of subdir/file*/
		if (lstat(full_path, &st) == -1) {
			if (!no_err_msg)
				perror(full_path);
			free(full_path);
			continue;
		}

		/* deal with symlink -f*/
		if (S_ISLNK(st.st_mode)) {
			/* if -f is set, do not follow link */
			if (not_follow_link) {
				if (!no_err_msg)
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

		if (depth == MAX_DEPTH && (S_ISDIR(st.st_mode) || sym_link_flag == 1)) {
			if (!no_err_msg) {
				fprintf(stderr,
						"%s is detected to exceed the search limit %d\n",
						full_path, MAX_DEPTH);
			}
			free(full_path);
			continue;
		}
		/* if it is the directory or symlink to a directory */
		if (depth < MAX_DEPTH && (S_ISDIR(st.st_mode) || sym_link_flag == 1)) {
			/* make a node contains info */
			if ((next = make_node(full_path, depth + 1, stk)) == NULL ) {
				free(full_path);
				continue;
			}
			/* check if it used to appear in the history stack*/
			else if ((prev = stack_find_history(stk, current, next->path))
					!= NULL ) {
				if (!no_err_msg) {
					fprintf(stderr, "%s is detected to cause a loop\n",
							full_path);
					/*
					 * go back to print all the loop elements in the loop branch
					 * in the history stack
					 */
					for (temp = current; temp != prev->prev; temp =
							temp->prev) {
						fprintf(stderr, "%*s\n",
								3 * temp->depth + (int) strlen(temp->path),
								temp->path);
						fflush(stderr);
					}
				}
				clear_node(next);
				free(full_path);
				continue;
			}
			/* push the next node to the stack on the top of current node*/
			stack_push(stk, current, next);
			/* recursively search */

			walk_to_next(next);
		}
		free(full_path);
		fflush(stderr);
	}
	stack_job_done(stk, current);
	/* now list the dir */
	return 0;
}

/*
 * wrapper function for directory search
 */
void* search_dir(void *para) {
	Node* temp;
	long err = 0;
	temp = (Node*) para;
	/*
	 * I have to remember the lock & stk here, since once the node get free
	 * I will have no chance to come back, a more elegant way may exist
	 */
	stack *stk=temp->stk;
	pthread_rwlock_t *lock=&temp->stk->s_lock;

	if ((err = pthread_rwlock_wrlock(lock)) != 0) {
		if (!no_err_msg)
			fprintf(stderr, "rwlock_wrlock: %s\n", strerror(err));
	}
	/*
	 * increment the alive thread counts
	 */
	stk->thread_counts++;

	if ((err = pthread_rwlock_unlock(lock)) != 0) {
		if (!no_err_msg)
			fprintf(stderr, "rwlock_unlock: %s\n", strerror(err));
	}

	err = walk_recur((Node*) para);
	  //	dbg(__func__,temp->path,__LINE__);
	if ((err = pthread_rwlock_wrlock(lock)) != 0) {
		if (!no_err_msg)
			fprintf(stderr, "rwlock_wrlock: %s\n", strerror(err));
	}
	/*decrement the thread counts if detached*/
	  stk->thread_counts--;

	  if ((err = pthread_rwlock_unlock(lock)) != 0) {
	    if (!no_err_msg)
	      fprintf(stderr, "rwlock_unlock: %s\n", strerror(err));
	}
	fflush(stderr);
	pthread_exit((void*) err);
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
	/*if the thread number limit is 0, use main thread*/
	if (thread_limits == 0) {
		walk_recur(next);
		return 0;
	}
	/*use wrlock to exclude other thread create thread
	 * because it will check the alive thread number
	 */
	pthread_rwlock_wrlock(&next->stk->s_lock);
	err = 0;
	/* the threads number is enough, cannot create a thread*/
	if (next->stk->thread_counts >= thread_limits && thread_limits >= 0)
		err = 1;
	else {
		/*
		 * spawn a thread, if cannot create a thread, issue
		 * an error message and let the parent thread do the "left" recursion
		 * however, when it is available, always use a new thread
		 */
		err = pthread_create(&id, &next->stk->attr, search_dir, (void*) next);
		if (err != 0 && !no_err_msg)
			fprintf(stderr, "pthread_create: %s\n", strerror(err));
	}
	pthread_rwlock_unlock(&next->stk->s_lock);
	/* if the thread creation fails, use parent thread*/
	if (err != 0)
		err = walk_recur(next);
	return err;
}
stack Stack;

int main(void) {

  //	struct stat info;
	stack_init(&Stack);
	Node *current;
	//	lstat(argv[1], &info);
	//if (S_ISLNK(info.st_mode))
	//	printf("This is symlink\n");
	current = make_node(".", 1, &Stack);
	if(current==NULL)
	  dbg(__func__,"make node",__LINE__);
	stack_push(&Stack, current, NULL );
	walk_to_next(current);
	//walk_recur(1,&st,current);
	//stack_destroy(&Stack);
	//sleep(5);
	pthread_exit(0);
}
