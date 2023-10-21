#
#include "hd.h"
/*  Display and cursor control routines */

#define BELL	07

int putch ();

/*  Position cursor at the given (x, y) location.
	(1, 1) is the top left, (80, 24) is the lower right
 */

atxy (x, y)  int x, y; 
{

    tputs (tgoto (CM, x - 1, y - 1), 0, putch);
}

/*
 * Erase screen or upper directory window
 */
erase () 
{
    register int i;

    bufout();
    if (ewindow) {
	if (CB) {
		atxy(CO, window);
		tputs(CB, 0, putch);
	}
	else {
		atxy(1, 1);
		for (i = 1; i <= window; i++) {
			if (CE)
				tputs(CE, 0, putch);
			else
				clearline();
			printf("\r\n");
		}
	}
	atxy(1, 1);
    }
    else {
	tputs(CL, 0, putch);
	putch(CR);
    }
    unbufout();
}

/*
 * Erase below a line
 */
erasebelow(l)
register int l;
{
	if (CD) {
		atxy(1, l);
		tputs(CD, 0, putch);
		return;
	}
	do {
		atxy(1, l);
		clearline();
	} while (l++ < LI);
}

/* erase chars on current line */
clearline () 
{
    bufout ();
    if (CE) {
	putch(CR);
	tputs(CE, 0, putch);
    }
    else
	printf ("%c%80s%s", CR, "", UP);
    unbufout ();
}

/* Position cursor at specified file */
atfile (file, col) int file, col; 
{

    atxy (col, 3 + file);
}

/* buffering subroutines */

char outbuf [BUFSIZ];	/* the buffer */
int bcount = 0;	/* can buffer only once */
/* additional attempts ignored */

bufout () 
{

    if (bcount++ == 0) setbuf (stdout, outbuf);
}

unbufout () 
{

    fflush (stdout);
    if (--bcount == 0) setbuf (stdout, CNULL);
}

/* Clear message lines (23-24) for a pcount line message */
/* Lastcount keeps track of the lines with characters on them */
/* Dispdir calls with parameter -1 to reset lastcount */

clearmsg (pcount) int pcount; 
{

    static int lastcount;

    atxy(1, window-1);
    if (pcount == -1) lastcount = 0;
    else
    {
	if (lastcount == 0);
	else if (lastcount == 1) 
	{
	    clearline ();
	}
	else 
	{
	    bufout ();
	    if (CE) {
		putch(CR);
		tputs(CE, 0, putch);
		putch(LF);
		tputs(CE, 0, putch);
	    }
	    else
		printf ("%159s", "");
	    unbufout ();
	}
	lastcount = pcount;
    }
    atxy(1, window-1);
}

/* Putmsg counts the number of lines in its parameter,
   calls clearmsg with that count, and then displays the message.  */

putmsg (msg, a0, a1)
register char * msg; 
char *a0, *a1;
{

    clearmsg (any(LF, msg) == NULL ? 1 : 2);
    printf ("    ");
    hilite (msg, a0, a1);
    /* Cosmetic */
    if (*msg) {
    	while (*msg++);
	if (msg[-2] == ' ')
		printf("%s  ", BC);
    }
}

/* Beep bell on terminal */
beep () 
{
    putch (BELL);
}

/* Print error message about a file */
myperror (parm) char * parm; 
{

    extern int errno, sys_nerr;
    extern char *sys_errlist[];
    register char *c;

    c = "Unknown error";
    if(errno < sys_nerr)
	c = sys_errlist[errno];
    clearmsg (1);
    printf ("    ");
    hilite ("%s: %s", parm, c);
}
