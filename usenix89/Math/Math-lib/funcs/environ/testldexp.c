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
 *	testldexp.c    test the runtime environment function ldexp
 *
 *  DESCRIPTION
 *
 *	This simple minded program is provided to aid in testing
 *	the "ldexp" function assumed to be provided in the runtime
 *	environment.  If not provided, a suitable substitute can
 *	be coded in C, however the necessary code is very machine
 *	dependent and is generally almost trivial to code in assembly
 *	language for the specific host machine.
 *
 *	The ldexp() function takes two arguments, the first is a double
 *	which is the mantissa of the returned double value.  The second
 *	is an int which is the exponent of the returned double.  The
 *	result is (mantissa * (2 ** exp)).
 *
 *	See "frexp(3C)" in the Unix System V User's Manual for more
 *	information.
 *
 *	This program is typically used as:
 *
 *		testldexp <testldexp.in >junkfile
 *		diff testldexp.out junkfile
 *
 */
 
#include <stdio.h>

extern double ldexp ();

main ()
{
    double dmant;			/* Mantissa, 0.5 LE |dmant| LT 1.0 */
    int intexp;				/* Exponent as power of 2 */
    double result;			/* dmant times 2 to the intexp */
    
    while (scanf ("%le %d", &dmant, &intexp) == 2) {
	result = ldexp (dmant, intexp);
	printf ("%le\n", result);
    }
}
