/*
	random -- random number generation library routines


	This source file may be made available to others, so long as
	Geotronics Corporation (Austin, Texas) is given proper credit.


				REVISION HISTORY

09-May-1981	D A Gwyn	Created.


Usage:
	% cc program.c -lGC -lm		if "-lGC" contains this package.

Notes:
	Don't forget to include extern declarations for these routines.
	These routines use the UNIX "rand()" routine; you may want to
	invoke "srand()" to obtain unpredictability before using these.
*/

#include	<errno.h>
#include	<math.h>

extern int	errno ;
/*
	RED - returns random exponential deviate with mean value 1.0

Example:
	red = mu * RED();		// has mean value `mu'

Method:
	Logarithm method; see Knuth 3.4.1D(1)
*/

double	RED()
	{
	extern double	Random();	// returns 0.0 to 1.0-epsilon
	double		temp ;

	do
		temp = Random();
		while ( temp <= 0.0 );
	return ( - log( temp ) );
	}
/*
	RND - returns random normal deviate with mean 0.0 and s.d. 1.0

Example:
	rnd = mu + sigma * RND();	// has mean `mu' & s.d. `sigma'

Method:
	Polar method; see Knuth 3.4.1C(1)
*/

static int	rndflag = 0 ;		// flip-flop for saved value
static double	rndlast = 0.0 ;		// saved value if rndflag==1

double	RND()
	{
	extern double	Random();	// returns 0.0 to 1.0-epsilon
	double		s , x , y ;

	if ( rndflag )
		{
		rndflag = 0 ;
		return ( rndlast );	// already on hand
		}
	++ rndflag ;

	// generate a pair of numbers; save one for next time
	do	{		// loop executed 1.27 +_ 0.587 times
		x = 2.0 * Random() - 1.0 ;	// uniform from -1 to 1
		y = 2.0 * Random() - 1.0 ;
		s = x * x + y * y ;
		} while ( s >= 1.0 || s <= 0.0 );
	rndlast = sqrt( -2.0 * log( s ) / s );
	x *= rndlast ;
	rndlast *= y ;			// save for next call
	return ( x );
	}
/*
	RPD - returns random Poisson deviate with specified mean
		(returns -1 if specified mean < 0.0)

Example:
	rpd = RPD( mu );		// has mean value `mu'

Method:
	See Knuth 3.4.1F(3)
*/

#define	MAXINT	32767

int	RPD( mu )
	double		mu ;		// average value (>= 0.0)
	{
	extern double	Random();	// returns 0.0 to 1.0-epsilon
	register int	val ;
	double		p , q ;

	if ( mu < 0.0 )
		{
		errno = EDOM ;
		return( -1 );
		}
	p = exp( - mu );

	q = 1.0 ;
	for ( val = 0 ; val < MAXINT ; ++ val )
		if ( (q *= Random()) < p )
			break ;
	return ( val );
	}
/*
	RandFlt - returns random floating-point number in specified range
		(uniformly distributed)

Example:
	val = RandFlt( min , max );	// ranges from min to max-epsilon
*/

double	RandFlt( min , max )
	double		min , max ;	// range desired
	{
	extern double	Random();	// returns 0.0 to 1.0-epsilon

	return ( min + (max - min) * Random() );
	}
/*
	RandInt - returns random integer in specified range
		(uniformly distributed)

Example:
	deal = card[RandInt( 0 , 51 )];	// selects one of 52 cards
*/

int	RandInt( min , max )
	register int	min , max ;	// (inclusive) range desired
	{
	extern double	Random();	// returns 0.0 to 1.0-epsilon
	register int	temp ;

	temp = ( ++ max - min ) * Random();	// truncated
	return ( min + temp );
	}
/*
	Random - returns random floating-point number from 0.0 to 1.0-
		 (0.0 included; 1.0 excluded)	(uniformly distributed)

	 UNIX version (uses 7th edition C library "rand()" routine)

Example:
	r = mn + (mx - mn) * Random();	// maps into range (mn,mx)
*/

double	Random()
	{
	return ( rand() / 32768.0 );
	}
