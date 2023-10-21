#
/* Useful string handling routines */
#
#include "hd.h"
#include "strings.h"

#define	ALARMARROW			/* choose ALARM or EMPTY code */

#ifdef VT100ARROW
#ifdef ALARMARROW			/* Use alarm(2) implementation */
#include <setjmp.h>
#include <signal.h>
static int caught = 0;
jmp_buf arrowsave;
int arrowcatch();
#endif
#ifndef EMPTY
#ifdef V7TTY
#ifdef VENIX
#include <sgtty.h>
#else
#include <sys/ioctl.h>
#endif
#endif
#endif
#endif

/* Scanspace reads in the standard input and returns the first character
   which is not a space.  Note this could be an end of file.  */

scanspace () 
{
    register ch;

    while (WHITESPACE (ch = getch ()));
    return ch;
}

/* Scanend scans to the end of a line */

scanend () 
{
    register ch;

    while (!ENDLINE (ch =  getch ()));
    return ch;
}

/* Xgetword inputs a string up to clim - 1 chars long */
/* String is returned in argument;  Length is return value.  */

/* Use getword (char_array) instead.  Clim is filled in for you. */

xgetword (input, clim) char *input; int clim; 
{

#define LASTCHAR	(input + clim - 1)

    register ch; register char *cp, *lchar;

    ch = scanspace ();
    cp = input; lchar = LASTCHAR;

    while (cp < lchar &&
	!ENDLINE (ch) &&
	!WHITESPACE (ch)) 
    {

	*cp++ = ch; ch = getch ();
    }

    while (!ENDLINE (ch)) ch = getch ();
    *cp = 0;
    return (cp - input);
}

/* Xgetline inputs a string up to clim - 1 chars long */
/* String is returned in argument;  Length is return value.  */

/* Use getline (char_array) instead.  Clim is filled in for you. */
/* Use fgetline (stream, char_array) to get a line from a file   */
/* other than stdin. */

xgetline (stream, input, clim) FILE *stream; char *input; int clim; 
{

    register ch; register char *cp, *lchar;
    cp = input; lchar = LASTCHAR;

    do
    {
	ch = fgetch(stream);
	if (cp > lchar) cp = lchar;
	*cp++ = ch;
    }
    while (!ENDLINE (ch));

    *--cp = 0;
    return (cp - input);
}

getrtn () 
{		/* Ask person to press return */
    printf ("  ");
    hilite ("Press");
    printf (" -Return-");
    scanend ();
}

putch (ch) int ch; 
{
    putchar (ch);
}

/*
 * Added to patch anomaly with fgetc returning EOF
 *	probably caused by switch between CBREAK and cooked modes
 */
fgetch(stream)
FILE *stream;
{
	if (stream == stdin)
		return getch();
	else
		return fgetc(stream);
}

#ifndef VT100ARROW

getch () 
{
    char c;
    static int errcnt = 0;

    if (read(0, &c, 1) != 1) {
	if (errcnt++ > 10) {
		printf(" (\07Input error)\r\n\n");
		leave();
	}
	c = QUITCHAR-'@';
    }
    else
	errcnt = 0;
    return (0177&c);
}

#else
#define NEMPTY 500	/* Yuck! Basically a timed loop. Make this larger for
				machines faster than a 780 and smaller for
				machines slower than a 750. Better yet, rewrite
				this code to use the termcap(5) arrow key
				definitions and select(2) for 4.2BSD or MINTIME
				for System V. Unfortunately, it will still leave
				other machines "out in the cold." */
getch () 
{
    register int i, j;
    char c;
    static int errcnt = 0;
    static char peek[2] = { 0377, 0377 };

    if ((peek[0]&0377) != 0377) {
	c = peek[0]&0177;
	peek[0] = 0377;
    }
    else if ((peek[1]&0377) != 0377) {
	c = peek[1]&0177;
	peek[1] = 0377;
    }
    else if (read(0, &c, 1) != 1) {
	if (errcnt++ > 10) {
		printf(" (\07Input error)\r\n\n");
		leave();
	}
	c = QUITCHAR-'@';
    }
    else {
	errcnt = 0;
	c &= 0177;
	if (c == 033) {
		peek[0] = peek[1] = 0377;
#ifdef ALARMARROW
		i = caught = 0;
		setjmp(arrowsave);
		if (!caught) {
			signal(SIGALRM, arrowcatch);
			/* If your system doesn't guarantee at least .1 seconds
				use alarm(2) (or nap(.1 second)) */
			alarm(1);
			i = (read(0, &peek[0], 1) == 1) ? 1 : 0;
			if (i) {
				peek[0] &= 0177;
				if (read(0, &peek[1], 1) == 1) {
					peek[1] &= 0177;
					i++;
				}
			}
			alarm(0);
		}
		signal(SIGALRM, SIG_IGN);
#else
#ifdef nokludge
		if (!empty(0)
			sleep(1);
		if (!empty(0)) {
			i = read(0, peek, 2);
#else
		/* empty() loop kludge */
		for (j = 0; j < NEMPTY; j++)
			if (!empty(0)) {
				if (i = (read(0, &peek[0], 1) == 1) ? 1 : 0)
					peek[0] &= 0177;
				for (j = 0; j < NEMPTY; j++)
					if (!empty(0)) {
						if (read(0, &peek[1], 1) == 1) {
							peek[1] &= 0177;
							i++;
						}
						break;
					}
				break;
			}
#endif
#endif
		if (i == 2 && (peek[0] == 'O' || peek[0] == '['))
			switch(peek[1]) {
				case 'A':
					c = 020;	/* up */
					peek[0] = peek[1] = 0377;
					break;
				case 'B':
					c = 016;	/* down */
					peek[0] = peek[1] = 0377;
					break;
				case 'C':
					c = 06;		/* right */
					peek[0] = peek[1] = 0377;
					break;
				case 'D':
					c = 02;		/* left */
					peek[0] = peek[1] = 0377;
					break;
			}
	}
    }
    return c;
}

#ifdef ALARMARROW
arrowcatch()
{
	signal(SIGALRM, arrowcatch);
	caught = 1;
	longjmp(arrowsave);
}
#else
#ifndef EMPTY
#ifdef VENIX

/*
 * VENIX empty call - check if tty has any characters in input queue
 */

empty(fd)
int fd;
{
	struct sgttyb sg;

	ioctl(fd, TIOCQCNT, &sg);
	return (sg.sg_ispeed == 0);
}

full(fd)
int fd;
{
	struct sgttyb sg;

	ioctl(fd, TIOCQCNT, &sg);
	return sg.sg_ispeed;
}

#else

/*
 * empty(fildes) - For BSD systems
 * Is the tty or pipe empty ? (Will a read() block)
 */

empty(fd)
int fd;
{
	long nchar;
	int f;

	f = ioctl(fd, FIONREAD, &nchar);
	if (f == -1)
		return -1;
	if (nchar)
		return 0;
	return 1;
}

#endif
#endif
#endif
#endif

any(c, s)
register char c, *s;
{

	while(*s)
		if(c == *s++)
			return(1);
	return(0);
}
