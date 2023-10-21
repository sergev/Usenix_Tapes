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
 *	frac   double precision fractional portion
 *
 *  KEY WORDS
 *
 *	frac
 *	machine dependent routines
 *	math libraries
 *
 *  DESCRIPTION
 *
 *	Returns fractional portion of double precision number as double
 *	precision number.
 *
 *  USAGE
 *
 *	double frac(x)
 *	double x;
 *
 *  PROGRAMMER
 *
 *	Fred Fish
 *	Tempe, Az 85281
 *	(602) 966-8871
 *
 */
 
#include <stdio.h>
#include <pmluser.h>
#include "pml.h"
 

double frac(x)
double x;
{
    double dint();
 
    return (x - dint(x));
}
