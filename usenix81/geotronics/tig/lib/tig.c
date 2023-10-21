/*
 * tig.c - TIG-pack C-callable output routines
 *
 *	last edit: 02-Dec-1980	D A Gwyn
 *
 * These routines are the graphic primitives for /lib/libg.a .
 * The original TIG-pack author was Michael J. Muuss of Johns Hopkins.
 * This edition always writes to stdout (use "tee" if you need to).
 *
 * Compile:
 *	# cc -c -O tig.c
 *	# ar r /lib/libg.a tig.o
 *
 * Usage:
 *	% cc application_program.c -lg
 */

#include	<stdio.h>


penup()					/* raise virtual pen */
	{
	putchar( 'U' );
	}


pendown()				/* lower virtual pen */
	{
	putchar( 'D' );
	}


newform()				/* start new display */
	{
	putchar( 'F' );
	}


struct	coords				/* TIG "UPA" coordinates */
	{
	unsigned	x ;
	unsigned	y ;
	};

extern struct coords	tiglast = { 0 , 0 };	/* last "move" pos */
static struct coords	offset = { 0 , 0 };	/* add to move pos */


neworigin( neworg )			/* set (0,0) for future moves */
	struct coords	neworg ;
	{
	offset = neworg ;
	}


movepen( pos )				/* move pen to pos + offset */
	struct coords	pos ;
	{
	pos.x = (tiglast.x = pos.x) + offset.x ;
	pos.y = (tiglast.y = pos.y) + offset.y ;
	putchar( 'M' );
	fwrite( &pos , sizeof(pos) , 1 , stdout );
	}


newcolor( color , bright )		/* set new color & brightness */
	register char	color , bright ;
	{
	putchar( 'C' );
	putchar( color );
	putchar( bright );
	}
