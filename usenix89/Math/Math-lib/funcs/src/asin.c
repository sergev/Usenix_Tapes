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
 *	asin   double precision arc sine
 *
 *  KEY WORDS
 *
 *	asin
 *	machine independent routines
 *	trigonometric functions
 *	math libraries
 *
 *  DESCRIPTION
 *
 *	Returns double precision arc sine of double precision
 *	floating point argument.
 *
 *	If argument is less than -1.0 or greater than +1.0, calls
 *	matherr with a DOMAIN error.  If matherr does not handle
 *	the error then prints error message and returns 0.
 *
 *  USAGE
 *
 *	double asin (x)
 *	double x;
 *
 *  REFERENCES
 *
 *	Fortran IV-plus user's guide, Digital Equipment Corp. pp B-2.
 *
 *  RESTRICTIONS
 *
 *	For precision information refer to documentation of the floating
 *	point library primatives called.
 *	
 *  PROGRAMMER
 *
 *	Fred Fish
 *
 *  INTERNALS
 *
 *	Computes arcsine(x) from:
 *
 *		(1)	If x < -1.0 or x > +1.0 then
 *			call matherr and return 0.0 by default.
 *
 *		(2)	If x = 0.0 then asin(x) = 0.0
 *
 *		(3)	If x = 1.0 then asin(x) = PI/2.
 *
 *		(4)	If x = -1.0 then asin(x) = -PI/2
 *
 *		(5)	If -1.0 < x < 1.0 then
 *			asin(x) = atan(y)
 *			y = x / sqrt[1-(x**2)]
 *
 */

#include <stdio.h>
#include <pmluser.h>
#include "pml.h"

static char funcname[] = "asin";


double asin (x)
double x;
{
    extern double atan ();
    extern double sqrt ();
    struct exception xcpt;

    DBUG_ENTER (funcname);
    DBUG_3 ("asinin", "arg %le", x);
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
	xcpt.retval = 0.0;
    } else if (x == 1.0) {
	xcpt.retval = HALFPI;
    } else if (x == -1.0) {
	xcpt.retval = -HALFPI;
    } else {
	xcpt.retval = atan ( x / sqrt (1.0 - (x * x)) );
    }
    DBUG_3 ("asinout", "result %le", xcpt.retval);
    DBUG_RETURN (xcpt.retval);
}
