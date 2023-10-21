/************************************************************************
 *									*
 *				N O T I C E				*
 *									*
 *			Copyright Abandoned, 1987, Fred Fish		*
 *									*
 *	This previously copyrighted work has been placed into the	*
 *	public domain by the author (Fred Fish) and may be freely used	*
 *	for any purpose, private or commercial.  I would appreciate	*
 *	it, as a courtesy, if this notice is left in all copies and	*
 *	derivative works.  Thank you, and enjoy...			*
 *									*
 *	The author makes no warranty of any kind with respect to this	*
 *	product and explicitly disclaims any implied warranties of	*
 *	merchantability or fitness for any particular purpose.		*
 *									*
 ************************************************************************
 */


/*
 *  FUNCTION
 *
 *	cabs   double precision complex absolute value
 *
 *  KEY WORDS
 *
 *	cabs
 *	complex functions
 *	machine independent routines
 *	math libraries
 *
 *  DESCRIPTION
 *
 *	Computes double precision absolute value of a double
 *	precision complex argument, where "absolute value"
 *	is taken to mean magnitude.  The result replaces the
 *	argument.
 *
 *  USAGE
 *
 *	double cabs (z)
 *	COMPLEX z;
 *
 *  PROGRAMMER
 *
 *	Fred Fish
 *	Tempe, Az
 *
 *  INTERNALS
 *
 *	Computes cabs (z) where z = (x) + j(y) from:
 *
 *		cabs (z) = sqrt (x*x + y*y)
 *
 */

#include <stdio.h>
#include <pmluser.h>
#include "pml.h"


double cabs (z)
COMPLEX z;
{
    double result;
    extern double sqrt ();

    ENTER ("cabs");
    DEBUG4 ("cabsin", "arg %le +j %le", z.real, z.imag);
    result = sqrt ((z.real * z.real) + (z.imag * z.imag));
    DEBUG3 ("cabsout", "result %le", result);
    LEAVE ();
    return (result);
}
