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
 *	cdiv   double precision complex division
 *
 *  KEY WORDS
 *
 *	cdiv
 *	complex functions
 *	machine independent routines
 *	math libraries
 *
 *  DESCRIPTION
 *
 *	Computes double precision complex result of division of
 *	first double precision complex argument by second double
 *	precision complex argument.
 *
 *  USAGE
 *
 *	COMPLEX cdiv (numerator, denominator)
 *	COMPLEX numerator;
 *	COMPLEX denominator;
 *
 *  PROGRAMMER
 *
 *	Fred Fish
 *	Tempe, Az 85281
 *	(602) 966-8871
 *
 *  INTERNALS
 *
 *	Computes cdiv(znum,zden) from:
 *
 *		1.	Let znum = a + j b
 *			Let zden = c + j d
 *
 *		2.	denom = c*c + d*d
 *
 *		3.	If denom is zero then log error,
 *			set r_cdiv = maximum floating value,
 *			i_cdiv = 0, and go to step 5.
 *
 *		4.	r_cdiv = (a*c + b*d) / denom
 *			i_cdiv = (c*b - a*d) / denom
 *
 *		5.	Then cdiv(znum,zden) = r_cdiv + j i_cdiv
 *
 */

#include <stdio.h>
#include <pmluser.h>
#include "pml.h"


COMPLEX cdiv (znum, zden)
COMPLEX znum;
COMPLEX zden;
{
    COMPLEX result;
    double denom;

    ENTER ("cdiv");
    DEBUG4 ("cdivin", "arg1 %le %le", znum.real, znum.imag);
    DEBUG4 ("cdivin", "arg2 %le %le", zden.real, zden.imag);
    denom = (zden.real * zden.real) + (zden.imag * zden.imag);
    if (denom == 0.0) {
/****
	pmlerr(C_DIV_ZERO);
	result.real = MAX_POS_DBLF;
******/
	result.real = 0.0;	/* TERRIBLY WRONG! */
	result.imag = 0.0;
    } else {
	result.real = ((znum.real*zden.real)+(znum.imag*zden.imag)) / denom;
	result.imag = ((zden.real*znum.imag)-(znum.real*zden.imag)) / denom;
    }
    DEBUG4 ("cdivout", "result %le %le", result.real, result.imag);
    LEAVE ();
    return (result);
}
