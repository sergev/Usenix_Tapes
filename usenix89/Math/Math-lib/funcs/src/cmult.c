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
 *	cmult   double precision complex multiplication
 *
 *  KEY WORDS
 *
 *	cmult
 *	complex functions
 *	machine independent routines
 *	math libraries
 *
 *  DESCRIPTION
 *
 *	Computes double precision complex result of multiplication of
 *	first double precision complex argument by second double
 *	precision complex argument.
 *
 *  USAGE
 *
 *	COMPLEX cmult (z1, z2)
 *	COMPLEX z1;
 *	COMPLEX z2;
 *
 *  PROGRAMMER
 *
 *	Fred Fish
 *	Tempe, Az 85281
 *	(602) 966-8871
 *
 *  INTERNALS
 *
 *	Computes cmult(z1,z2) from:
 *
 *		1.	Let z1 = a + j b
 *			Let z2 = c + j d
 *
 *		2.	r_cmult = (a*c - b*d)
 *			i_cmult = (a*d + c*b)
 *
 *		3.	Then cmult(z1,z2) = r_cmult + j i_cmult
 *
 */

#include <stdio.h>
#include <pmluser.h>
#include "pml.h"


COMPLEX cmult (z1, z2)
COMPLEX z1;
COMPLEX z2;
{
    COMPLEX result;

    ENTER ("cmult");
    DEBUG4 ("cmultin", "arg1 %le %le", z1.real, z1.imag);
    DEBUG4 ("cmultin", "arg2 %le %le", z2.real, z2.imag);
    result.real = (z1.real * z2.real) - (z1.imag * z2.imag);
    result.imag = (z1.real * z2.imag) + (z2.real * z1.imag);
    DEBUG4 ("cmultout", "result %le %le", result.real, result.imag);
    LEAVE ();
    return (result);
}
