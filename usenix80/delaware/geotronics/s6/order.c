#
/*
 * order.c -	Order library routines so that only one pass
 *		of the library will be required by the linker.
 *
 * last edit: 10-Jun-1980  D A Gwyn
 *
 * input:	Each line contains an entry-point name (Def) followed by
 *		a list of calls (Refs).  A Def may occur more than once;
 *		the Refs from all occurrences are merged.  Do not repeat
 *		a Ref name for the same Def, however.
 *
 * output:	One entry-point name per line, ordered so that all
 *		references are to entries later in the list.
 */


/*								      *
 *			data structures				      *
 *								      */

#define	MAXCHARS	1500		/* maximum chars in strings */
#define	MAXNAME		15		/* maximum chars in Def/Ref */
#define	MAXNODES	1500		/* maximum total Refs */
#define	MAXSYMBS	250		/* maximum number of symbols */
#define	NBUCKETS	128		/* number of hash buckets */

struct	syment  {			/* symbol table entry */
	struct syment	*hshlink;	/* -> next in hash chain */
	char		*symb;		/* -> symbol string */
	int		ncalls;		/* references to this entry */
	int		ref;		/* -> Ref chain
	struct node	*ref;	actually (compiler restriction) */
};

struct	node  {				/* Ref chain node */
	struct syment	*name;		/* -> symtab entry */
	struct node	*reflink;	/* -> next Ref in chain */
};

struct node	vertex[MAXNODES];	/* Ref chain node pool */
struct node	*verfree  &vertex[0];	/* -> next avail node */

struct syment	*bucket[NBUCKETS];	/* hash buckets */

struct syment	symtab[MAXSYMBS];	/* symbol table */
struct syment	*symfree  &symtab[0];	/* -> next avail symtab slot */

char	sspace[MAXCHARS];		/* string storage space */
char	*strfree  &sspace[0];		/* -> next avail sspace slot */

char	endline;			/* end-of-line flag */


/*								      *
 *			data structure routines			      *
 *								      */

/*
 * main -	Main program.
 */
main( argc, argv )
int	argc;
char	*argv[];
{
	register	arg;

	arg = 1;
	do  {
		if ( argc > 1 )  {	/* have arguments */
			close( 0 );
			open( argv[arg], 0 );	/* fd = 0 */
		}
		Input();		/* build linked structure */
	} while ( ++arg < argc );

	Output();			/* tear down structure again */
}


/*
 * Input -	Input Def/Ref information and build adjacency lists.
 */
Input()
{
	char			inpnam[MAXNAME];
	int			len;
	register struct syment	*dp, *rp;
	register struct node	*np;
	struct syment		*LookUp();

	while ( (len = GetName( inpnam, MAXNAME )) >= 0 )  {
		if ( len == 0 )
			continue;	/* skip blank lines */

		dp = LookUp( inpnam );	/* find Def entry */

		/* get Refs and link into Ref node chain */

		while ( GetName( inpnam, MAXNAME ) > 0 )  {
			rp = LookUp( inpnam );	/* find entry */
			rp->ncalls++;	/* tally reference */

			np = verfree++;	/* allocate Ref chain node */
			if ( np == &vertex[MAXNODES] )  {
				write( 2, "* Node pool overflow *\n",
								23 );
				exit();
			}
			np->name = rp;	/* -> symtab entry */
			np->reflink = dp->ref;
			dp->ref = np;	/* link node into Ref chain */
		}
	}
}


/*
 * Output -	Write out vertices in topological order.
 */
Output()
{
	register struct syment	*dp, *rp;
	register struct node	*np;
	struct syment		*sp, *top;

	/* create linked stack of entries with no callers */

	for ( top = 0,  dp = &symtab[0];  dp < symfree;  dp++ )
		if ( dp->ncalls == 0 )  {
			dp->hshlink = top;	/* now a stack link */
			top = dp;
		}

	/* sp keeps count of entries written */

	for ( sp = &symtab[0];  sp < symfree;  sp++ )  {
		if ( top == 0 )  {
			write( 2, "* Self-referential system *\n", 28 );
			exit();
		}

		dp = top;		/* top of 0-ref-count stack */
		PutName( dp->symb );	/* write entry name */
		top = dp->hshlink;	/* top of remaining stack */

		/* decrease reference count of called entries */

		for ( np = dp->ref;  np;  np = np->reflink )  {
			rp = np->name;	/* -> symtab entry */
			if ( --rp->ncalls == 0 )  {
				rp->hshlink = top;
				top = rp;	/* add to stack */
			}
		}
	}
}


/*								      *
 *			i/o support routines			      *
 *								      */


/*
 * GetName -	Get "word" from input.
 */
GetName( string, size )
char	*string;
int	size;
{
	register	c, n;

	if ( endline )  {
		endline = 0;
		return ( 0 );		/* EOL */
	}

	while ( (c = GetC()) == ' ' || c == '\t' )
		;			/* skip leading whitespace */

	if ( c < 0 )
		return ( -1 );		/* EOF */

	for ( n = 0;  c > 0 && c != ' ' && c != '\t' && c != '\n';
							c = GetC() )
		if ( ++n < size )
			*string++ = c;

	if ( (c < 0 || c == '\n') && n > 0 )
		endline++;		/* EOL next */
	*string = 0;			/* EOS */
	return ( n );
}


/*
 * GetC -	Get character from input.
 */
GetC()
{
	char	c;

	if ( read( 0, &c, 1 ) <= 0 )
		c = -1;			/* end of file */

	return ( c );
}


/*
 * PutName -	Write entry name and newline.
 */
PutName( string )
char	*string;
{
	char	c;

	while ( c = *string++ )
		write( 1, &c, 1 );

	write( 1, "\n", 1 );
}


/*								      *
 *			symbol table routines			      *
 *								      */

/*
 * Hash -	Compute hash function on symbol string.
 */

Hash( symbol )
char	*symbol;
{
	register	sum;

	for ( sum = 0;  *symbol;  sum =+ *symbol++ )
		;

	return ( sum % NBUCKETS );
}


/*
 * StrCmp -	Compare two character strings.
 */

StrCmp( string1, string2 )
char	*string1, *string2;
{
	register	diff;

	for ( ;  (diff = *string1 - *string2) == 0;  string1++,
						     string2++ )
		if ( *string1 == 0 )
			return ( 0 );	/* equal */

	return ( diff );		/* differ */
}


/*
 * LookUp -	Find entry in table; insert if not already there.
 */

struct syment	*LookUp( symbol )
char	*symbol;
{
	register struct syment	**p1, *p2;

	p1 = &bucket[Hash( symbol )];	/* -> bucket slot */

	for ( p2 = *p1;  p2;  p1 = &p2->hshlink,  p2 = *p1 )
		if ( StrCmp( symbol, p2->symb ) == 0 )
			return ( p2 );	/* already in table */

	/* new symtab entry */

	p2 = symfree;			/* allocate next slot */
	if ( p2 == &symtab[MAXSYMBS] )  {
		write( 2, "* Symbol table overflow *\n" , 26);
		exit();
	}
	*p1 = symfree++;		/* -> entry */

	p2->symb = strfree;		/* -> string storage */
	while ( *strfree++ = *symbol++ )/* store symbol string */
		if ( strfree == &sspace[MAXCHARS] )  {
			write( 2, "* String space overflow *\n" , 26);
			exit();
		}
	return ( p2 );
}
