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
 *	dint   double precision integer portion
 *
 *  KEY WORDS
 *
 *	dint
 *	machine dependent routines
 *	math libraries
 *
 *  DESCRIPTION
 *
 *	Returns integer portion of double precision number as double
 *	precision number.
 *
 *  USAGE
 *
 *	double dint(x)
 *	double x;
 *
 *  PROGRAMMER
 *
 *	Fred Fish
 *	Tempe, Az 85281
 *	(602) 966-8871
 *
 *  RESTRICTIONS
 *
 *	The current DEC-20 C system treats double as float.  This
 *	routine will need to be modified when true double precision
 *	is implemented.
 *
 */
 
#include <stdio.h>
#include <pmluser.h>
#include "pml.h"
 

#ifdef PDP10

#define W1_FBITS  27		/* Number of fractional bits in word 1 */
#define WORD_MASK 0777777777777	/* Mask for all 36 bits of first word	*/
 
double dint(x)
double x;
{
    register int *vpntr, exponent;
    int xexp();

    vpntr = &x;
    if ((exponent=xexp(x)) <= 0) {
	x = 0.0;
    } else if (exponent <= W1_FBITS) {
	*vpntr &= (WORD_MASK << (W1_FBITS - exponent));
    } else {
	pmlerr(DINT_2BIG);
	x = 0.0;
    }
    if (x < 0.0) {
	x += 1.0;
    }
    return (x);
}

#else

#define W1_FBITS  24		/* (NOTE HIDDEN BIT NORMALIZATION)	*/
#define W2_FBITS  32		/* Number of fractional bits in word 2	*/
#define WORD_MASK 0xFFFFFFFF	/* Mask for each long word of double	*/
 
static union {
    double dval;
    long lval[2];
} share;

double dint(x)
double x;
{
    int exponent, xexp(), fbitdown;
    register long *lpntr;
 
    lpntr = &share.lval[0];
    share.dval = x;
    if ((exponent=xexp(x)) <= 0) {
	share.dval = 0.0;
    } else if (exponent <= W1_FBITS) {
	*lpntr++ &= (WORD_MASK << (W1_FBITS - exponent));
	*lpntr++ = 0;
    } else if (exponent <= (fbitdown = W1_FBITS+W2_FBITS)) {
	lpntr++;
	*lpntr++ &= (WORD_MASK << (fbitdown - exponent));
    } else {
	pmlerr(DINT_2BIG);
    }
    return (share.dval);
}
#endif
