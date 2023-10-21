#include <stdio.h>
#include "config.h"
#include "vn.h"

/* hash table manipulation routines */

static NODE *Tab [HASHSIZE];	/* hash Table */

hashinit ()
{
	int i;
	for (i=0; i < HASHSIZE; ++i)
		Tab[i] = NULL;
}

/*
	enter new node (name s, articles n, low l) in hash Table, 
	initial flags = 0.
*/
NODE *hashenter(s,n,l)
char *s;
int n;
int l;
{
	static int tcount=0;
	char *str_store();
	NODE *node_store();
	int i;

	if (tcount >= HASHMAX)
	{
		fgprintf("too many newsgroups - can't enter %s",s);
		return(NULL);
	}
	for (i=hash(s); Tab[i] != NULL; i = (++i) % HASHSIZE)
		;
	Tab[i] = node_store();
	if (l > n)
		l = n;
	(Tab[i])->rdnum = l;
	(Tab[i])->state = 0;
	(Tab[i])->art = n;
	(Tab[i])->nd_name = str_store(s);
	++tcount;
	return (Tab[i]);
}

/*
	find hash Table entry s, return node with nd_name NULL for insertion
	if non-existant
*/
NODE *hashfind(s)
char *s;
{
	int idx;
	for (idx = hash(s); Tab[idx] != NULL && strcmp((Tab[idx])->nd_name,s) != 0;
					idx = (++idx) % HASHSIZE)
		    ;
	return (Tab[idx]);
}

static hash (s)
char *s;
{
	int rem;
	for (rem=0; *s != '\0'; ++s)
		rem = (rem*128 + (*s&0x7f)) % HASHSIZE;
	return (rem);
}
