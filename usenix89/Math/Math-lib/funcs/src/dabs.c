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
 *	dabs,fabs   double precision absolute value
 *
 *  KEY WORDS
 *
 *	dabs
 *	fabs
 *	machine independent routines
 *	math libraries
 *
 *  DESCRIPTION
 *
 *	Returns absolute value of double precision number.
 *
 *	The fabs routine is supplied for compatibility with unix
 *	libraries where for some bizarre reason, the double precision
 *	absolute value routine is called fabs.
 *
 *  USAGE
 *
 *	double dabs (x)
 *	double x;
 *
 *	double fabs (x)
 *	double x;
 * 
 *  PROGRAMMER
 *
 *	Fred Fish
 *
 */

#include <stdio.h>
#include <pmluser.h>
#include "pml.h"

static char funcname[] = "dabs";


double dabs (x)
double x;
{
    DBUG_ENTER (funcname);
    DBUG_3 ("dabsin", "arg %le", x);
    if (x < 0.0) {
	x = -x;
    }
    DBUG_3 ("dabsout", "result %le", x);
    DBUG_RETURN (x);
}


double fabs (x)
double x;
{
    return (dabs(x));
}
