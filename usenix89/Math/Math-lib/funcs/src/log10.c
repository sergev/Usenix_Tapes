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
 *	log10   double precision common log
 *
 *  KEY WORDS
 *
 *	log10
 *	machine independent routines
 *	math libraries
 *
 *  DESCRIPTION
 *
 *	Returns double precision common log of double precision
 *	floating point argument.
 *
 *  USAGE
 *
 *	double log10 (x)
 *	double x;
 *
 *  REFERENCES
 *
 *	PDP-11 Fortran IV-plus users guide, Digital Equip. Corp.,
 *	1975, pp. B-3.
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
 *	Computes log10(x) from:
 *
 *		log10(x) = log10(e) * log(x)
 *
 */

#include <stdio.h>
#include <pmluser.h>
#include "pml.h"

static char funcname[] = "log10";


double log10 (x)
double x;
{
    extern double log ();

    DBUG_ENTER (funcname);
    DBUG_3 ("log10in", "arg %le", x);
    x = LOG10E * log (x);
    DBUG_3 ("log10out", "result %le", x);
    DBUG_RETURN (x);
}
