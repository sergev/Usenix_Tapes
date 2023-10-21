/*	SSORT.C		Works just like qsort() except that a shell
 *			sort, rather than a quick sort, is used. This
 *	is more efficient than quicksort for small numbers of elements
 *	and it's not recursive so will use much less stack space.
 *	This routine started out as the one in K&R.
 *
 *	12/13/85:  Modified to select a gap from the series
 *		   1, 4, 13, 40, 121 ... as per Knuth.
 *
 *	Copyright (C) 1985, Allen I. Holub.  All rights reserved
 */

void	ssort( base, nel, width, cmp )
char	*base;
int	nel, width;
int	(*cmp)();
{
	register int	i, j;
	int		gap, k, tmp ;
	char		*p1, *p2;

	for( gap=1; gap <= nel; gap = 3*gap + 1 )
		;

	for( gap /= 3;  gap > 0  ; gap /= 3 )
		for( i = gap; i < nel; i++ )
			for( j = i-gap; j >= 0 ; j -= gap )
			{
				p1 = base + ( j      * width);
				p2 = base + ((j+gap) * width);

				if( (*cmp)( p1, p2 ) <= 0 )
					break;

				for( k = width; --k >= 0 ;)
				{
					tmp   = *p1;
					*p1++ = *p2;
					*p2++ = tmp;
				}
			}
}

#ifdef DEBUG

cmp( cpp1, cpp2 )
char	**cpp1, **cpp2;
{
	register int	rval;
	printf("comparing %s to %s ", *cpp1, *cpp2 );

	rval = strcmp( *cpp1, *cpp2 );

	printf("returning %d\n", rval );
	return rval;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

main( argc, argv )
int	argc;
char	**argv;
{
	ssort( ++argv, --argc, sizeof(*argv), cmp );

	while( --argc >= 0 )
		printf("%s\n", *argv++ );
}

#endif

