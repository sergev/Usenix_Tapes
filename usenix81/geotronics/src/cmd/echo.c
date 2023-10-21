/*
	echo -- echo arguments (compatible with 7th Edition UNIX)

	last edit: 25-Jan-1981	D A Gwyn
*/

#include	<stdio.h>

extern int	strcmp();

static int	nflag = 0 ;		/* "-n" means no newline */


main( argc , argv )
	register int	argc ;
	register char	*argv[];
	{
	if ( strcmp( argv[1] , "-n" ) == 0 )
		{
		++ nflag ;
		-- argc , ++ argv ;
		}

	while ( --argc > 0 )
		{
		fputs( *++argv , stdout );
		if ( argc > 1 )
			putchar( ' ' );
		}

	if ( ! nflag )
		putchar( '\n' );

	exit( 0 );
	}
