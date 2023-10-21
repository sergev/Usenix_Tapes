/*
 * number.c - TIG-pack library routine to plot a number
 *
 *	last edit: 02-Dec-1980	D A Gwyn
 *
 * This routine converts a floating-point number to a character string
 * then plots the string.
 *
 * Calling sequence:
 * number( input , x , y , height , theta , digits );
 *	where
 * double input	= floating-point number to be plotted
 * unsigned x,y	= position (in 1/1000ths of an inch) of the string LLC
 * int height	= height of the characters (in 1/1000ths)
 * double theta	= degrees of rotation counter-clockwise from x-axis
 * int digits	= number of digits to follow the decimal point
 *
 * Compile:
 *	# cc -c -O number.c
 *	# ar r /lib/libg.a number.o
 *
 * Usage:
 *	% cc application_program.c -lg
 */

#include	<math.h>

struct	coords
	{
	unsigned	x ;
	unsigned	y ;
	};


number( input , xy , height , theta , digits )
	double		input ;
	struct coords	xy ;
	int		height ;
	double		theta ;
	int		digits ;
	{
	extern char	*gcvt();
	char		buf[25];

	if ( digits < -20 | digits > 20 )
		digits = 0 ;
	symbol( gcvt( input , (int)ceil( log10( input ) ) + digits , buf ) ,
	xy , height , theta );
	}
