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
 *	sinh   double precision hyperbolic sine
 *
 *  KEY WORDS
 *
 *	sinh
 *	machine independent routines
 *	math libraries
 *
 *  DESCRIPTION
 *
 *	Returns double precision hyperbolic sine of double precision
 *	floating point number.
 *
 *  USAGE
 *
 *	double sinh (x)
 *	double x;
 *
 *  REFERENCES
 *
 *	Fortran IV plus user's guide, Digital Equipment Corp. pp B-5
 *
 *  RESTRICTIONS
 *
 *	Inputs greater than ln(MAXDOUBLE) result in overflow.
 *	Inputs less than ln(MINDOUBLE) result in underflow.
 *
 *	For precision information refer to documentation of the
 *	floating point library routines called.
 *	
 *  PROGRAMMER
 *
 *	Fred Fish
 *
 *  INTERNALS
 *
 *	Computes hyperbolic sine from:
 *
 *		sinh(x) = 0.5 * (exp(x) - 1.0/exp(x))
 *
 */

#include <stdio.h>
#include <pmluser.h>
#include "pml.h"

static char funcname[] = "sinh";


double sinh (x)
double x;
{
    extern double exp ();
    auto struct exception xcpt;

    DBUG_ENTER (funcname);
    DBUG_3 ("sinhin", "arg %le", x);
    if (x > LOGE_MAXDOUBLE) {
	xcpt.type = OVERFLOW;
	xcpt.name = funcname;
	xcpt.arg1 = x;
	if (!matherr (&xcpt)) {
	    fprintf (stderr, "%s: OVERFLOW error\n", funcname);
	    errno = ERANGE;
	    xcpt.retval = MAXDOUBLE;
	}
    } else if (x < LOGE_MINDOUBLE) {
	xcpt.type = UNDERFLOW;
	xcpt.name = funcname;
	xcpt.arg1 = x;
	if (!matherr (&xcpt)) {
	    fprintf (stderr, "%s: UNDERFLOW error\n", funcname);
	    errno = ERANGE;
	    xcpt.retval = MINDOUBLE;
	}
    } else {
	x = exp (x);
	xcpt.retval = 0.5 * (x - (1.0 / x));
    }
    DBUG_3 ("sinhout", "result %le", xcpt.retval);
    DBUG_RETURN (xcpt.retval);
}
