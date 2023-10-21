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
 *	atanh   double precision hyperbolic arc tangent
 *
 *  KEY WORDS
 *
 *	atanh
 *	machine independent routines
 *	math libraries
 *
 *  DESCRIPTION
 *
 *	Returns double precision hyperbolic arc tangent of double precision
 *	floating point number.
 *
 *  USAGE
 *
 *	double atanh (x)
 *	double x;
 *
 *  RESTRICTIONS
 *
 *	The range of the atanh function is -1.0 to +1.0 exclusive.
 *	Certain pathological cases near 1.0 may cause overflow
 *	in evaluation of 1+x / 1-x, depending upon machine exponent
 *	range and mantissa precision.
 *
 *	For precision information refer to documentation of the
 *	other floating point library routines called.
 *	
 *  PROGRAMMER
 *
 *	Fred Fish
 *
 *  INTERNALS
 *
 *	Computes atanh(x) from:
 *
 *		1.	If x <= -1.0 or x >= 1.0
 *			then report argument out of
 *			range and return 0.0
 *
 *		2.	atanh(x) = 0.5 * log((1+x)/(1-x))
 *
 */

#include <stdio.h>
#include <pmluser.h>
#include "pml.h"

static char funcname[] = "atanh";


double atanh (x)
double x;
{
    auto struct exception xcpt;
    extern double log ();

    DBUG_ENTER (funcname);
    DBUG_3 ("atanhin", "arg %le", x);
    if (x <= -1.0 || x >= 1.0) {
	xcpt.type = DOMAIN;
	xcpt.name = funcname;
	xcpt.arg1 = x;
	if (!matherr (&xcpt)) {
	    fprintf (stderr, "%s: DOMAIN error\n", funcname);
	    errno = ERANGE;
	    xcpt.retval = 0.0;
	}
    } else {
	xcpt.retval = 0.5 * log ((1+x) / (1-x));
    }
    DBUG_3 ("atanhout", "result %le", xcpt.retval);
    DBUG_RETURN (xcpt.retval);
}
