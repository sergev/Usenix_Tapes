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
 *	max   double precision maximum of two arguments
 *
 *  KEY WORDS
 *
 *	max
 *	machine independent routines
 *	math libraries
 *
 *  DESCRIPTION
 *
 *	Returns maximum value of two double precision numbers.
 *
 *  USAGE
 *
 *	double max (x,y)
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


double max (x, y)
double x;
double y;
{
    ENTER ("max");
    DEBUG4 ("maxin", "x = %le  y = %le", x, y);
    if (x < y) {
	x = y;
    }
    DEBUG3 ("maxout", "result %le", x);
    LEAVE ();
    return (x);
}
