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
 *	acos   double precision arc cosine
 *
 *  KEY WORDS
 *
 *	acos
 *	machine independent routines
 *	trigonometric functions
 *	math libraries
 *
 *  DESCRIPTION
 *
 *	Returns double precision arc cosine of double precision
 *	floating point argument.
 *
 *  USAGE
 *
 *	double acos (x)
 *	double x;
 *
 *  REFERENCES
 *
 *	Fortran IV-plus user's guide, Digital Equipment Corp. pp B-1.
 *
 *  RESTRICTIONS
 *
 *	The maximum relative error for the approximating polynomial
 *	in atan is 10**(-16.82).  However, this assumes exact arithmetic
 *	in the polynomial evaluation.  Additional rounding and
 *	truncation errors may occur as the argument is reduced
 *	to the range over which the polynomial approximation
 *	is valid, and as the polynomial is evaluated using
 *	finite-precision arithmetic.
 *	
 *  PROGRAMMER
 *
 *	Fred Fish
 *
 *  INTERNALS
 *
 *	Computes arccosine(x) from:
 *
 *		(1)	If x < -1.0  or x > +1.0 then call
 *			matherr and return 0.0 by default.
 *
 *		(2)	If x = 0.0 then acos(x) = PI/2.
 *
 *		(3)	If x = 1.0 then acos(x) = 0.0
 *
 *		(4)	If x = -1.0 then acos(x) = PI.
 *
 *		(5)	If 0.0 < x < 1.0 then
 *			acos(x) = atan(Y)
 *			Y = sqrt[1-(x**2)] / x 
 *
 *		(4)	If -1.0 < x < 0.0 then
 *			acos(x) = atan(Y) + PI
 *			Y = sqrt[1-(x**2)] / x 
 *
 */

#include <stdio.h>
#include <pmluser.h>
#include "pml.h"

static char funcname[] = "acos";


double acos (x)
double x;
{
    double y;
    extern double atan();
    extern double sqrt();
    auto struct exception xcpt;
    
    DBUG_ENTER (funcname);
    DBUG_3 ("acosin", "arg %le", x);
    if ( x > 1.0 || x < -1.0) {
	xcpt.type = DOMAIN;
	xcpt.name = funcname;
	xcpt.arg1 = x;
	if (!matherr (&xcpt)) {
	    fprintf (stderr, "%s: DOMAIN error\n", funcname);
	    errno = EDOM;
	    xcpt.retval = 0.0;
	}
    } else if (x == 0.0) {
	xcpt.retval = HALFPI;
    } else if (x == 1.0) {
	xcpt.retval = 0.0;
    } else if (x == -1.0) {
	xcpt.retval = PI;
    } else {
	y = atan ( sqrt (1.0 - (x * x)) / x );
	if (x > 0.0) {
	    xcpt.retval = y;
	} else {
	    xcpt.retval = y + PI;
	}
    }
    DBUG_3 ("acosout", "result %le", x);
    DBUG_RETURN (x);
}
