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
 *	crcp   complex double precision reciprocal
 *
 *  KEY WORDS
 *
 *	crcp
 *	complex functions
 *	math libraries
 *
 *  DESCRIPTION
 *
 *	Computes double precision complex reciprocal of
 *	a double precision complex argument.
 *
 *  USAGE
 *
 *	COMPLEX crcp (z)
 *	COMPLEX z;
 *
 *  PROGRAMMER
 *
 *	Fred Fish
 *	Tempe, Az 85281
 *	(602) 966-8871
 *
 *  INTERNALS
 *
 *	Computes complex reciprocal of z = x + j y from:
 *
 *		1.	Compute denom = x*x + y*y
 *
 *		2.	If denom = 0.0 then flag error
 *			and return MAX_POS_DBLF + j 0.0
 *
 *		3.	Else crcp(z) = (x/denom) + j (-y/denom)
 *
 */

#include <stdio.h>
#include <pmluser.h>
#include "pml.h"


COMPLEX crcp (z)
COMPLEX z;
{
    double denom;

    ENTER ("crcp");
    DEBUG4 ("crcpin", "arg %le %le", z.real, z.imag);
    denom = (z.real * z.real) + (z.imag * z.imag);
    if (denom == 0.0) {
/*****
	pmlerr(CRCP_OF_ZERO);
	z.real = MAX_POS_DBLF;
******/
	z.real = 0.0;		/* TERRIBLY WRONG */
	z.imag = 0.0;
    } else {
	z.real /= denom;
	z.imag /= -denom;
    }
    DEBUG4 ("crcpout", "result %le %le", z.real, z.imag);
    LEAVE ();
    return (z);
}
