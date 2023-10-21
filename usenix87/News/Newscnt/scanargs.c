/* 
 * $Header: scanargs.c,v 1.3 83/05/22 02:21:32 thomas Exp $
 * 		Version 7 compatible
 * 	Argument scanner, scans argv style argument list.
 * 
 * 	Some stuff is a kludge because sscanf screws up
 * 
 * 	Gary Newman - 10/4/1979 - Ampex Corp. 
 * 
 * 	Modified by Spencer W. Thomas, Univ. of Utah, 5/81 to
 * 	add args introduced by 	a flag, add qscanargs call,
 * 	allow empty flags.
 * 
 * 	Compiling with QUICK defined generates 'qscanargs' ==
 * 	scanargs w/o floating point support; avoids huge size
 * 	of scanf.
 * 
 * 	If you make improvements we'd like to get them too.
 * 	Jay Lepreau	lepreau@utah-20, decvax!harpo!utah-cs!lepreau
 * 	Spencer Thomas	thomas@utah-20, decvax!harpo!utah-cs!thomas 
 * 
 *	(I know the code is ugly, but it just grew, you see ...)
 * 
 * Modified by:	Spencer W. Thomas
 * 	Date:	Feb 25 1983
 * 1. Fixed scanning of optional args.  Now args introduced by a flag
 *    must follow the flag which introduces them and precede any other
 *    flag argument.  It is still possible for a flag introduced
 *    argument to be mistaken for a "bare" argument which occurs
 *    earlier in the format string.  This implies that flags may not
 *    be conditional upon other flags, and a message will be generated
 *    if this is attempted.
 * 
 * 2. Usage message can be formatted by inserting newlines, tabs and
 *    spaces into the format string.  This is especially useful for
 *    long argument lists.
 * 
 * 3. Added n/N types for "numeric" args.  These args are scanned
 *    using the C language conventions - a number starting 0x is
 *    hexadecimal, a number starting with 0 is octal, otherwise it is
 *    decimal.
 */

#include <stdio.h>
#include <ctype.h>
#include <varargs.h>

typedef char bool;
/* 
 * An explicit assumption is made in this code that all pointers look
 * alike, except possible char * pointers.
 */
typedef int *ptr;

#define YES 1
#define NO 0
#define ERROR(msg)  {fprintf(stderr, "msg\n"); goto error; }

/* 
 * Storage allocation macros
 */
#define NEW( type, cnt )	(type *) malloc( (cnt) * sizeof( type ) )
#define RENEW( type, ptr, cnt )	(type *) realloc( ptr, (cnt) * sizeof( type ) )

static char * prformat();
static bool isnum();

/* 
 * Argument list is (argc, argv, format, ... )
 */
#ifndef	QUICK
scanargs ( va_alist )
#else
qscanargs ( va_alist )
#endif
va_dcl
{
    int     argc;			/* Actual arguments */
    char  **argv;
    char   *format;
    va_list argl;

    register    check;			/* check counter to be sure all argvs
					   are processed */
    register char  *cp;
    register    cnt;
    int	    optarg = 0;			/* where optional args start */
    int	    nopt;
    char    tmpflg,			/* temp flag */
	    typchr;			/* type char from format string */
    char    c;
    bool  * arg_used;			/* array of flags */
    char  * malloc();
    ptr	    aptr;			/* pointer to return loc */

    bool    required;
    int	    excnt;			/* which flag is set */
    bool    exflag;			/* when set, one of a set of exclusive
					   flags is set */

    bool    list_of;			/* set if parsing off a list of args */
    bool    comma_list;			/* set if AT&T style multiple args */
    int	  * cnt_arg;			/* where to stuff list count */
    int	    list_cnt;			/* how many in list */
    /* These are used to build return lists */
    char ** strlist;
    int   * intlist;
    long  * longlist;
    float * fltlist;
    double *dbllist;

    char   *ncp;			/* remember cp during flag scanning */
#ifndef	QUICK
    char   *cntrl;			/* control string for scanf's */
    char    junk[2];			/* junk buffer for scanf's */

    cntrl = "% %1s";			/* control string initialization for
					   scanf's */
#endif

    va_start( argl );
    argc = va_arg( argl, int );
    argv = va_arg( argl, char ** );
    format = va_arg( argl, char * );

    arg_used = NEW( bool, argc );
    if (arg_used == NULL)
    {
	fprintf(stderr, "malloc failed in scanargs, exiting\n");
	exit(-1);
    }
    else
    {
	for (cnt=0; cnt<argc; cnt++)
	    arg_used[cnt] = NO;
    }

    check = 0;
    cp = format;
    /* 
     * Skip program name
     */
    while ( *cp != ' ' && *cp != '\t' && *cp != '\n' && *cp != '\0' )
	cp++;

    while (*cp)
    {
	required = NO;			/* reset per-arg flags */
	list_of = NO;
	comma_list = NO;
	list_cnt = 0;
	switch (*(cp++))
	{
	    default: 			/* all other chars */
		break;
	    case ' ':			/* separators */
	    case '\t':
	    case '\n':
		optarg = 0;		/* end of optional arg string */
		break;

	    case '!': 			/* required argument */
		required = YES;
	    case '%': 			/* not required argument */
reswitch:				/* after finding '*' or ',' */
		switch (typchr = *(cp++))
		{
		    case ',':		/* argument is AT&T list of things */
			comma_list = YES;
		    case '*':		/* argument is list of things */
			list_of = YES;
			list_cnt = 0;	/* none yet */
			cnt_arg = va_arg( argl, int *);	/* item count * here */
			goto reswitch;	/* try again */

		    case '$':		/* "rest" of argument list */
			while ( argc > 1 && !arg_used[argc-1] )
			    argc--;	/* find last used argument */
			*va_arg( argl, int * ) = argc;
			break;

		    case '-': 		/* argument is flag */
			if (optarg > 0)
			    ERROR(Format error: flag conditional on flag not allowed);

		    /* go back to label */
			ncp = cp-1;	/* remember */
			cp -= 3;
			for (excnt = exflag = 0
				; *cp != ' ' && !(*cp=='-' &&(cp[-1]=='!'||cp[-1]=='%'));
				(--cp, excnt++))
			{
			    for (cnt = optarg+1; cnt < argc; cnt++)
			    {
			    /* flags all start with - */
				if (*argv[cnt] == '-' && !arg_used[cnt] &&
					!isdigit(argv[cnt][1]))
				    if (*(argv[cnt] + 1) == *cp)
				    {
					if (*(argv[cnt] + 2) != 0)
					    ERROR (extra flags ignored);
					if (exflag)
					    ERROR (more than one exclusive flag chosen);
					exflag++;
					required = NO;
					check += cnt;
					arg_used[cnt] = 1;
					nopt = cnt;
					*va_arg( argl, int *) |= (1 << excnt);
					break;
				    }
			    }
			}
			if (required)
			    ERROR (flag argument missing);
			cp = ncp;
			/* 
			 * If none of these flags were found, skip any
			 * optional arguments (in the varargs list, too).
			 */
			if (!exflag)
			{
			    va_arg( argl, int * );	/* skip the arg, too */
			    while (*++cp && ! isspace(*cp))
				if (*cp == '!' || *cp == '%')
				{
				    if ( *++cp == '*' || *cp == ',' )
				    {
					cp++;
					va_arg( argl, int * );
				    }
				    /* 
				     * Assume that char * might be a
				     * different size, but that all
				     * other pointers are same size.
				     */
				    if ( *cp == 's' )
					va_arg( argl, char * );
				    else
					va_arg( argl, ptr );
				}
			}
			else
			{
			    optarg = nopt;
			    cp++;	/* skip over - */
			}

			break;

		    case 's': 		/* char string */
		    case 'd': 		/* decimal # */
		    case 'o': 		/* octal # */
		    case 'x': 		/* hexadecimal # */
		    case 'n':		/* "number" in C syntax */
#ifndef	QUICK
		    case 'f': 		/* floating # */
#endif
		    case 'D': 		/* long decimal # */
		    case 'O': 		/* long octal # */
		    case 'X': 		/* long hexadecimal # */
		    case 'N':		/* long number in C syntax */
#ifndef	QUICK
		    case 'F': 		/* double precision floating # */
#endif
			for (cnt = optarg+1; cnt < argc; cnt++)
			{
			    ncp = argv[cnt];

			    if ( isnum( ncp, typchr, comma_list ) )
			    {
				if ( typchr == 's' )	/* string? */
				    continue;	/* don't want numbers, then */
			    }
			    else if ( *ncp == '-' )
				if ( optarg > 0 ) /* end optional args? */
				{
				    /* Eat the arg, too, if necessary */
				    if ( list_cnt == 0 )
					if ( typchr == 's' )
					    va_arg( argl, char * );
					else
					    va_arg( argl, ptr );
				    break;
				}
				else
				    continue;
			    else if ( typchr != 's' )
				continue;	/* not number, keep looking */
			    
			    /* 
			     * Otherwise usable argument may already
			     * be used.  (Must check this after
			     * checking for flag, though.)
			     */
			    if (arg_used[cnt]) continue;

			    /* 
			     * If it's a comma-and-or-space-separated
			     * list then count how many, and separate
			     * the list into an array of strings.
			     */
			    if ( comma_list )
			    {
				register char * s;
				int pass;

				/* 
				 * On pass 0, just count them.  On
				 * pass 1, null terminate each string 
				 */
				for ( pass = 0; pass <= 1; pass++ )
				{
				    for ( s = ncp; *s != '\0'; )
				    {
					if ( pass )
					    strlist[list_cnt] = s;
					while ( (c = *s) != '\0' && c != ' ' &&
						c != '\t' && c != ',' )
					    s++;
					if ( pass )
					    *s = '\0';

					list_cnt++;	/* count separators */
					/* 
					 * Two commas in a row give a null
					 * string, but two spaces
					 * don't.  Also skip spaces
					 * after a comma.
					 */
					if ( c != '\0' )
					    while ( *++s == ' ' || *s == '\t' )
						;
				    }
				    if ( pass == 0 )
				    {
					strlist = NEW( char *, list_cnt );
					list_cnt = 0;
				    }
				}
			    }
			    else if ( list_of )
				list_cnt++;   /* getting them one at a time */
			    /* 
			     * If it's either type of list, then alloc
			     * storage space for the returned values
			     * (except that comma-separated string
			     * lists already are done).
			     */
			    if ( list_of )
			    {
				if ( list_cnt == 1 || comma_list )
				    switch( typchr )
				    {
					case 's':
					    if ( !comma_list )
						strlist = NEW( char *, 1 );
					    aptr = (ptr) &strlist[0];
					    break;
					case 'n':
					case 'd':
					case 'o':
					case 'x':
					    intlist = NEW( int, list_cnt );
					    aptr = (ptr) &intlist[0];
					    break;
					case 'N':
					case 'D':
					case 'O':
					case 'X':
					    longlist = NEW( long, list_cnt );
					    aptr = (ptr) &longlist[0];
					    break;
					case 'f':
					    fltlist = NEW( float, list_cnt );
					    aptr = (ptr) &fltlist[0];
					    break;
					case 'F':
					    dbllist = NEW( double, list_cnt );
					    aptr = (ptr) &dbllist[0];
					    break;
				    }
				else
				    switch( typchr )
				    {
					case 's':
					    strlist = RENEW( char *, strlist,
							     list_cnt );
					    aptr = (ptr) &strlist[list_cnt-1];
					    break;
					case 'n':
					case 'd':
					case 'o':
					case 'x':
					    intlist = RENEW( int, intlist,
							     list_cnt );
					    aptr = (ptr) &intlist[list_cnt-1];
					    break;
					case 'N':
					case 'D':
					case 'O':
					case 'X':
					    longlist = RENEW( long, longlist,
							      list_cnt );
					    aptr = (ptr) &longlist[list_cnt-1];
					    break;
					case 'f':
					    fltlist = RENEW( float, fltlist,
							     list_cnt );
					    aptr = (ptr) &fltlist[list_cnt-1];
					    break;
					case 'F':
					    dbllist = RENEW( double, dbllist,
							     list_cnt );
					    aptr = (ptr) &dbllist[list_cnt-1];
					    break;
				    }
			    }
			    else
				aptr = va_arg( argl, ptr );

			    if ( typchr == 's' )
			    {
				if ( ! comma_list )
				    *(char **)aptr = ncp;
			    }
			    else
			    {
				nopt = 0;
				do {
				    /* 
				     * Need to update aptr if parsing
				     * a comma list
				     */
				    if ( comma_list && nopt > 0 )
				    {
					ncp = strlist[nopt];
					switch( typchr )
					{
					    case 'n':
					    case 'd':
					    case 'o':
					    case 'x':
						aptr = (ptr) &intlist[nopt];
						break;
					    case 'N':
					    case 'D':
					    case 'O':
					    case 'X':
						aptr = (ptr) &longlist[nopt];
						break;
					    case 'f':
						aptr = (ptr) &fltlist[nopt];
						break;
					    case 'F':
						aptr = (ptr) &dbllist[nopt];
						break;
					}
				    }
				    /* 
				     * Do conversion for n and N types
				     */
				    tmpflg = typchr;
				    if (typchr == 'n' || typchr == 'N' )
					if (*ncp != '0')
					    tmpflg = 'd';
					else if (*(ncp+1) == 'x' ||
						 *(ncp+1) == 'X')
					{
					    tmpflg = 'x';
					    ncp += 2;
					}
					else
					    tmpflg = 'o';
				    if (typchr == 'N')
					toupper( tmpflg );


#ifndef	QUICK
				    cntrl[1] = tmpflg;/* put in conversion */
				    if (sscanf (ncp, cntrl, aptr, junk) != 1)
					ERROR (Bad numeric argument);
#else
				    if (numcvt(ncp, tmpflg, aptr) != 1)
					ERROR (Bad numeric argument);
#endif
				} while ( comma_list && ++nopt < list_cnt );
			    }
			    check += cnt;
			    arg_used[cnt] = 1;
			    required = NO;
			    /*
			     * If not looking for multiple args,
			     * then done, otherwise, keep looking.
			     */
			    if ( !( list_of && !comma_list ) )
				break;
			    else
				continue;
			}
			if (required)
			    switch (typchr)
			    {
				case 'x': 
				case 'X': 
				    ERROR (missing hexadecimal argument);
				case 's': 
				    ERROR (missing string argument);
				case 'o': 
				case 'O': 
				    ERROR (missing octal argument);
				case 'd': 
				case 'D': 
				    ERROR (missing decimal argument);
				case 'f': 
				case 'F': 
				    ERROR (missing floating argument);
				case 'n':
				case 'N':
				    ERROR (missing numeric argument);
			    }
			if ( list_cnt > 0 )
			{
			    *cnt_arg = list_cnt;
			    switch ( typchr )
			    {
				case 's':
				    *va_arg( argl, char *** ) = strlist;
				    break;
				case 'n':
				case 'd':
				case 'o':
				case 'x':
				    *va_arg( argl, int ** ) = intlist;
				    break;
				case 'N':
				case 'D':
				case 'O':
				case 'X':
				    *va_arg( argl, long ** ) = longlist;
				    break;
				case 'f':
				    *va_arg( argl, float ** ) = fltlist;
				    break;
				case 'F':
				    *va_arg( argl, double **) = dbllist;
				    break;
			    }
			    if ( typchr != 's' )
				free( (char *) strlist );
			}
			else if ( cnt >= argc )
			{
			    /* Fell off end looking, so must eat the arg */
			    if ( typchr == 's' )
				va_arg( argl, char * );
			    else
				va_arg( argl, ptr );
			}
			break;
		    default: 		/* error */
			fprintf (stderr, "error in call to scanargs\n");
			return (0);
		}
	}
    }

    /*  Count up empty flags */
    for (cnt=1; cnt<argc; cnt++)
	if (argv[cnt][0] == '-' && argv[cnt][1] == '-' && argv[cnt][2] == 0
	    && !arg_used[cnt] )
	    check += cnt;

    /* sum from 1 to N = n*(n+1)/2 used to count up checks */
    if (check != (((argc - 1) * argc) / 2))
	ERROR (extra arguments not processed);

    free(arg_used);
    return (1);

error: 
    fprintf (stderr, "usage : ");
    if (*(cp = format) != ' ')
    {
	if ( *cp == '%' )
	{
	    /* 
	     * This is bogus, but until everyone can agree on a name
	     * for (rindex/strrchr) ....
	     */
	    for ( cp = argv[0]; *cp != '\0'; cp++ )
		;			/* find the end of the string */
	    for ( ; cp > argv[0] && *cp != '/'; cp-- )
		;			/* find the last / */
	    if ( *cp == '/' )
		cp++;
	    fprintf( stderr, "%s", cp );

	    cp = format + 1;		/* reset to where it should be */
	}
	while (putc (*cp++, stderr) != ' ');
    }
    else
	fprintf (stderr, "?? ");
    while (*cp == ' ')
	cp++;
    prformat (cp, NO);
    free(arg_used);
    return 0;
}

static char *
prformat (format, recurse)
char   *format;
{
    register char  *cp;
    bool    required, comma_list;
    int    list_of;

    cp = format;
    if (recurse)
	putc (' ', stderr);

    required = NO;
    list_of = 0;
    comma_list = NO;
    while (*cp)
    {
	switch (*cp)
	{
	    default: 
		cp++;
		break;
	    case ' ':
	    case '\t':
	    case '\n':
		putc(*cp, stderr);
		format = ++cp;
		break;
	    case '!': 
		required = YES;
	    case '%': 
reswitch:
		switch (*++cp)
		{
		    case ',':
			comma_list++;
		    case '*':
			list_of++;
			goto reswitch;

		    case '$':		/* "rest" of argument list */
			if (!required)
			    putc ('[', stderr);
			for (; format < cp - 1 - list_of; format++)
			    putc (*format, stderr);
			fputs( " ...", stderr );
			if ( !required )
			    putc( ']', stderr );
			break;

		    case '-': 		/* flags */
			if (!required)
			    putc ('[', stderr);
			putc ('-', stderr);

			if (cp - format > 2 + list_of)
			    putc ('{', stderr);
			cp = format;
			while (*cp != '%' && *cp != '!')
			    putc (*cp++, stderr);
			if (cp - format > 1 + list_of)
			    putc ('}', stderr);
			cp += 2;	/* skip !- or %- */
			if (*cp && !isspace(*cp))
			    cp = prformat (cp, YES);
					/* this is a recursive call */
			if (!required)
			    putc (']', stderr);
			break;
		    case 's': 		/* char string */
		    case 'd': 		/* decimal # */
		    case 'o': 		/* octal # */
		    case 'x': 		/* hexadecimal # */
		    case 'f': 		/* floating # */
		    case 'D': 		/* long decimal # */
		    case 'O': 		/* long octal # */
		    case 'X': 		/* long hexadecimal # */
		    case 'F': 		/* double precision floating # */
		    case 'n':		/* numeric arg (C format) */
		    case 'N':		/* long numeric arg */
			if (!required)
			    putc ('[', stderr);
			for (; format < cp - 1 - list_of; format++)
			    putc (*format, stderr);
			if ( list_of != 0 )
			{
			    if ( comma_list )
				putc( ',', stderr );
			    else
				putc( ' ', stderr );
			    fputs( "...", stderr );
			}
			if (!required)
			    putc (']', stderr);
			break;
		    default: 
			break;
		}
		required = NO;
		list_of = NO;
		comma_list = NO;
		if (*cp)		/* check for end of string */
		    format = ++cp;
		if (*cp && !isspace(*cp))
		    putc (' ', stderr);
	}
	if (recurse && isspace(*cp))
	    break;
    }
    if (!recurse)
	putc ('\n', stderr);
    return (cp);
}

/* 
 * isnum - determine whether a string MIGHT represent a number.
 * typchr indicates the type of argument we are looking for, and
 * determines the legal character set.  If comma_list is YES, then
 * space and comma are also legal characters.
 */
static bool
isnum( str, typchr, comma_list )
register char * str;
char typchr;
bool comma_list;
{
    register char * allowed, * digits, * cp;
    bool hasdigit = NO;

    switch( typchr )
    {
	case 'n':
	case 'N':
	    allowed = " \t,+-x0123456789abcdefABCDEF";
	    break;
	case 'd':
	case 'D':
	    allowed = " \t,+-0123456789";
	    break;
	case 'o':
	case 'O':
	    allowed = " \t,01234567";
	    break;
	case 'x':
	case 'X':
	    allowed = " \t,0123456789abcdefABCDEF";
	    break;
	case 'f':
	case 'F':
	    allowed = " \t,+-eE.0123456789";
	    break;
	case 's':			/* only throw out decimal numbers */
	default:
	    allowed = " \t,+-.0123456789";
	    break;
    }
    digits = allowed;
    while ( *digits != '0' )
	digits++;
    if ( ! comma_list )
	allowed += 3;		      /* then don't allow space, tab, comma */

    while ( *str != '\0' )
    {
    	for ( cp = allowed; *cp != '\0' && *cp != *str; cp++ )
    	    ;
    	if ( *cp == '\0' )
	    return NO;		     /* if not in allowed chars, not number */
	if ( cp - digits >= 0 )
	    hasdigit = YES;
	str++;
    }
    return hasdigit;
}

#ifdef	QUICK
numcvt(str, conv, val)
register char *str;
char conv;
int *val;
{
    int base, neg = 0;
    register unsigned int d;
    long retval = 0;
    register char *digits;
    extern char *index();
    if (conv == 'o' || conv == 'O')
	base = 8;
    else if (conv == 'd' || conv == 'D')
	base = 10;
    else if (conv == 'x' || conv == 'X')
	base = 16;
    else
	return 0;

    if (*str == '-')
    {
	neg = 1;
	str++;
    }
    while (*str)
    {
	if (*str >= '0' && *str < '0'+base)
	    d = *str - '0';
	else if (base == 16 && *str >= 'a' && *str <= 'f')
	    d = 10 + *str - 'a';
	else if (base == 16 && *str >= 'A' && *str <= 'F')
	    d = 10 + *str - 'A';
	else
	    return 0;
	retval = retval*base + d;
	str++;
    }
    if (neg)
	retval = -retval;
    if (conv == 'D' || conv == 'O' || conv == 'X')
	*(long *) val = retval;
    else
	*val = (int) retval;
    return 1;
}
#endif	QUICK
