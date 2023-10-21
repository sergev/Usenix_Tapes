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
 *	mod   double precision modulo
 *
 *  KEY WORDS
 *
 *	mod
 *	machine independent routines
 *	math libraries
 *
 *  DESCRIPTION
 *
 *	Returns double precision modulo of two double
 *	precision arguments.
 *
 *  USAGE
 *
 *	double mod (value, base)
 *	double value;
 *	double base;
 *
 *  PROGRAMMER
 *
 *	Fred Fish
 *
 */


#include <stdio.h>
#include <pmluser.h>
#include "pml.h"

double mod (value, base)
double value;
double base;
{
    auto double intpart;
    extern double modf ();

    DBUG_ENTER ("mod");
    DBUG_4 ("modin", "args %le %le", value, base);
    value /= base;
    value = modf (value, &intpart);
    value *= base;
    DBUG_3 ("modout", "result %le", value);
    DBUG_RETURN (value);
}
