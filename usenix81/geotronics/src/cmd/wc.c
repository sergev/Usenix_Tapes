/*
	wc -- word count (compatible with 7th Edition UNIX)

	last edit:	22-Jun-1981	D A Gwyn
*/

#include <stdio.h>

#define	NO	0
#define	YES	1

main( argc , argv )
	int	argc ;
	char	*argv[] ;
	{
	static		lflag = NO , wflag = NO , cflag = NO ;
	register int	c , inword ;
	int		errs = 0 , files , several ;
	register FILE	*fp = stdin ;
	long		linect , wordct , charct ;
	long		tlinect = 0 , twordct = 0 , tcharct = 0 ;

	if ( --argc > 0 && argv[1][0] == `-' )
		for ( -- argc , ++ argv , c = 1 ; (*argv)[c] ; ++ c )
			switch ( *argv[c] )
				{
			case `l':
				lflag = YES ;
				break ;
			case `w':
				wflag = YES ;
				break ;
			case `c':
				cflag = YES ;
				break ;
				}

	if ( ! lflag && ! wflag && ! cflag )
		lflag = wflag = cflag = YES ;	/* default */

	files = argc > 0 ;
	several = argc > 1 ;

	do	{
		if ( files && freopen( *++argv , "r" , stdin ) == NULL )
			{
			fprintf( stderr , "\07* wc * can't open %s\n",
				 *argv );
			++ errs ;
			continue ;
			}

		linect = wordct = charct = 0 ;
		inword = NO ;

		while ( (c = getchar()) != EOF )
			{
			++ charct ;

			if ( c == `\n' )
				++ linect ;

			if ( c == ` ' || c == `\t' || c == `\n' )
				inword = NO ;
			else if ( ! inword )
				{
				inword = YES ;
				++ wordct ;
				}
			}

		inword = NO ;			/* for clean spacing */
		if ( lflag )
			{
			inword = YES ;
			printf( "%7D" , linect );
			}
		if ( wflag )
			{
			if ( inword )
				putchar( ` ' );
			inword = YES ;
			printf( "%7D" , wordct );
			}
		if ( cflag )
			{
			if ( inword )
				putchar( ` ' );
			printf( "%7D" , charct );
			}
		if ( files )
			printf( " %s" , *argv );
		putchar( `\n' );

		tlinect += linect ;
		twordct += wordct ;
		tcharct += charct ;

		} while ( --argc > 0 ) ;

	if ( several )
		printf( "%7D %7D %7D total\n" ,
			tlinect , twordct , tcharct );
	exit( errs );
	}
