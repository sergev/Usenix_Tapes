/*
 *	RT11 EMULATOR
 *	terminal i/o
 *
 *	Daniel R. Strick
 *	May 31, 1979
 *	7/17/81 -- uses definitions from "define.h" and returns error
 *		   codes with exit()
 *	8/30/81 -- now exits on tty EOF so we can feed command files to programs
 */

#include "define.h"

#define	STDIN	0
#define	STDOUT	1

#define	GCBSIZ	80
#define	PCBSIZ	90

static	char	ttinbuf[GCBSIZ];
static	char	*ttinxp;
static	char	*ttiend;

static	char	ttoutbuf[PCBSIZ];
static	char	*ttoutnxt	= ttoutbuf;


/*
 *  Get a character from the "console" terminal.
 *  Quit after and end-of-file.
 *  The putchr() buffer is flushed before every
 *  read from the standard input.
 */
getchr()
{
	register n;

	if(ttinxp >= ttiend) {
		flush();
		n = read (STDIN, ttinbuf, GCBSIZ);
		if (n < 0) {
			perror ("STDIN");
			exit(1);
		}
		if (n == 0) {
			printf ("^Z\n\n");
			exit (0);
		}
		ttinxp = ttinbuf;
		ttiend = &ttinbuf[n];
	}
	return (*ttinxp++ & 0377);
}


/*
 *  Write one character to the "console" terminal.
 *  Output is buffered; the buffer is flushed if
 *  it fills up, a newline is output, or getchr()
 *  needs to fill its input buffer.
 */
putchr (ac)
{
	register char c;

	c = ac;
	*ttoutnxt++ = c;
	if (c == '\n'  ||  c == '\r'  ||  ttoutnxt >= &ttoutbuf[PCBSIZ])
		flush();
	return (c);
}


/*
 *  Flush the "console" terminal output buffer.
 */
flush()
{
	if (write (STDOUT, ttoutbuf, ttoutnxt-ttoutbuf) < 0) {
		perror ("STDOUT");
		exit(1);
	}
	ttoutnxt = ttoutbuf;
}
