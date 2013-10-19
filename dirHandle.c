/*
 * dirHandle.c
 * For CS820 Assignment 2
 * Created on: Oct 17, 2013
 * Author: Yanhua Liu ytz2
 */


#include "dirHandle.h"


void dbg(const char* func, const char* path, int count)
{
	printf("%s: %s is %d \n",func,path,count);
}
/*
 * make a history stack node
 */
Node* make_node(char *dname)
{
	Node* temp;
	DIR *dir;
	char* rptr,*memptr;

	errno=0;
	if((dir=opendir(dname))==NULL)
	{
		perror("Opendir");
		return NULL;
	}
	if ((rptr = get_realpath(dname,&memptr))==NULL)
	{
		perror("realpath");
		if (memptr)
			free(memptr);
		return NULL;
	}

	if ((temp=(Node*)malloc(sizeof(Node)))==NULL)
	{
		perror("malloc()");
		return NULL;
	}
	temp->counter=1;
	temp->dir=dir;
	temp->path=memptr;
	temp->prev=NULL;
	dbg(__func__,temp->path,temp->counter);
	return temp;
}


/*/*
 * Initialize a stack
 */
int stack_init(stack *st)
{
	int err;
	st->head=NULL;
	err=pthread_rwlock_init(&st->s_lock,NULL);
	if (err!=0)
		return(err);
	return 0;
}

/*
 * push a node on the "top" of the tree-like stack
 */
int stack_push(stack *st,Node *current, Node *next)
{
	int err;
	if ((err=pthread_rwlock_wrlock(&st->s_lock))!=0)
	{
		fprintf(stderr, "rwlock_wrlock: %s", strerror(err));
		return err;
	}

	/* when the head is null set the current as head
	 * this handles the call stack_push(st,current,NULL)
	 * */
	if (st->head==NULL)
		st->head=current;
	else
	/* if one subdirectory added, the current counter increase*/
	/* link the next to the current node*/
	{
		current->counter++;
		next->prev=current;
	}


	if ((err=pthread_rwlock_unlock(&st->s_lock))!=0)
	{
		fprintf(stderr, "rwlock_unlock: %s", strerror(err));
		return err;
	}

	if (next)
	{
	  dbg(__func__,next->path,next->counter);
	  printf("\t\t\t%s is changed: %lu\n",current->path,current->counter);
	}

	return 0;
}

/*
 * pop a node on the "top" of the tree-like stack
 */
int stack_job_done(stack *st,Node *current)
{
	int err;
	Node *temp,*tofree;
	temp=current;
	if ((err=pthread_rwlock_wrlock(&st->s_lock))!=0)
	{
		fprintf(stderr, "rwlock_wrlock: %s", strerror(err));
		return err;
	}
	// decrement the leaf counter

	/*
	 * from the current node to loop, if the counter of
	 * the current node is 0, clean the node and continue
	 * to check, else just stop
	 */
	while (temp!=NULL)
	{
		temp->counter--;
		if (temp->counter==0)
		{
			// closedir
			if (temp->dir)
				closedir(temp->dir);
			// clean the memory
			if (temp->path)
				free(temp->path);
			tofree=temp;

			dbg(__func__,temp->path,temp->counter);
			temp=temp->prev;
			free(tofree); // delete the node

		}
		else
			break;
	}


	if ((err=pthread_rwlock_unlock(&st->s_lock))!=0)
	{
		fprintf(stderr, "rwlock_unlock: %s", strerror(err));
		return err;
	}
	return 0;
}

/*
 *  find the realpath which used to appear in the tree
 */
Node* stack_find_history(stack *st,Node* current,char *path)
{
	int err;
	Node *temp;
	if ((err=pthread_rwlock_rdlock(&st->s_lock))!=0)
	{
		fprintf(stderr, "rwlock_rdlock: %s", strerror(err));
		return NULL;
	}

	for (temp=current;temp!=NULL;temp=temp->prev)
	{
		if(strcmp(temp->path,path)==0)
			break;
	}

	if ((err=pthread_rwlock_unlock(&st->s_lock))!=0)
	{
		fprintf(stderr, "rwlock_unlock: %s", strerror(err));
		return NULL;
	}
	return temp;
}

/*
 * recursively walk the directory
 */

int walk_recur(int depth,stack *stk,Node* current)
{

	if (depth>3)
		return 0;
	DIR *dir;
	char *current_path,*full_path,*fname;
	struct dirent entry;
	struct dirent *result;
	struct stat st;
	int err,len;
	Node *next;
	Node *prev;

	/* get the current directory info*/
	dir=current->dir;
	current_path=current->path;

	/* now loop directory */
	for (err = readdir_r(dir, &entry, &result);
	         result != NULL && err == 0;
	         err = readdir_r(dir, &entry, &result))
	{

		if (!strcmp(result->d_name, ".") || !strcmp(result->d_name, ".."))
			continue;
		/*
		 * get the full name of the subdir or files
		 */
		if (result->d_name[0]=='.')
			continue;
		full_path=path_alloc(NULL);
		len=strlen(current_path);
		strcpy(full_path,current_path);
		full_path[len++]='/';
		full_path[len]='\0';
		fname=result->d_name;
		strcat(full_path,fname);
		if (stat(full_path,&st)==-1)
		{
			perror(full_path);
			free(full_path);
			continue;
		}
		if (S_ISDIR(st.st_mode))
		{
			if ((next=make_node(full_path))==NULL)
			{
				free(full_path);
				continue;
			}
			else if ((prev=stack_find_history(stk,current,next->path))!=NULL)
			{
				closedir(next->dir);
				free(next->path);
				free(next);
				free(full_path);
				continue;
			}
			stack_push(stk,current, next);
			walk_recur(depth+1, stk, next);
		}
		free(full_path);
	}
	stack_job_done(stk,current);
	/* now list the dir */
	return 0;
}

int main(int argc, char *argv[])
{
	stack st;
	stack_init(&st);
	Node *current;
	current=make_node(argv[1]);
	stack_push(&st,current, NULL);
	walk_recur(0,&st,current);
	return 0;
}




