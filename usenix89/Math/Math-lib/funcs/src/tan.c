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
 *	tan   Double precision tangent
 *
 *  KEY WORDS
 *
 *	tan
 *	machine independent routines
 *	trigonometric functions
 *	math libraries
 *
 *  DESCRIPTION
 *
 *	Returns tangent of double precision floating point number.
 *
 *  USAGE
 *
 *	double tan (x)
 *	double x;
 *
 *  INTERNALS
 *
 *	Computes the tangent from tan(x) = sin(x) / cos(x).
 *
 *	If cos(x) = 0 and sin(x) >= 0, then returns largest
 *	floating point number on host machine.
 *
 *	If cos(x) = 0 and sin(x) < 0, then returns smallest
 *	floating point number on host machine.
 *
 *  REFERENCES
 *
 *	Fortran IV plus user's guide, Digital Equipment Corp. pp. B-8
 *
 */

#include <stdio.h>
#include <pmluser.h>
#include "pml.h"

static char funcname[] = "tan";

double tan (x)
double x;
{
    double sinx;
    double cosx;
    auto struct exception xcpt;
    extern double sin ();
    extern double cos();

    DBUG_ENTER (funcname);
    DBUG_3 ("tanin", "arg %le", x);
    sinx = sin (x);
    cosx = cos (x);
    if (cosx == 0.0) {
	xcpt.type = OVERFLOW;
	xcpt.name = funcname;
	xcpt.arg1 = x;
	if (!matherr (&xcpt)) {
	    fprintf (stderr, "%s: OVERFLOW error\n", funcname);
	    errno = ERANGE;
	    if (sinx >= 0.0) {
		xcpt.retval = MAXDOUBLE;
	    } else {
		xcpt.retval = -MAXDOUBLE;
	    }
	}
    } else {
	xcpt.retval = sinx / cosx;
    }
    DBUG_3 ("tanout", "result %le", xcpt.retval);
    DBUG_RETURN (xcpt.retval);
}
