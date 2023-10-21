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
 *	testfrexp.c    test the runtime environment function frexp
 *
 *  DESCRIPTION
 *
 *	This simple minded program is provided to aid in testing
 *	the "frexp" function assumed to be provided in the runtime
 *	environment.  If not provided, a suitable substitute can
 *	be coded in C, however the necessary code is very machine
 *	dependent and is generally almost trivial to code in assembly
 *	language for the specific host machine.
 *
 *	The frexp() function takes two arguments, the first is a double
 *	and the second is a pointer to an int.  Frexp() returns the
 *	mantissa of the first arg, and stores the exponent indirectly
 *	in the location pointed to by the second arg.
 *	
 *	See "frexp(3C)" in the Unix System V User's Manual for more
 *	information.
 *
 *	This program is typically used as:
 *
 *		testfrexp <testfrexp.in >junkfile
 *		diff testfrexp.out junkfile
 *
 */
 
#include <stdio.h>

extern double frexp ();

main ()
{
    double input;			/* Input value */
    double dmant;			/* Mantissa, 0.5 LE |dmant| LT 1.0 */
    int intexp;				/* Exponent as power of 2 */
    
    while (scanf ("%le", &input) == 1) {
	dmant = frexp (input, &intexp);
	printf ("%le %d\n", dmant, intexp);
    }
}
