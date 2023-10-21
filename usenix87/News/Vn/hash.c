/*
** vn news reader.
**
** hash.c - hash table routines
**
** see copyright disclaimer / history in vn.c source file
*/

#include <stdio.h>
#include "config.h"
#include "tune.h"
#include "vn.h"

/*
** hash table manipulation routines:
**	also sets Ncount, allocates Newsorsder array, and sets Newsorder
**	initially to order newsgroups were entered in (active file order)
*/

extern int Ncount;
extern NODE **Newsorder;

static NODE *Tab [HASHSIZE];	/* hash Table */

hashinit ()
{
	int i;
	for (i=0; i < HASHSIZE; ++i)
		Tab[i] = NULL;
	Ncount = 0;
}

/*
	enter new node (name s, articles n, low l) in hash Table, 
	initial flags = 0.  As nodes are entered, pnum item is temporarily
	used to indiacte entry order for initial construction of Newsorder
	array via entry_order();
*/
NODE *hashenter(s,n,l)
char *s;
int n;
int l;
{
	char *str_store();
	NODE *ptr,*node_store();
	int i;

	i=hash(s);
	ptr = node_store();
	ptr->next = Tab[i];
	Tab[i] = ptr;
	if (l > n)
		l = n;
	ptr->pnum = Ncount;
	++Ncount;
	ptr->rdnum = l;
	ptr->state = 0;
	ptr->art = n;
	ptr->nd_name = str_store(s);
	return (ptr);
}

NODE *hashfind(s)
char *s;
{
	NODE *ptr;

	for (ptr = Tab[hash(s)]; ptr != NULL && strcmp(ptr->nd_name,s) != 0;
					ptr = ptr->next)
		    ;
	return (ptr);
}

/*
** entry order is called after all hash_enter's have been done, PRIOR
** to the use of pnum item for anything else.  It constructs the initial
** Newsorder array.
*/
entry_order()
{
	int i;
	NODE *ptr;

	if ((Newsorder = (NODE **) malloc(Ncount*sizeof(NODE *))) == NULL)
		printex("Cannot allocate memory for Newsorder array");
	for (i=0; i < HASHSIZE; ++i)
	{
		for (ptr = Tab[i]; ptr != NULL; ptr = ptr->next)
			Newsorder[ptr->pnum] = ptr;
	}
}

static hash (s)
char *s;
{
	int rem;
	for (rem=0; *s != '\0'; ++s)
		rem = (rem*128 + (*s&0x7f)) % HASHSIZE;
	return (rem);
}
