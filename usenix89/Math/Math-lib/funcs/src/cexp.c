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
 *	cexp   complex double precision exponential
 *
 *  KEY WORDS
 *
 *	cexp
 *	complex functions
 *	machine independent routines
 *	math libraries
 *
 *  DESCRIPTION
 *
 *	Computes double precision complex exponential of
 *	a double precision complex argument.
 *
 *  USAGE
 *
 *	COMPLEX cexp (z)
 *	COMPLEX z;
 *
 *  REFERENCES
 *
 *	Fortran 77 user's guide, Digital Equipment Corp. pp B-13
 *
 *  PROGRAMMER
 *
 *	Fred Fish
 *	Tempe, Az 85281
 *	(602) 966-8871
 *
 *  INTERNALS
 *
 *	Computes complex exponential of z = x + j y from:
 *
 *		1.	r_cexp = exp(x) * cos(y)
 *
 *		2.	i_cexp = exp(x) * sin(y)
 *
 *		3.	cexp(z) = r_cexp + j i_cexp
 *
 */

#include <stdio.h>
#include <pmluser.h>
#include "pml.h"


COMPLEX cexp (z)
COMPLEX z;
{
    COMPLEX result;
    double expx;
    extern double exp(), sin(), cos();

    ENTER ("cexp");
    DEBUG4 ("cexpin", "arg %le %le", z.real, z.imag);
    expx = exp (z.real);
    result.real = expx * cos (z.imag);
    result.imag = expx * sin (z.imag);
    DEBUG4 ("cexpout", "result %le %le", result.real, result.imag);
    LEAVE ();
    return (result);
}
