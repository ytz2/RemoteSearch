/*
 * dirHandle.c
 * For CS820 Assignment 2
 * Created on: Oct 17, 2013
 * Author: Yanhua Liu ytz2
 */


#include "dirHandle.h"


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
	return 0;
}

/*
 * pop a node on the "top" of the tree-like stack
 */
int stack_job_done(stack *st,Node *current)
{
	int err;
	Node *temp,tofree;
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
			free(tofree); // delete the node
			temp=temp->prev;
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
int walk_recur(char* dname, int depth)
{

}

