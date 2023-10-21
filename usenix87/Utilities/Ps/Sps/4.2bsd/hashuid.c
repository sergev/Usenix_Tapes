# include       "sps.h"

/* The hashing functions themselves ... */
# define        HASHFN1( a )            (((a)*91 + 17) % MAXUSERID)
# define        HASHFN2( a )            (((a) + 47) % MAXUSERID)

/*
** HASHUID - Returns a pointer to a slot in the hash table that corresponds
** to the hash table entry for `uid'. It returns a null pointer if there is
** no such slot.
*/
struct hashtab  *hashuid ( uid )

int                             uid ;

{
	register struct hashtab *hp ;
	register int            i ;
	register int            j ;
	extern struct info      Info ;

	j = HASHFN1( uid ) ;
	for ( i = 0 ; i < MAXUSERID ; i++ )
	{
		hp = &Info.i_hnames[ j ] ;
		if ( !hp->h_uname[0] )
			return ( (struct hashtab*)0 ) ;
		if ( hp->h_uid == uid )
			return ( hp ) ;
		j = HASHFN2( j ) ;
	}
	return ( (struct hashtab*)0 ) ;
}

/*
** HASHNEXT - Returns a pointer to the next slot in the hash table that
** may be use for storing information for `uid'. It returns a null pointer
** if there are no more free slots available.
*/
struct hashtab  *hashnext ( uid )

int                             uid ;

{
	register struct hashtab *hp ;
	register int            i ;
	register int            j ;
	extern struct info      Info ;

	j = HASHFN1( uid ) ;
	for ( i = 0 ; i < MAXUSERID ; i++ )
	{
		hp = &Info.i_hnames[ j ] ;
		if ( !hp->h_uname[0] )
			return ( hp ) ;
		j = HASHFN2( j ) ;
	}
	return ( (struct hashtab*)0 ) ;
}
