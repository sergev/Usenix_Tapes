/*
	mulplot -- combines two 8.5x11 inch plots into 8.5x22 inches

	last edit: 25-Feb-1981	D A Gwyn

Usage:
	% mulplot[[ bot_plot] top_plot]

		There is always a top (shifted) plot;
		if one argument, stdin will be used for bot_plot.

		If no arguments, stdin will be used for top_plot and
		there will be no bottom (unshifted) plot.

Example:
	% upl <pltxfr_file_1 >/usr/tmp/bp$$ ; \
	  upl <file_2 | mulplot /usr/tmp/bp$$ | hi ; rm /usr/tmp/bp$$

		plots pltxfr_file_1 above file_2 on H.I. plotter
*/

#include	<stdio.h>

main( argc , argv )
	register int	argc ;
	register char	*argv[];
	{
	if ( --argc > 2 )
		{
		fprintf( stderr ,
		"\07Usage: mulplot[ bot_plot][ top_plot]\n" );
		exit( 1 );
		}

	if ( argc > 0 )			// there is a bottom plot
		{
		if ( argc == 2 &&	// bot_plot given as argument
		     freopen( *++argv , "r" , stdin ) == NULL )
			{
			perror( *argv );
			exit( 1 );
			}
		filter( 0 );		// unshifted
		putchar( `U' );		// penup between plots
		}

	// there is always a top plot

	if ( argc > 0 &&		// top_plot given as argument
	     freopen( *++argv , "r" , stdin ) == NULL )
		{		
		perror( *argv );
		exit( 1 );
		}
	filter( 1 );			// top plot

	exit( 0 );
	}

static struct	{
		unsigned	x ;
		unsigned	y ;
		}	coords ;	// `M' coordinates

static
filter( shift )				/* mostly file copy */
	register int		shift ;	// nonzero => shift by 11 inches
	{
	register char		code ;	// TIG graphic code
	register unsigned	offset = 11000 ;	// 11 inches

	while ( (code = getchar()) != EOF )
		switch ( code )
			{
		case `F':		// newframe
			continue ;	// (stripped)

		case `U':		// penup
		case `D':		// pendown
			putchar( code );
			continue ;

		case `M':		// move
			putchar( code );
			fread( &coords , sizeof coords , 1 , stdin );
			if ( shift )
				coords.y += offset ;
			fwrite( &coords , sizeof coords , 1 , stdout );
			continue ;

		default:
			fprintf( stderr , "\07* mulplot * bad input\n" );
			exit( 1 );
			}
	}
