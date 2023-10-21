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
 *	sign   transfer of sign
 *
 *  KEY WORDS
 *
 *	sign
 *	machine independent routines
 *	math libraries
 *
 *  DESCRIPTION
 *
 *	Returns first argument with same sign as second argument.
 *
 *  USAGE
 *
 *	double sign (x, y)
 *	double x;
 *	double y;
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


double sign (x, y)
double x;
double y;
{
    double rtnval;
    
    ENTER ("sign");
    DEBUG4 ("signin", "args %le %le", x, y);
    if (x > 0.0) {
	if (y > 0.0) {
	    rtnval = x;
	} else {
	    rtnval = -x;
	}
    } else {
	if (y < 0.0) {
	    rtnval = x;
	} else {
	    rtnval = -x;
	}
    }
    DEBUG3 ("signout", "result %le", rtnval);
    LEAVE ();
    return (rtnval);
}
