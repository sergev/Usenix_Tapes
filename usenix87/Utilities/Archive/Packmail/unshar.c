/****************************************************************
 * unshar.c: Unpackage one or more shell archive files
 *
 * Usage:	unshar [-c] [-o] [ -d directory ] [ file ] ...
 *
 * Description:	unshar is a filter which removes the front part
 *		of a file and passes the rest to the 'sh' command.
 *		It understands phrases like "cut here", and also
 *		knows about shell comment characters and the Unix
 *		commands "echo", "cat", and "sed".
 *		The -o flag causes it to preserve the header.
 *		With -O also the mail headers are preserved.
 *
 * HISTORY
 *  9-Feb-85  Andries Brouwer (aeb@mcvax) at CWI, Amsterdam
 *	Fixed filter mode; added -o and -O options; improved heuristics.
 *  6-Feb-85  Stephen C. Woods (scw@cepu) Los Angeles CA
 *	fixed '#' for v7 flavor systems (sh dosen't know about # comments)
 *	added -c flag to allow echo of skipped #comments
 *  1-Feb-85  Guido van Rossum (guido@mcvax) at CWI, Amsterdam
 *	Added missing 'quit' routine;
 *	added -d flag to change to directory first;
 *	added filter mode (read stdin when no arguments);
 *	added 'getopt' to get flags (makes it self-contained).
 * 29-Jan-85  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Created.
 ****************************************************************/

# include <stdio.h>
# define EOL '\n'

extern char *rindex ();		/* for SYSV: change to strrchr() */
extern char *optarg;
extern int  optind;
extern char *position ();

int     outheader = 0;
int     outall = 0;
#ifdef V7
int     cflag = 0;
#endif V7

main (argc, argv)
int     argc;
char   *argv[];
{
    register int    i,
                    ch;
    FILE * in;

 /* Process options */

    while ((ch = getopt (argc, argv, "cd:oO")) != EOF) {
	switch (ch) {
	    case 'd': 
		if (chdir (optarg) == -1) {
		    fprintf (stderr, "unshar: cannot chdir to '%s'\n", optarg);
		    exit (2);
		}
		break;
	    case 'O': 
		outall++;
	    /* fall into next case */
	    case 'o': 
		outheader++;
		break;
#ifdef V7
	    case 'c': 
		cflag++;
		if (cflag == 1)
		    break;
	    /* fall into next case */
	    default: 
		quit (2, "Usage: unshar [-c] [-o or -O] [-d directory] [input files]\n");
#else V7
	    default: 
		quit (2, "Usage: unshar [-o or -O] [-d directory] [input files]\n");
#endif V7
	}
    }

    if (optind < argc) {
	for (i = optind; i < argc; ++i) {
	    if ((in = fopen (argv[i], "r")) == NULL) {
		fprintf (stderr, "unshar: file '%s' not found\n", argv[i]);
		exit (1);
	    }
	    process (argv[i], in);
	    fclose (in);
	}
    }
    else
	process ((char *) 0, stdin);

    exit (0);
}


process (name, in)
char   *name;
FILE * in;
{
    char    ch;
    register char  *firstline;
    FILE * shpr, *popen ();

    if ((firstline = position (name, in)) != NULL) {
	printf ("%s:\n", name ? name : "standard input");
#ifdef V7
	{
	    int     inline,
	            comment;

	    ch = fgetc (in);

	    if ((inline = comment = (ch == '#'))) {
		fprintf (stderr, " skipping comments\n");
		if (cflag)
		    fputc (ch, stderr);
		do {
		    ch = fgetc (in);
		    if (ch == EOF) {
			fprintf (stderr, "file was all comments, nothing extracted\n");
			return;
		    }
		    if (inline) {
			inline = ch != '\n';
		    }
		    else {
			inline = comment = (ch == '#');
		    }
		    if (comment && cflag)
			fputc (ch, stderr);
		} while (comment);
	    }
	    ungetc (ch, in);
	}
#endif V7
	if ((shpr = popen ("sh", "w")) == NULL)
	    quit (1, "unshar: cannot open 'sh' process\n");
	fputs (firstline, shpr);
	while ((ch = fgetc (in)) != EOF)
	    fputc (ch, shpr);

	pclose (shpr);
    }
}

/****************************************************************
 * position: position 'fil' at the start of the shell command
 * portion of a shell archive file.
 ****************************************************************/

char   *
        position (fn, fil)
char   *fn;
FILE * fil;
{
    char    buf[BUFSIZ],
            bufl[BUFSIZ];
    FILE * hdr;
    register char  *xfn = fn ? fn : "the standard input";
    short   inmailheader;

    if (outheader) {
	char    hdrnam[15];
	if (fn) {
	    register char  *base = rindex (fn, '/');

	    strncpy (hdrnam, base ? base + 1 : fn, 14);
	    hdrnam[10] = 0;
	    strcat (hdrnam, ".hdr");
	}
	else {
	    printf ("Sending header to unshar.hdr\n");
	    strcpy (hdrnam, "unshar.hdr");
	}
	if ((hdr = fopen (hdrnam, "a")) == NULL) {
	    fprintf (stderr, "unshar: cannot open %s\n", hdrnam);
	    return (0);
	}
	inmailheader = 1;
    }

    while (1) {
    /* Read next line, fail if no more */
	if (fgets (buf, BUFSIZ, fil) == NULL) {
	    fprintf (stderr, "unshar: found no shell commands in %s\n", xfn);
	    return (0);
	}

    /* Bail out if we see C preprocessor commands or C comments */
	if (lookslikeC (buf)) {
	    fprintf (stderr,
		    "unshar: %s looks like raw C code, not a shell archive\n",
		    xfn);
	    return (0);
	}

	if (lookslikePASCAL (buf)) {
	    fprintf (stderr,
		    "unshar: %s looks like raw PASCAL code, not a shell archive\n",
		    xfn);
	    return (0);
	}

	if (lookslikeTROFF (buf)) {
	    fprintf (stderr,
		    "unshar: %s looks like raw TROFF input, not a shell archive\n",
		    xfn);
	    return (0);
	}

    /* Does this line start with a shell command or comment */
	if ((stlmatch (buf, "#") && !textline (buf)) || stlmatch (buf, ":") ||
		stlmatch (buf, "echo ") || stlmatch (buf, "sed ") ||
		stlmatch (buf, "cat ")) {
	    if (outheader)
		(void) fclose (hdr);
	    return (buf);
	}

    /* Does this line say "Cut here" */
    /* Of course we have to be a little careful: for example it says in a
       recent distribution: ... this program is cute, but there are ... */
	tolowercase (buf, bufl);
	if (stlmatch (buf, "--------") ||
		contains (bufl, "cut", "here") ||
		contains (bufl, "cut", "cut") ||
		contains (bufl, "tear", "here")) {
	/* Read next line after "cut here", skipping blank lines */
	    while (1) {
		if (fgets (buf, BUFSIZ, fil) == NULL) {
		    fprintf (stderr,
			    "unshar: found no shell commands after 'cut' in %s\n",
			    fn ? fn : "the standard input");
		    return (0);
		}

		if (*buf != '\n')
		    break;
	    }

	/* Win if line starts with a comment character of lower case
	   letter */
	    if (*buf == '#' || *buf == ':' || (('a' <= *buf) && ('z' >= *buf)))
		return (buf);

	/* Cut here message lied to us */
	    fprintf (stderr, "unshar: %s is probably not a shell archive,\n", fn);
	    fprintf (stderr, "        the 'cut' line was followed by: %s", buf);
	    return (0);
	}

	if (outheader) {
	    if (inmailheader && !mailheaderline (buf))
		inmailheader = 0;
	    if (!inmailheader || outall)
		fputs (buf, hdr);
	}
    }
}

/*****************************************************************
 * stlmatch  --  match leftmost part of string
 *
 * Usage:  i = stlmatch (big,small)
 *	int i;
 *	char *small, *big;
 *
 * Returns 1 iff initial characters of big match small exactly;
 * else 0.
 *
 * HISTORY
 * 18-May-82 Michael Mauldin (mlm) at Carnegie-Mellon University
 *      Ripped out of CMU lib for Rog-O-Matic portability
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Rewritten for VAX from Ken Greer's routine.
 *
 *  Originally from klg (Ken Greer) on IUS/SUS UNIX
 *****************************************************************/

int     stlmatch (big, small)
char   *small,
       *big;
{
    register char  *s,
                   *b;
    s = small;
    b = big;
    do {
	if (*s == '\0')
	    return (1);
    }
    while (*s++ == *b++);
    return (0);
}

/*****************************************************************
 * smatch: Given a data string and a pattern containing one or
 * more embedded stars (*) (which match any number of characters)
 * return true if the match succeeds, and set res[i] to the
 * characters matched by the 'i'th *.
 *****************************************************************/

smatch (dat, pat, res)
register char  *dat,
               *pat,
              **res;
{
    register char  *star = 0,
                   *starend,
                   *resp;
    int     nres = 0;

    while (1) {
	if (*pat == '*') {
	    star = ++pat;	/* Pattern after * */
	    starend = dat;	/* Data after * match */
	    resp = res[nres++];	/* Result string */
	    *resp = '\0';	/* Initially null */
	}
	else
	    if (*dat == *pat) {	/* Characters match */
		if (*pat == '\0')/* Pattern matches */
		    return (1);
		pat++;		/* Try next position */
		dat++;
	    }
	    else {
		if (*dat == '\0')/* Pattern fails - no more */
		    return (0);	/* data */
		if (star == 0)	/* Pattern fails - no * to */
		    return (0);	/* adjust */
		pat = star;	/* Restart pattern after * */
		*resp++ = *starend;/* Copy character to result */
		*resp = '\0';	/* null terminate */
		dat = ++starend;/* Rescan after copied char */
	    }
    }
}

/*****************************************************************
 * Addendum: quit subroutine (print a message and exit)
 *****************************************************************/

quit (status, message)
int     status;
char   *message;
{
    fprintf (stderr, message);
    exit (status);
}

/*****************************************************************
 * Public Domain getopt routine
 *****************************************************************/

/*
 * get option letter from argument vector
 */
int     optind = 1,		/* index into parent argv vector */
        optopt;			/* character checked for validity */
char   *optarg;			/* argument associated with option */

#define BADCH	(int)'?'
#define EMSG	""
#define tell(s)	fputs(*nargv,stderr);fputs(s,stderr); \
fputc (optopt, stderr); \
fputc ('\n', stderr); \
return (BADCH);

getopt (nargc, nargv, ostr)
int     nargc;
char  **nargv,
       *ostr;
{
    static char *place = EMSG;	/* option letter processing */
    register char  *oli;	/* option letter list index */
    char   *index ();

    if (!*place) {		/* update scanning pointer */
	if (optind >= nargc || *(place = nargv[optind]) != '-' || !*++place)
	    return (EOF);
	if (*place == '-') {	/* found "--" */
	    ++optind;
	    return (EOF);
	}
    }				/* option letter okay? */
    if ((optopt = (int) * place++) == (int) ':' ||
	    !(oli = index (ostr, optopt))) {
	if (!*place)
	    ++optind;
	tell (": illegal option -- ");
    }
    if (*++oli != ':') {	/* don't need argument */
	optarg = NULL;
	if (!*place)
	    ++optind;
    }
    else {			/* need an argument */
	if (*place)
	    optarg = place;	/* no white space */
	else
	    if (nargc <= ++optind) {/* no arg */
		place = EMSG;
		tell (": option requires an argument -- ");
	    }
	    else
		optarg = nargv[optind];/* white space */
	place = EMSG;
	++optind;
    }
    return (optopt);		/* dump back option letter */
}

/********************************************************************
 *
 * Differentiate mail headers from the rest of the article.
 *
 ********************************************************************/

mailheaderline (buf)
register char  *buf;
{
    register int    cnt = 0;

    if (!*buf || *buf == '\n')
	return (0);
    if (*buf == ' ' || *buf == '\t')
	return (1);
    while (letdig (*buf)) {
	buf++;
	cnt++;
    }
    return (cnt && *buf == ':');
}

letdig (c)			/* Symbols found in field tags, like 
				   Article-I.D.: */
char    c;
{
    return (c == '-' || c == '_' || c == '.' || digit (c) || letter (c));
}

/*
 * Some shar's produce output like:
 *	#	This is a shell archive.
 *	#	...
 *	#	---- cut here ----
 *	#!/bin/sh
 *
 * so if we haven't seen the 'cut' message yet, we have to skip #text lines.
 */
textline (buf)
register char  *buf;
{
    if (*buf == '#')
	buf++;
    while (textsym (*buf))
	buf++;
    return (!*buf);
}

textsym (c)
char    c;
{
    return (letter (c) ||
	    c == ' ' || c == '\t' || c == '\n' || c == ',' || c == '.');
}

/****************************************************************
 *
 * Recognize various types of files.
 *
 ****************************************************************/

lookslikeC (buf)
register char  *buf;
{
    return (stlmatch (buf, "#include") || stlmatch (buf, "# include") ||
	    stlmatch (buf, "#define") || stlmatch (buf, "# define") ||
	    stlmatch (buf, "#ifdef") || stlmatch (buf, "# ifdef") ||
	    stlmatch (buf, "#ifndef") || stlmatch (buf, "# ifndef") ||
	    stlmatch (buf, "/*")
	);
}

lookslikePASCAL (buf)
register char  *buf;
{
    return (stlmatch (buf, "(*"));
}

lookslikeTROFF (buf)
register char  *buf;
{
    return (*buf == '.' &&
	    letter (buf[1]) && letter (buf[2]) && !letter (buf[3]));
}

/************************************************************
 *
 * Some miscellaneous goodies
 *
 ************************************************************/

letter (c)
char    c;
{
    return (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'));
}

digit (c)
char    c;
{
    return ('0' <= c && c <= '9');
}

tolowercase (buf, bufl)
register char  *buf,
               *bufl;
{
    while (*buf) {
	if ('A' <= *buf && *buf <= 'Z')
	    *bufl++ = *buf++ + ('a' - 'A');
	else
	    *bufl++ = *buf++;
    }
    *bufl = 0;
}

/* check whether buf contains wd1 and wd2 as words
   (i.e., no preceding or following letters) */
contains (buf, wd1, wd2)
register char  *buf, *wd1, *wd2;
{
    register char  *wd = wd1;
    int     first = 1;

again: 
    while (*buf) {
	if (!letter (*buf)) {
	    buf++;
	    continue;
	}
	while (*buf++ == *wd++) {
	    if (!*wd) {
		if (!letter (*buf)) {
		    if (!first)
			return (1);
		    first = 0;
		    wd = wd2;
		    goto again;
		}
		break;
	    }
	}
	while (letter (*buf))
	    buf++;
	wd = first ? wd1 : wd2;
    }
    return (0);
}
