#include "hd.h"
#include "strings.h"

/* Table for scanner machine.  The machine can handle any regular
   expression.  Table elements come in pairs.  The first is
   the match character and the second is the next state assoc with
   that character.  The character '*' matches all character.
*/
static char st00[] = 
{
    ' ',  0, '\t', 0, '\n', 3, '\\', 1,
	'\'', 8, '"', 10, '*',  4
} ;
static char st01[] = 
{
    '\n', 0, '*',  4
} ;
static char st02[] = 
{
    '\n', 3, '*',  0
} ;
static char st03[] = 
{
    '*',  0
} ;
static char st04[] = 
{
    '*',  5
} ;
static char st05[] = 
{
    ' ',  2, '\n', 2, '\t', 2, '\\', 6, '\'', 8,
	'"', 10, '*',  4
} ;
static char st06[] = 
{
    '\n', 5, '*',  4
} ;
static char st07[] = 
{
    '*',  0
} ;
static char st08[] = 
{
    '\'', 5, '*', 9
} ;
static char st09[] = 
{
    '*',  8
} ;
static char st10[] = 
{
    '"',  5, '\\',12, '*',11
} ;
static char st11[] = 
{
    '*', 10
} ;
static char st12[] = 
{
    '\n',10, '*', 11
} ;

static char *lextab[] = 
{
    st00, st01, st02, st03, st04, st05, st06,
	st07, st08, st09, st10, st11, st12
} ;

/* Readarg reads a line from pstream and converts it into
   argv-argc format.  One may use a format similar to the shell.
*/
readarg (pstream, pline, pargc, pargv, pbuf)
FILE *pstream;	/* Stream being read */
int *pline;	/* Current line number */
int *pargc;	/* Count of words */
char *pargv [ARGVMAX];	/* Array of pointers to the words */
char pbuf [STRMAX];	/* Buf to hold the words themselves */
{
    /* Space is allocated from pargv and pbuf as words are read.
       These vars keep track of the allocations.
    */
    char **cargv = pargv;	/* Current allocations */
    char * cbuf  = pbuf;
    /* Limit of allocation */
    char **largv = pargv + ARGVMAX - 1;
    char * lbuf  = pbuf  + STRMAX - 1;

    /* Scanner vars */
    register int state = 0;	/* Scanner machine state */
    register int ch;	/* Current char */
    register char *ltp;	/* Lexical table pointer */

    int prompt;	/* Flag indicates when to prompt */
#define	NOPR	0	/* Don't */
#define	FPR	1	/* First prompt */
#define	CPR	2	/* Continuation prompt */

    /* Initialization */
    *pargc = 0;
    cargv[0] = pbuf;
    prompt = FPR;

    for (;;) 
    {
	switch (state) 
	{

	    /* Get a char */
	  case 0: 
	  case 1: 
	  case 5: 
	  case 6: 
	  case 8:
	  case 10: 
	  case 12:
	    if (prompt && pstream == stdin) {
		hilite((prompt == FPR) ? "Parameter:" : ":");
		printf(" ");
	    }
	    ch = fgetch (pstream);
	    prompt = (ch == LF) ? CPR : NOPR;
	    if (ch == (QUITCHAR-'@') || ascii [ch] == EF) 
	    {
		if (*pargc != 0) lderror
		    ("Incomplete line", *pline);
		return BAD;
	    }
	    else if (ENDLINE (ch)) ++*pline;
	    else if (ascii [ch] == UD) 
	    {
		lderror ("Non-Ascii character", *pline);
		continue;
	    }
	    break;

	    /* Store a char */
	  case 4: 
	  case 9: 
	  case 11:
	    if (cbuf >= lbuf || cargv >= largv) 
	    {
		lderror ("Too long", *pline);
		return BAD;
	    }
	    *cbuf++ = ch;
	    break;

	    /* Store a word */
	  case 2: 
	  case 7:
	    *cbuf++ = 0;
	    *++cargv = cbuf;
	    ++*pargc;
	    break;

	    /* Return from readarg */
	  case 3:
	    *cargv = CNULL; return GOOD;
	}
	for (ltp = lextab [state];
	*ltp != ch && *ltp != '*'; ltp += 2);
	state = *++ltp;
    }
}

/* Load error */
lderror (cp, line) char *cp; int line; 
{
    hilite ("Line %d:  %s\r\n", line, cp);
    getrtn ();
}
