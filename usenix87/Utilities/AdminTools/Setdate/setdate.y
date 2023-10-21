/************************************************************************
 *									*
 *			Copyright (c) 1985, Fred Fish			*
 *			    All Rights Reserved				*
 *									*
 *	This software and/or documentation is released into the		*
 *	public domain for personal, non-commercial use only.		*
 *	Limited rights to use, modify, and redistribute are hereby	*
 *	granted for non-commercial purposes, provided that all		*
 *	copyright notices remain intact and all changes are clearly	*
 *	documented.  The author makes no warranty of any kind with	*
 *	respect to this product and explicitly disclaims any implied	*
 *	warranties of merchantability or fitness for any particular	*
 *	purpose.							*
 *									*
 ************************************************************************
 */


/*
 *  FILE
 *
 *	setdate.y
 *
 *  SCCS
 *
 *	@(#)setdate.y	1.1	2/10/85
 *
 *  DESCRIPTION
 *
 *	YACC grammar for a system bootup time/date set utility.
 *
 *	Accepts date and time in several common formats, for
 *	example:
 *
 *		January 18,1983  18:45
 *		Jan 18, 1983 18:45:00
 *		Jan 18, 1983 6:45 PM
 *		18:43  18-Jan-83
 *		01/18/83
 *		18:43
 *		8 AM
 *
 *  AUTHOR
 *
 *	Fred Fish
 *	(Currently at UniSoft Systems Inc.)
 *
 */

%{
#include <stdio.h>
#include <time.h>

#define SYS_ERROR (-1)		/* Returned by system calls on error */
#define FALSE (0)		/* Boolean false */
#define TRUE (1)		/* Boolean true */

static struct tm *newtime;	/* Time to set current time to */
static int morning = FALSE;	/* Explicit morning specified */
static int evening = FALSE;	/* Explicit evening specified */

extern long time ();		/* Get current clock value */
extern struct tm *localtime ();	/* Get local time from clock value */
extern char *asctime ();	/* Convert local time to string form */

%}

%token DIGITS MONTH SLASH DASH COMMA COLON AM PM

%%

when	:	date
	|	time
	|	date time
	|	time date
	|	/* empty */
	;

date	:	DIGITS SLASH DIGITS SLASH DIGITS
			{
				newtime -> tm_mon = $1 - 1;
				newtime -> tm_mday = $3;
				newtime -> tm_year = $5 % 100;
			}
	|	DIGITS DASH MONTH DASH DIGITS
			{
				newtime -> tm_mday = $1;
				newtime -> tm_mon = $3 - 1;
				newtime -> tm_year = $5 % 100;
			}
	|	MONTH DIGITS COMMA DIGITS
			{
				newtime -> tm_mon = $1 - 1;
				newtime -> tm_mday = $2;
				newtime -> tm_year = $4 % 100;
			} 
	|	MONTH DIGITS
			{
				newtime -> tm_mon = $1 - 1;
				newtime -> tm_mday = $2;
			} 
	;

time	:	clock
	|	clock period
	;

clock	:	DIGITS COLON DIGITS COLON DIGITS
			{
				newtime -> tm_hour = $1;
				newtime -> tm_min = $3;
				newtime -> tm_sec = $5;
			}
	|	DIGITS COLON DIGITS 
			{
				newtime -> tm_hour = $1;
				newtime -> tm_min = $3;
				newtime -> tm_sec = 0;
			}
	|	DIGITS
			{
				newtime -> tm_hour = $1;
				newtime -> tm_min = 0;
				newtime -> tm_sec = 0;
			}
	;

period	:	AM
			{
			    	morning = TRUE;
			}
	|	PM
			{
				evening = TRUE;
			}
	;
	
%%

int main (argc, argv)
int argc;
char *argv[];
{
    char buffer[64];		/* buffer to hold arg for date util */
    register char ch;		/* peek at first character of input */
    register int status;	/* system call result code */
    auto long systime;		/* Current system time */

    if (time (&systime) == SYS_ERROR) {
	fprintf (stderr, "%s: can't get current time: ", argv[0]);
	perror ("");
    }
    newtime = localtime (&systime);
    do {
	printf ("Enter current date & time (? for help) >> ");
	fflush (stdout);
	ch = getchar ();				/* Peek at first */
	if (ch == '?') {				/* Wants help? */
	    usage ();					/* Give good help */
	    while (getchar () != '\n') {;}		/* Flush line */
	} else {
	    ungetc (ch, stdin);				/* Put back peek */
	}
    } while (ch == '?');				/* Until no ? */
    yyparse ();
    if (morning) {
	newtime -> tm_hour %= 12;
    } else if (evening) {
	newtime -> tm_hour %= 12;
	newtime -> tm_hour += 12;
    }
#ifdef OLDFORMAT
    sprintf (buffer,"date %02.2d%02.2d%02.2d%02.2d%02.2d.%02.2d",
	newtime -> tm_year,
	newtime -> tm_mon + 1,
	newtime -> tm_mday,
	newtime -> tm_hour,
	newtime -> tm_min,
	newtime -> tm_sec);
#else
    sprintf (buffer,"date %02.2d%02.2d%02.2d%02.2d%02.2d",
	newtime -> tm_mon + 1,
	newtime -> tm_mday,
	newtime -> tm_hour,
	newtime -> tm_min,
	newtime -> tm_year);
#endif
    status = system (buffer);
    if (status == SYS_ERROR) {
	fprintf (stderr, "%s: can't set time: ", argv[0]);
	perror ("");
    }
    return (status);
}

static char *documentation[] = {
    "",
    "Date and time may be entered in any of several common formats.",
    "Default is current date and time.  Examples are:",
    "",
    "\tApril 14, 1983  13:41",
    "\t14-Apr-83  13:41:00",
    "\t4/14/83  13:41",
    "\t1:41 PM",
    "\tApr 14",
    "",
    "Acceptable but not recommended are:",
    "",
    "\tApr 14, 83",
    "\t14-April-1983",
    "\t4/14/1983",
    "",
    0,					/* MARKS END OF STRING LIST */
};

static usage()
{
    register char **dp;

    dp = documentation;			/* Init string pointer pointer */
    while (*dp) {			/* While another good pointer */
	printf ("%s\n", *dp++);		/* Print what it points to */
    }
}

yywrap ()				/* Thought this was in library */
{
    return (1);
}

#include "lex.c"			/* Output of "lex" from lex.l */

