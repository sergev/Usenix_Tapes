180c
	putc( ch, &lp11 );
.
177a
	register ch;

	if ( (ch = c) == FORM )
		lp11.flag =| EJECT;	/* prevent double FF */
	else
		lp11.flag =& ~EJECT;

.
169d
164a
		for ( i = 0;  i < 2;  i++ )
			;		/* small delay for handshake */
	}
.
163c
	while (LPADDR->lpsr&DONE && (c = getc(&lp11)) >= 0)  {
.
161c
	register c, i;
.
133c
	case '\b':
.
129,130d
126a
	case '\n':
		lpoutput( c1 );
		lp11.mcc = 0;

.
115,125c
		if ( lp11.flag & EJECT )
			return;		/* already at top of page */
.
61c
	lp11.flag =& ~OPEN;
.
53c
	lp11.flag =| OPEN|MAPLC;
.
42d
39c
#define	CAP	01
.
36d
31,32c
	char	*cf;
	char	*cl;
.
21,22c
#define	MAXCOL	132
#define	MAPLC	0		/* set to 1 to map lowercase to UPPER */
.
5,8d
2a
 * lp.c - LP11 line printer driver
 *
 *	modified 26-May-1980 by D A Gwyn:
 *	1) adapt characteristics to Trilog (Printronix);
 *	2) remove indenting and page-seam skipping code;
 *	3) improve throughput.
.
w
q
