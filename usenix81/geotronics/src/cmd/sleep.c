/*
	sleep -- suspend execution for an interval

	Usage:	sleep time
*/

#include	<ctype.h>
#include	<stdio.h>


main( argc , argv )
	int	argc ;
	char	*argv[] ;
	{
	register char	c , *s ;
	unsigned	n ;

	if ( argc < 2 )
		Error( "missing time arg" );

	for ( n = 0 , s = argv[1] ; c = *s++ ; )
		{
		if ( ! isdigit( c ) )
			Error( "non-numeric arg" );
		n = 10 * n + (c - `0') ;
		}

	sleep( n );
	}


static
Error( msg )
	char	*msg ;
	{
	fputs( "\07* sleep * " , stderr );
	fputs( msg , stderr );
	putc( `\n' , stderr );

	exit( 1 );
	}
