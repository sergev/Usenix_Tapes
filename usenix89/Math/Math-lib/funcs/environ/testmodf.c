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
 *	testmodf.c    test the runtime environment function modf
 *
 *	This simple minded program is provided to aid in testing
 *	the "modf" function assumed to be provided in the runtime
 *	environment.  If not provided, a suitable substitute can
 *	be coded in C, however the necessary code is very machine
 *	dependent and is generally almost trivial to code in assembly
 *	language for the specific host machine.
 *
 *	The modf() function takes two arguments.  The first is a double value
 *	and the second is a pointer to a double.  Modf() returns the
 *	signed fraction part of the first argument, and stores the integral
 *	part (as a double) indirectly in the location pointed to by the
 *	second argument.  Note that both the direct and indirect result will
 *	have the same sign, and:
 *
 *		<integral part> + <fractional part> = <original value>
 *
 *	See "frexp(3C)" in the Unix System V User's Manual for more
 *	information.
 *
 */
 
extern double modf ();

main ()
{
    double input;			/* Input value */
    double frac;
    double ipart;
    
    while (scanf ("%le", &input) == 1) {
	frac = modf (input, &ipart);
	printf ("%le %le\n", frac, ipart);
    }
}
