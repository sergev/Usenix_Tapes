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
 *  FILE
 *
 *	pmluser.h    include file for users of portable math library
 *
 *  SYNOPSIS
 *
 *	#include <pmluser.h>
 *
 *  DESCRIPTION
 *
 *	This file should be included in any user compilation module
 *	which accesses routines from the Portable Math Library (PML).
 *
 */


/*
 *	Create the type "COMPLEX".  This is an obvious extension that I
 *	hope becomes a part of standard C someday.
 *
 */

typedef struct cmplx {			/* Complex structure */
    double real;			/* Real part */
    double imag;			/* Imaginary part */
} COMPLEX;
