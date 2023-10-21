/*
**		    Copyright (c) 1985	Ken Wellsch
**
**     Permission is hereby granted to all users to possess, use, copy,
**     distribute, and modify the programs and files in this package
**     provided it is not for direct commercial benefit and secondly,
**     that this notice and all modification information be kept and
**     maintained in the package.
**
*/

#include "adefs.h"

#ifdef CACHE
struct ctabstr
{
	short int  c_key ;
	short int  c_siz ;
	short int *c_cod ;
} ;

static int full = NO ;
static struct ctabstr ctab[CACHESIZE] ;
static short int cache[MAXCACHE] ;
static int cachesize = 0 ;
#endif CACHE
/*
**	rdcode -- fetch the code addressed by key.
**
**	Note: an open (chain) hashing strategy was tried
**	      on the cache table, with no significant speed
**	      increase! This seems puzzling, but since the
**	      hashing takes more memory, why bother with it?
**
*/

int rdcode (key)
  int key ;
{
	register int endb  ;

#ifdef CACHE
	if ( ( endb = GetCache (key) ) > 0 )	/* already in memory? */
		return (endb) ;
#endif CACHE

	if ( ( endb = readk (dbunit,key,codebuf,BUFSIZE*2) ) < 1 )
		return (ERROR) ;

	endb = (endb+1)/2 ;

#ifdef CACHE
	if ( full )
		return (endb) ;

	if ( ok2sav (key) == NO )
		return (endb) ;

	PutCache (key,endb) ;		/* lets keep a copy around */
#endif CACHE

	return (endb) ;
}

#ifdef CACHE
/*
**	ok2sav -- determine whether the code for the given key
**		  should be saved (if possible).
**		  Note: there is no need to save the INIT code,
**		  	but certainly saving the REPEAT code is
**			well worthwhile.
*/

static int ok2sav (key)
  int key ;
{
	register int val ;

	if ( key >= REPEAT && key < 1000 )	/* save `REPEAT' code */
		return (YES) ;

	switch ( class (key) )
	{

		case VERB :
		case OBJECT :
		case LABEL :
		case PLACE :
				val = YES ;
				break ;
		default :
				val = NO ;
				break ;
	}
	return (val) ;
}

static int PutCache (key,size)
  int key, size ;
{
	register int i ;

	if ( size <= 0 )
		return ;

	for ( i = 0 ; i < (CACHESIZE-1) ; i++ )
	{
		if ( ctab[i].c_key == key )
			return ;
		if ( ctab[i].c_key == -1 )
		{
			ctab[i+1].c_key = -1 ;
			break ;
		}
	}

	if ( i == (CACHESIZE-1) )	/* cache address table is full */
	{
		full = YES ;
		return ;
	}

	if ( (cachesize+size) >= MAXCACHE ) /* cache memory is (almost) full */
	{
		if ( (MAXCACHE-cachesize) < 50 ) /* less than 50 bytes left */
			full = YES ;
		return ;
	}

	ctab[i].c_key = key ;
	ctab[i].c_siz = size ;
	ctab[i].c_cod = &(cache[cachesize]) ;

	cachesize += size ;

	wcopy (codebuf,ctab[i].c_cod,size) ;

	return ;
}

static int GetCache (key)
  int key ;
{
	register int i, size ;

	for ( i = 0 ; i < CACHESIZE ; i++ )
	{
		if ( ctab[i].c_key == key )
			break ;
		if ( ctab[i].c_key == -1 )
			return (ERROR) ;
	}

	size = ctab[i].c_siz ;
	if ( size <= 0 )
		return (ERROR) ;
	wcopy (ctab[i].c_cod,codebuf,size) ;
	return (size) ;
}

int ClrCache ()
{
	register int i ;

	full = NO ;
	cachesize = 0 ;
	for ( i = 0 ; i < CACHESIZE ; i++ )
		ctab[i].c_key = -1 ;

	return ;
}

int wcopy (a,b,n)
  register short int *a, *b ;
  register int n ;
{
	while ( n-- )
		*b++ = *a++ ;
	return ;
}

#ifdef STATUS

int pcstat ()
{
	register int i ;

	printf ("Cache Status: (%s) -- (%d) of (%d) available\n\n",
		(full?"full":"active"),(MAXCACHE-cachesize),MAXCACHE) ;
	
	for ( i = 0 ; i < (CACHESIZE-1) ; )
	{
		if ( ctab[i].c_key == -1 )
			break ;
		printf ("\t[%d] key(%d) size(%d)\n",
			i,ctab[i].c_key,ctab[i].c_siz) ;
	}

	printf ("\n") ;
	return ;
}

#endif STATUS

#endif CACHE
