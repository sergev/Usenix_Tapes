/*
 * marker.c - TIG-pack library routine to plot a marker symbol
 *
 *	last edit: 02-Dec-1980	D A Gwyn
 *
 * This routine plots a specified character (either from the ASCII set,
 * or one of the 32 special marker characters) centered about the
 * current pen position.
 *
 * Calling sequence:
 * marker( c );
 *	where
 * int c	= the character to be used for a marker,
 *		  or one of the special markers -
 *			-16 .. -1	nonstandard
 *			  0 .. 15	Calcomp standard
 *
 * Compile:
 *	# cc -c -O marker.c
 *	# ar r /lib/libg.a marker.o
 *
 * Usage:
 *	% cc application_program.c -lg
 */

#define HEIGHT	100		/* Marker height in 1/1000ths */
#define HALF	75		/* symbol box height / 2 */

struct	coords
	{
	unsigned	x ;
	unsigned	y ;
	};

extern struct coords	tiglast ;	/* current pen position */


marker( c )				/* plot marker symbol */
	register int	c ;
	{
	struct coords	save ;
	char		str[2];

	save = tiglast ;		/* save pen position */

	if ( c < ' ' )			/* special character */
		c += 16 ;

	str[0] = c ;
	str[1] = '\0';
	symbol( str , save.x - HEIGHT/2 , save.y - HALF , HEIGHT , 0.0 );

	movepen( save );		/* restore pen position */
	}
