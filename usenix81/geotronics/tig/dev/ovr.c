/*
	ovr -- combines multiple TIG plots into one overlaid plot

	last edit: 21-Apr-1981	D A Gwyn

Usage:
	% ovr[ plot_files] > combined_plot

		If fewer than two arguments, stdin will also be used.
*/

#include	<stdio.h>

main( argc , argv )
	register int	argc ;
	register char	*argv[];
	{
	register int	ns = argc > 2 ;	// "no stdin" flag

	if ( ns )
		-- argc ;		// else "phantom" argument
	while ( --argc >= 0 )
		{
		if ( ns && freopen( *++argv , "r" , stdin ) == NULL )
			{
			perror( *argv );
			exit( 1 );
			}
		++ ns ;			// do stdin at most once
		filter();		// process file
		}

	exit( 0 );
	}

static struct coord
	{
	unsigned	x ;
	unsigned	y ;
	}	coords ;		// `M' coordinates

static
filter()				/* mostly file copy */
	{
	register struct coord	*cp = &coords ;
	register int		code ;	// TIG graphic code

	for ( ; ; )
		switch ( code = getchar() )
			{
		case EOF:		// end of file
		case `F':		// newframe
			putchar( `U' );
			putchar( `M' );
			cp->x = cp->y = 0 ;
			fwrite( cp , sizeof coords , 1 , stdout );
			if ( code == EOF )
				return ;
			continue ;

		case `U':		// penup
		case `D':		// pendown
			putchar( code );
			continue ;

		case `M':		// move
			putchar( code );
			fread( cp , sizeof coords , 1 , stdin );
			fwrite( cp , sizeof coords , 1 , stdout );
			continue ;

		default:
			fputs( "\07* ovr * bad input\n" , stderr );
			exit( 1 );
			}
	}
