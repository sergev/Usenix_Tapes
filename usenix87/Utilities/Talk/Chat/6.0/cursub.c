/* cursub.c :=: 9/25/85   By SLB */

/*  Various cursor subroutines to add nice touches to Chat */
#include "chat.h"

#define	NULLCHAR	((char *)0)
#define CR	015

char *BC, *CM, *CL, *CE, *SO, *SE;
char *CD, *TI, *TE;
int CO, LI;

char tspace[512], *bufspace;
char *tgetstr(), *getenv();
int putch();

cinit() 
{
	char bbuf[1024];
	char *tgoto();
	char *TERM;

	if ((TERM=getenv("TERM")) == FERROR)
		TERM = "dumb";

	tgetent (bbuf, TERM);
	bufspace = tspace;
	CM = tgetstr ("cm", &bufspace);
	CL = tgetstr ("cl", &bufspace);
	BC = tgetstr ("bc", &bufspace);
	if (BC == NULLCHAR || tgetflag("bs"))
		BC = "\b";
	CO = tgetnum ("co");
	LI = tgetnum ("li");
	SO = tgetstr ("so", &bufspace);
	SE = tgetstr ("se", &bufspace);
	CD = tgetstr ("cd", &bufspace);
	TI = tgetstr ("ti", &bufspace);
	TE = tgetstr ("te", &bufspace);

	if (SO == NULLCHAR)
	SO = "";
	if (SE == NULLCHAR)
	SE = "";
	if (CM == NULLCHAR) 
	return 1;
	return 0;
}

hilight(str)
char *str;
{
	tputs(SO, 0, putch);
	fputs(str, stdout);
	tputs(SE, 0, putch);
}

/*  Move cursor to specified spot.  (1,1) upper left.  (80,24) lower right */

mvcur(x, y)
int x, y; 
{
	tputs (tgoto (CM, x-1, y-1), 0, putch);
}

/* Clear screen */
clear() 
{
	int i;

	if (CL) {
		tputs(CL, 0, putch);
		putch(CR);
	}
	mvcur(1,1);
}

/* Erase current line */
zapline() 
{
	if (CE) {
		putch('\r');
		tputs(CE, 0, putch);
	} else
		puts("\r");
}
