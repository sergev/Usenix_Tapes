#include "hd.h"
#include "strings.h"
#include "command.h"

static char EONE [] = "+1";	/* Default editor parameter */

/* Global data about file being shown */

FILE *sstream;	/* Stream being shown */

int cline;	/* Current (next) line to be shown */
int tline;	/* Top line of page */

char *sdesc [2] = 
{
    "Grep", "Make"
} ;

showerror () 
{
    return show (MAKEMODE);
}
showgrep () 
{
    return show (GREPMODE);
}

show (p) int p; 
{		/* Use parms GREPMODE or MAKEMODE */

    FILE *showopen ();
    register int cmd, next, number;
    int replot;
    int spaces;
    int (*cproc)();
    extern grep(), wmake(), showerror(), showgrep();

    sstream = showopen ("r", p);
    if (sstream == NULL) return NOREPLOT;

    cline = 1;
    next = REPLOT;
    replot = REPLOT;

    for (;;) 
    {
	if (next & REPLOT) 
	{
	    sdisp (p);
	    number = spaces = 0;
	}

	if (number == 0)  
	{
	    atxy (1, window-1);
	    printf ("  \r");
	}
	cmd = getch ();
	putch(cmd);
	next = REPLOT;

	/* This accumulates a number */
	if (NUMERIC (cmd) && spaces == 0) 
	{
	    number = number * 10 + TONUM (cmd);
	    next = 0;
	}
	else if (cmd == '\b' || cmd == 0177) 
	{
	    if (spaces > 0) --spaces;
	    else number /= 10;
	    if (cmd == 0177)
		printf("%s", BC);
	    printf (" %s", BC);
	    next = 0;
	}
	else if (cmd == '-' || cmd == 025) {
	    putch (CR);
	    number = cline-(window-7)*(number > 1 ? number : 2);
	    seekline (number < 0 ? 0 : number);
	    number = 0;
	}
	/* These commands use number */
	else if (cmd == LF || cmd == CR || cmd == 'e' || cmd == (PAGECHAR-'@')) 
	{
	    putch (CR);
	    next = sedit (p, number);
	    number = spaces = 0;
	}
	else if (cmd == ' ') 
	{
	    if (number > 0) ++spaces;
	    next = 0;
	}
	else if (cmd == 'p') 
	{
	    putch (CR);
	    seekline (number);
	}
	else
	{
	    number = spaces = 0; putch (CR);
	    cproc = cmdproc(cmd);
	    if (cproc == showerror || cproc == showgrep) 
	    {
		next = cmd | REPLOT;
		break;
	    }
	    else if (cmd == '?') 
	    {
		next = help (SHOWHELP);
	    }
	    else if (cproc == grep) 
	    {
		next = grep ();
		if (next != NOREPLOT) break;
	    }
	    else if (cproc == wmake) 
	    {
		next = wmake ();
		if (next != NOREPLOT) break;
	    }
	    else if (cmd == 'q' || cmd == EOT || cmd == (QUITCHAR - '@')) 
	    {
		break;
	    }
	    else
	    {
		if (any(cmd,
		"0123456789+L.\014\006\011\005\020\016\001\002\026hjkl")) {
			next = cmd;
			replot = NOREPLOT;
			dispdir(0);
			break;
		}
		next = command (cmd, SHOWCMD);
	    }
	    if (next & ENTERDIR)
		break;
	    if (next & REPLOT)
		seekline (tline);
	}
    }
    fclose (sstream);
    if (replot == NOREPLOT)
	next &= ~REPLOT;
    return next|replot;
}

/* Position show file at specified location */
seekline (line) int line; 
{

    register ch;

    cline = 1; rewind (sstream);
    while (cline < line && (ch = getc (sstream)) != EOF)
	if (ch == LF) cline++;

    if (feof (sstream)) 
    {
	if (cline <= 2) 
	{
	    rewind (sstream); cline = 1;
	}
	else 
	{
	    seekline (cline - 1);
	}
    }
}

sdisp (p)  int p; 
{	/* display sstream */

    register ch;	/* work char */
    int eline;	/* end line -- line to stop display */
    short lflag;	/* True if chars have printed on current line */
    register int col;
    extern char wdname [];	/* Name of working dir */

    if (feof (sstream)) seekline (1);

    tline = cline;
    eline = cline + window - 7;

    clearmsg (-1); bufout (); tty_push (COOKEDMODE);

    erase (); hilite("%s", wdname);
    atxy (61, 1); hilite("Showfile -- %s", sdesc [p]);
    printf("\r\n");

    while (cline < eline) 
    {
	lflag = 0;
	col = 0;
	do
	{
	    ch = getc (sstream);
	    if (ch == EOF) ch = LF;
	    if (ch == CR)
		continue;
	    else if (!lflag)  
	    {
		hilite ("%4d", cline);
		printf (" ");
		lflag = 1;
	    }
	    switch(ch) {
	    case '\t':
		col += 8 - (col&7);
		break;
	    case '\r':
		col = 0;
		break;
	    case '\b':
		if (col > 0)
		    col--;
		break;
	    case 0177:
		break;
	    default:
		if (ch >= ' ')
		    col++;
		break;
	    }
	    if (col >= CO-5) {
		printf("\r\n     ");
		eline--;
		col = 0;
	    }
	    putchar (ch);
	}
	while (ch != LF);

	cline++;
	if (feof (sstream)) 
	{
	    seekline (1); printf ("** End of File **\r\n");
	    break;
	}
    }
    printf ("\r\n     ");
    hilite ("Type the number of the line you wish to edit.");
    printf ("\r\n     ");
    hilite ("Type ^D to Leave.  Type ? for more options.");

    atxy (1, window-1); unbufout (); tty_pop ();
}

sedit (p, number) int p, number; 
{
    char inbuf [STRMAX];

    int oldline;

    oldline = cline;
    if (number == 0) return REPLOT;
    seekline (number);

    fgetline (sstream, inbuf);
    if (p == GREPMODE && grepfmt (inbuf));
    else if (p == MAKEMODE &&
	(ccfmt (inbuf) || grepfmt (inbuf) || cmdfmt (inbuf)));
    else
    {
	putmsg ("Cannot find file name");
	seekline (oldline);
	return NOREPLOT;
    }
    seekline (number);
    return REPLOT;
}

/* Extract file names and line numbers from various formats.  */

ccfmt (inbuf) char inbuf [STRMAX]; 
{	/*  Error from C compiler */
    char filebuf [STRMAX];	/*  or lint               */
    char *enumber ();
    register char *cpi, *cpo;

    cpi = inbuf;
    while (*cpi != '"') if (*cpi++ == 0) return 0;

    cpi++; cpo = filebuf;
    while (*cpi != '"') 
    {
	if (*cpi == 0) return 0;
	*cpo++ = *cpi++;
    }
    *cpo = 0;

    return nedit (filebuf, enumber (cpi));
}

grepfmt (inbuf) char inbuf [STRMAX]; 
{	/*  Line from grep        */
    char filebuf [STRMAX];	/*  or C preprocessor     */
    char *enumber ();
    register char *cpi, *cpo;

    cpi = inbuf; cpo = filebuf;
    while (*cpi != ':') 
    {
	if (*cpi == 0) return 0;
	*cpo++ = *cpi++;
    }
    if (*cpi == 0) return 0;
    *cpo = 0;

    return nedit (filebuf, enumber (cpi));
}

cmdfmt (inbuf) char inbuf [STRMAX]; 
{	/*  Command line          */
    char filebuf [STRMAX];
    register char *cpi, *cpo;

    cpi = inbuf;

    do
    {
	while (!WHITESPACE (*cpi)) if (*cpi++ == 0) return 0;
	while ( WHITESPACE (*cpi)) cpi++;
    }
    while (*cpi == '-');

    if (*cpi == 0) return 0;

    cpo = filebuf;
    while (!WHITESPACE (*cpi) && *cpi != 0) *cpo++ = *cpi++;
    *cpo = 0;

    return nedit (filebuf, EONE);
}

/* Extract number from line */

char *
    enumber (cpi) register char *cpi; 
{

    static char numbuf [8];

#define	NUMBUFLIM	(numbuf + sizeof numbuf)

    register char *cpo;
    cpo = numbuf; *cpo++ = '+';

    while (!NUMERIC (*cpi)) if (*cpi++ == 0) return EONE;

    while (NUMERIC (*cpi)) 
    {
	if (cpo >= NUMBUFLIM) return EONE;
	*cpo++ = *cpi++;
    }
    *cpo = 0;

    return numbuf;
}

nedit (file, number) char *file, *number; 
{

    if (access (file, 4)) return NOREPLOT;
    if (strcmp(EDITOR, "vi") && strcmp(EDITOR, "jove"))
	f_exec(EDITOR, EDITOR, file, 0);
    else
	f_exec (EDITOR, EDITOR, number, file, 0);
    return REPLOT;
}
