/*
	complx -- complex arithmetic library routines


	This source file may be made available to others, so long as
	Geotronics Corporation (Austin, Texas) is given proper credit.


				REVISION HISTORY

09-May-1981	D A Gwyn	Created.


Usage:
	% cc program.c -lGC -lm		if "-lGC" contains this package.

Notes:
	/usr/include/complex.h type-defines "complex" as a structure.
	The routines in this package require pointers to "complex"
	arguments; they return a "double" or a pointer to a "complex".

Example:
	#include	<complex.h>
	:
	complex a , b , c ;
	:
	CMul( &a , CConjug( CCopy( &b , &c ) ) );	// sets c = a b*
*/

#include	<complex.h>
#include	<math.h>
/*
	CConst - construct a complex from real and imaginary parts

	CConst( re , im , &c )	makes  c = re + i im  and returns  &c
*/

complex *CConst( re , im , cp )
	double			re , im ;
	register complex	*cp ;
	{
	cp->r = re ;
	cp->i = im ;
	return ( cp );
	}


/*
	To decompose a complex into real and imaginary parts:

	double	re , im ;
	complex c ;
	:
	re = c.r ;
	im = c.i ;
*/
/*
	CPhasor - construct a complex from amplitude and phase

	CPhasor( amp , phs , &c )	makes  c = amp exp(i phs)
					and returns  &c
*/

complex *CPhasor( amp , phs , cp )
	double			amp , phs ;
	register complex	*cp ;
	{
	cp->r = amp * cos( phs );
	cp->i = amp * sin( phs );
	return ( cp );
	}


/*
	Modulus - return modulus (magnitude, norm) of a complex

	Modulus( &c )	returns  |c|

	See also "cabs" in the standard 7th edition UNIX math library
*/

double	Modulus( cp )
	register complex	*cp ;
	{
	return ( hypot( cp->r , cp->i ) );
	}


/*
	Phase - return phase (angle, argument) of a complex

	Phase( &c )	returns  arg(c)  in radians (-Pi to +Pi)
*/

double	Phase( cp )
	register complex	*cp ;
	{
	return ( atan2( cp->i , cp->r ) );
	}
/*
	CCopy - copy one complex to another

	CCopy( &a , &b )	copies	a  to  b  and returns  &b
*/

complex *CCopy( ap , bp )
	register complex	*ap , *bp ;	// may coincide
	{
	bp->r = ap->r ;
	bp->i = ap->i ;
	return ( bp );
	}


/*
	CConjug - conjugate a complex

	CConjug( &c )	conjugates  c  and returns  &c
*/

complex *CConjug( cp )
	register complex	*cp ;
	{
	// real part unchanged
	cp->i = - cp->i ;
	return ( cp );
	}


/*
	CScale - multiply a complex by a scalar

	CScale( s , &c )	scales	c  by  s  and returns  &c
*/

complex *CScale( s , cp )
	double			s ;
	register complex	*cp ;
	{
	cp->r *= s ;
	cp->i *= s ;
	return ( cp );
	}


/*
	To negate a complex:

	CScale( -1.0 , &c )		(see above)
*/
/*
	CAdd - add one complex to another

	CAdd( &a , &b ) adds  a  to  b	and returns  &b
*/

complex *CAdd( ap , bp )
	register complex	*ap , *bp ;	// may coincide
	{
	bp->r += ap->r ;
	bp->i += ap->i ;
	return ( bp );
	}


/*
	CSub - subtract one complex from another

	CSub( &a , &b ) subtracts  a  from  b  and returns  &b
*/

complex *CSub( ap , bp )
	register complex	*ap , *bp ;	// may coincide (?)
	{
	bp->r -= ap->r ;
	bp->i -= ap->i ;
	return ( bp );
	}
/*
	CMul - multiply one complex by another

	CMul( &a , &b ) multiplies  b  by  a  and returns  &b
*/

complex *CMul( ap , bp )
	register complex	*ap , *bp ;	// may coincide
	{
	double			ar , br ;

	ar = ap->r ;			// in case they coincide
	br = bp->r ;			// to allow overwriting
	bp->r = br * ar - bp->i * ap->i ;
	bp->i = br * ap->i + bp->i * ar ;
	return ( bp );
	}


/*
	CDiv - divide one complex by another

	CDiv( &a , &b ) divides  b  by	a  and returns	&b ;
			a zero divisor fails
*/

complex *CDiv( ap , bp )
	register complex	*ap , *bp ;	// may coincide (?)
	{
	double			a , b , r , s ;

	// Note: classical formula may cause unnecessary overflow
	a = bp->r ;
	b = bp->i ;
#define abs( x )			((x) >= 0 ? (x) : -(x))
	if ( abs( ap->r ) >= abs( ap->i ) )
		{
		r = ap->i / ap->r ;
		s = ap->r + r * ap->i ;
		bp->r = (a + b * r) / s ;
		bp->i = (b - a * r) / s ;
		}
	else
		{
		r = ap->r / ap->i ;
		s = ap->i + r * ap->r ;
		bp->r = (a * r + b) / s ;
		bp->i = (b * r - a) / s ;
		}
	return ( bp );
	}
