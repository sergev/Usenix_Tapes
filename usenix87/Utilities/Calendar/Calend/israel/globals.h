/* Copyright 1983 - University of Maryland */

#include <stdio.h>
#include <pwd.h>
#include <time.h>

#define NO	0
#define YES	1
#define RCFILE	".calrc"	/* user's rc file */
#define REMIND "/usr/local/bin/remind"

#define SWAP(a,b) { int temp; temp = a; a = b; b = temp; }

typedef	char	bool;
extern	int	errno;

char	line[BUFSIZ];		/* dates and string on rest of line */
int	lineptr;		/* current location in 'line' */
char	lastchar;		/* last character sent to 'yacc' */

int	type1, type2;		/* date types for first and second date,
				   -1 - *, 0 - day, 1 - date, 2 - date_list */
int	dtype;			/* day or date type for line, 0,1 as above */
int	star;			/* offset if relative dates */
int	month1[35],day1[35];	/* arrays for month and day elements */
int	monthmax, daymax;	/* max locations for days and months arrays */
int	month2, day2;		/* second day items */
int	tnum[2][35];		/* temp nums list */
int	tnummax[2];		/* max location for tnums */
int	cnum;			/* current number array */

int tdate;
char message[BUFSIZ];		/* buffer for creating the message */
char *mptr;			/* pointer into message array */
int dayt1, dayt2;
int offmonth, offday;		/* offsets for '+' option */
