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
 *	tanh   double precision hyperbolic tangent
 *
 *  KEY WORDS
 *
 *	tanh
 *	machine independent routines
 *	math libraries
 *
 *  DESCRIPTION
 *
 *	Returns double precision hyperbolic tangent of double precision
 *	floating point number.
 *
 *  USAGE
 *
 *	double tanh (x)
 *	double x;
 *
 *  REFERENCES
 *
 *	Fortran IV plus user's guide, Digital Equipment Corp. pp B-5
 *
 *  RESTRICTIONS
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
 *	Computes hyperbolic tangent from:
 *
 *		tanh(x) = 1.0 for x > TANH_MAXARG
 *			= -1.0 for x < -TANH_MAXARG
 *			=  sinh(x) / cosh(x) otherwise
 *
 */

#include <stdio.h>
#include <pmluser.h>
#include "pml.h"

static char funcname[] = "tanh";


double tanh (x)
double x;
{
    auto struct exception xcpt;
    register int positive;
    extern double sinh ();
    extern double cosh ();

    DBUG_ENTER (funcname);
    DBUG_3 ("tanhin", "arg %le", x);
    if (x > TANH_MAXARG || x < -TANH_MAXARG) {
	if (x > 0.0) {
	    positive = 1;
	} else {
	    positive = 0;
	}
	xcpt.type = PLOSS;
	xcpt.name = funcname;
	xcpt.arg1 = x;
	if (!matherr (&xcpt)) {
	    fprintf (stderr, "%s: PLOSS error\n", funcname);
	    errno = ERANGE;
	    if (positive) {
		xcpt.retval = 1.0;
	    } else {
		xcpt.retval = -1.0;
	    }
	}
    } else {
	xcpt.retval = sinh (x) / cosh (x);
    }
    DBUG_3 ("tanhout", "result %le", xcpt.retval);
    return (xcpt.retval);
}
