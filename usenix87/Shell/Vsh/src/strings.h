/*  Ascii control table.  There are 129 entries.  To find the class
    of a character, mask out the parity bit (if raw I/O is being
    used) and use it as an index the the ascii array.  With "cooked"
    I/O, EOF is -1.  EOF with "parity" masked out is 7f, which is also
    coded as an eof.  With "raw" I/O, EOT acts as an eof. */

/*	Each entry in the table has the following bits:
	1	End of line
	2	End of file
	4	Alpha
	8	Upper case
	16	Number
	32	Special character
	64	White space
*/
/*	Codes for loading table	*/

#define UD	0	/* Undefined--assorted control characters */
#define EL	1	/* End of line */
#define EF	3	/* End of File (and line) */
#define LA	4	/* Lower case alpha */ 
#define UA	12	/* Upper case alpha */
#define NU	16	/* Numeric */
#define	SC	32	/* Special character */
#define WS	64	/* White space */


/* Ascii acts like an array indexed from -1 to 128 */

extern char charclass [];
#define ascii	(charclass + 1)

/* Useful macro functions */

#define WHITESPACE(arg)	(ascii [arg] == WS)
#define	NUMERIC(arg)	(ascii [arg] == NU)
#define ENDLINE(arg)	(ascii [arg] & EL)

#define	TONUM(arg)	(arg - '0')

/* This reads a line from stdin and returns the first word */
#define getword(arg)	xgetword (arg, sizeof arg)

/* Read a line from stdin */
#define getline(arg)	xgetline (stdin, arg, sizeof arg)

/* Read a line from any stream */
#define fgetline(arg1,arg2)	xgetline (arg1, arg2, sizeof arg2)
