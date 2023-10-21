/*
	rtest -- test random-number library routines

	last edit: 03-Jan-1981	D A Gwyn
*/

#include	<stdio.h>

extern double	RED(), RND(), RandFlt();
extern int	RPD(), RandInt();

#define	TRIALS	400

#define	MAX	40
static int	count[MAX];		// "bins" to tally "hits"

main()	{
	register int	i , k ;

	/* test random exponential deviates */
	printf( "\n\nRandom exponential deviates:\n\n" );
	for ( i = 0 ; i < MAX ; ++ i )
		count[i] = 0 ;		// clear counters
	for ( k = 0 ; k < TRIALS ; ++ k )
		{
		i = 10.0 * RED();	// truncated
		if ( i < 0 )
			i = 0 ;
		else if ( i >= MAX )
			i = MAX - 1 ;
		++ count[i];		// tally hit
		}
	Histo();			// display histogram

	/* test random normal deviates */
	printf( "\f\n\nRandom normal deviates:\n\n" );
	for ( i = 0 ; i < MAX ; ++ i )
		count[i] = 0 ;		// clear counters
	for ( k = 0 ; k < TRIALS ; ++ k )
		{
		i = 20.0 + 5.0 * RND();	// truncated
		if ( i < 0 )
			i = 0 ;
		else if ( i >= MAX )
			i = MAX - 1 ;
		++ count[i];		// tally hit
		}
	Histo();			// display histogram

	/* test random Poisson deviates */
	printf( "\f\n\nRandom Poisson deviates:\n\n" );
	for ( i = 0 ; i < MAX ; ++ i )
		count[i] = 0 ;		// clear counters
	for ( k = 0 ; k < TRIALS ; ++ k )
		{
		i = RPD( 10.0 );	// truncated
		if ( i < 0 )
			i = 0 ;
		else if ( i >= MAX )
			i = MAX - 1 ;
		++ count[i];		// tally hit
		}
	Histo();			// display histogram

	/* test random uniform floating-point deviates */
	printf( "\f\n\nRandom uniform floating-point deviates:\n\n" );
	for ( i = 0 ; i < MAX ; ++ i )
		count[i] = 0 ;		// clear counters
	for ( k = 0 ; k < TRIALS ; ++ k )
		{
		i = RandFlt( 10.0 , 30.0 );	// truncated
		if ( i < 0 )
			i = 0 ;
		else if ( i >= MAX )
			i = MAX - 1 ;
		++ count[i];		// tally hit
		}
	Histo();			// display histogram

	/* test random uniform integer deviates */
	printf( "\f\n\nRandom uniform integer deviates:\n\n" );
	for ( i = 0 ; i < MAX ; ++ i )
		count[i] = 0 ;		// clear counters
	for ( k = 0 ; k < TRIALS ; ++ k )
		{
		i = RandInt( 10 , 30 );
		if ( i < 0 )
			i = 0 ;
		else if ( i >= MAX )
			i = MAX - 1 ;
		++ count[i];		// tally hit
		}
	Histo();			// display histogram
	}
/*
	Histo - plot histogram of hit counts
*/

Histo()	{
	register int	i , j ;

	printf( "# ....5...10...15...20...25...30...35...40\n" );
	for ( i = 0 ; i < MAX ; ++ i )
		{
		printf( "%2d" , i );
		for ( j = 0 ; j < count[i] ; ++ j )
			putchar( `*' );
		putchar( `\n' );
		}
	}
