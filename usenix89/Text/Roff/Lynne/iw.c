/* iw.c

	Copyright (c) 1987 by Stuart Lynne

	Permission is hereby granted to use, distribute, modify or copy
	except for profit, providing this copyright notice and disclaimer
	is included.

	Version 1.0 - January 11, 1987
		Cleaned up from initial hacking.

	Bug reports and comments to:

		sl@van-bc.uucp
		ihpn4!alberta!ubc-vision!van-bc!sl

	

	iw is a filter for nroff output to be sent to the ImageWriter
	printer.

	It can be used as a filter for output from nroff as driven by three
	different tables with two different schema's for underline / boldface:

			Boldface	Underline  	Tab's	
	tablp 	 	C^hC		_^hC		^I
	tabiw 		<esc>!C<esc>"	<esc>XC<esc>Y	^I
	tabiwp		<esc>!C<esc>"	<esc>XC<esc>Y	<esc>'~<esc>$


	tabiwp uses the ~ in the custom font as a 1 dot width space char.

	Make
		make iw

	Usage:
		... | iw | ....
		iw < file.in > file.out
		nroff -Tiwp file | col -x -p | iw | ...
		nroff -Tlp file | col -x -p | iw | ....
*/

#include	<stdio.h>
#include	<ctype.h>

#define	FALSE	0
#define TRUE	1

#define	FIRSTCOL	0

main () 
{
	int	nextch = 0;		/* next character in input stream */
	int	lastch = 0;		/* last character (next to be printed) */
	
	int	boldface = FALSE;	/* do we want boldface ? Used with C^HC */
	int	sentboldface = FALSE;	/* have we sent boldface escape sequence */
	int	underline = FALSE;	/* do we want underline? Used with _^HC */
	int	sentunderline = FALSE;	/* have we sent an underline escape sequence */
	int	overstrike = FALSE;	/* is this an overstrike? Used with X^HY */
	int	seenunderline = FALSE;	/* have we seen an underline escape sequence */

	int	count;			/* counter for ignoring multiple backspaces */

	int	column = FIRSTCOL;	/* current column, for tabbing */
	int	newcolumn = FIRSTCOL;	/* where we should go, for tabbing */

	int	haveesc = FALSE;	/* flag for watching for escape seq. */


	/* create a special character for nroff to use, a single dot width
	   space. This allows proportional spacing to work. See tabiwp.c for
	   usage.
	*/

	putchar( '\033' );		/* escape - allows creation of */
	putchar( '-' );			/* 	custom char's */
	putchar( '\033' );		/* escape I sets to 8 bit size */
	putchar( 'I' );
	putchar( '~' );			/* use ~ to signal this char */
	putchar( 'A' );			/* width code */
	putchar( '\000' );		/* space is no bits set */
	putchar( '\004' );		/* end of character definition data */
	
	do {
		/* are we counting spaces for a tab. Unfortunately the
		   imagewriter and nroff disagree slightly on tab counting
		   if escape char's are in line (nroff ignores, printer 
		   doesn't).

		   We either get a char or provide a space.

		*/
		if ( column < newcolumn )
			nextch = ' ';
		else {
			newcolumn = FIRSTCOL;
			nextch = getchar();
		}

		/* are we in an escape sequence we care about (ignore the rest)
		   <esc>X and <esc>Y are underline start and stop, set underline
		   flag and go to beginning of loop.
 		*/
		if ( haveesc ) { 
			switch ( nextch ) {
			case 'X':
				seenunderline = TRUE;
				lastch = 0;
				continue;
			case 'Y':
				seenunderline = FALSE;
				lastch = 0;
				continue;
			}
			haveesc = FALSE;
		}
		else 
		/* check if next character is backspace for special handling 
		*/
		    if ( nextch == '\b' ) {
			nextch = getchar();
			
			/* nroff use's _^hX to underline a char for tablp
			*/
			if ( lastch == '_' ) {
				underline = TRUE;
				lastch = nextch;
				nextch = 0;
				continue;
			}
			/* nroff use's X^hX to boldface a char for tablp
			*/
			else if ( nextch == lastch ) {
				boldface = TRUE;
				continue;
			}
			/* nroff sometimes uses XXXXX^h^h^h^h^hYYYYYY
			   type of sequence. Imagewriter can't handle
		           so we ignore.
			   I think it is from combination of bold/italic.
			*/
			else if ( nextch == '\b' ) {
				if ( lastch )
					putchar( lastch );
				while (( nextch = getchar()) == '\b' ) {
					count++;
					column--;
				}
				lastch = 0;
				/*nextch = 0;*/
				count += 2;
			}
			/* ok, ok, we'll let a simple overstrike get through
			*/
			else
				overstrike = TRUE;
		}
		/* check for some special control chars
		*/	
		else switch ( nextch ) {

		case '\n':
			newcolumn = column = FIRSTCOL;
			break;

		case '\t':
			newcolumn = (column | 0x7) + 1;
			nextch = 0;
			break;

		case '\033':
			haveesc = TRUE;
			break;
			
		default :
			column++;
			break;
		}

		/* now we have check for various combinations of control
		   chars and are ready to print the char. First check to see
		   if we need to turn underline or boldface on or off.

		   Unfortunately the imagewriter auto underline also underlines
 		   spaces, which would be alright except that nroff assumes 
		   that spaces are not underlined. And consequently does not
		   turn off underlining at the end of a line. So even though
		   we use underline sequence direct from tabiw(p) we intercept
		   and regen around whitespace.
		*/

		if ( seenunderline && isalpha(lastch) )
				underline = TRUE;

		if ( (underline && !sentunderline) ) {
			printf( "\033X" );
			sentunderline = TRUE;
		}
		else if ( !underline && sentunderline ) {
			printf( "\033Y");
			sentunderline = FALSE;
		} 
		if ( boldface && !sentboldface ) {
			printf( "\033\!" );
			sentboldface = TRUE;
		}
		else if ( !boldface && sentboldface ) {
			printf( "\033\"" );
			sentboldface = FALSE;
		}

		/* to be able to properly handle _ or _____ we must
		   do a one char lookahead. So we save char until next
		   time around the loop. Always print previous char.
	
		   Also sometimes must simply throw away due to imagewriter
		   being unable to backup more than one position.
		*/
		if ( lastch )
			if ( count )
				count--;
			else 
				putchar( lastch );

		lastch = nextch;

		/* finally check for overstrike and perform if neccessary
		*/	
		if ( overstrike ) {
			printf( "\b%c", nextch );	
			lastch = 0;
		}

		/* turn off flags for next go around */	
		boldface = overstrike = underline = FALSE;

	}
	/* termination condition */
	while ( nextch != EOF);
}
	

