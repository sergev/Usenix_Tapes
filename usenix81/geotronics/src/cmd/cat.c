/*
	cat -- catenate and print (compatible with 7th Edition UNIX)

	last edit: 25-Jan-1981	D A Gwyn
*/

#include	<stdio.h>

extern int	strcmp();

static int	errs = 0 ;		/* tallies errors */


main( argc , argv )
	register int	argc ;
	register char	*argv[];
	{
	register FILE	*fp ;

	if ( --argc == 0 )
		filecopy( stdin );
	else	{
		if ( strcmp( argv[1] , "-u" ) == 0 )
			{
			setbuf( stdout , NULL );
			-- argc , ++ argv ;
			}

		while ( --argc >= 0 )
			{
			if ( strcmp( *++argv , "-" ) == 0 )
				filecopy( stdin );
			else if ( (fp = fopen( *argv , "r" )) == NULL )
				{
				fputs( "\07* cat * " , stderr );
				perror( *argv );
				++ errs ;
				}
			else	{
				filecopy( fp );
				fclose( fp );
				}
			}
		}

	exit( errs );
	}


static
filecopy( fp )				/* copy file fp to stdout */
	register FILE	*fp ;
	{
	register int	c ;

	while ( (c = getc( fp )) != EOF )
		putchar( c );
	}
