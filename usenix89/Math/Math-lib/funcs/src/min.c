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
 *	min   double precision minimum of two arguments
 *
 *  KEY WORDS
 *
 *	min
 *	machine independent routines
 *	math libraries
 *
 *  DESCRIPTION
 *
 *	Returns minimum value of two double precision numbers.
 *
 *  USAGE
 *
 *	double min (x, y)
 *	double x;
 *	double y;
 *
 *  PROGRAMMER
 *
 *	Fred Fish
 *	Tempe, Az 85281
 *
 */

#include <stdio.h>
#include <pmluser.h>
#include "pml.h"


double min (x, y)
double x;
double y;
{
    ENTER ("min");
    DEBUG4 ("minin", "x = %le  y = %le", x, y);
    if (x > y) {
	x = y;
    }
    DEBUG3 ("minout", "result %le", x);
    LEAVE ();
    return (x);
}
