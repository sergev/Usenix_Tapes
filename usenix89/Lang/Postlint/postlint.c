/*OVERVIEW
	postlint [#context-lines] < lint-messages

	prints lint messages with context lines from file

	postlint reads lint output and reacts to two patterns:
		^name[:]$  sets current input file to "name"
				   if it can't find the name, it looks in LINTLIB
		([0-9]+)   line number + Context lines before are printed after message
*/

#ifdef	SYSV
# define	LINTLIB   "/usr/lib"         /* where lint types are kept */
#else
# define	LINTLIB   "/usr/lib/lint"    /* where lint types are kept */
#endif

#include <stdio.h>
#include <ctype.h>

#define	EOS  '\0'

char	Lintfile[BUFSIZ];    /* current file lint is talking about */
int 	Linecount = 0;       /* line number in Lintfile */
int 	Context = 2;         /* number of context lines to print */
FILE	*getioptr ();

/*FUNCTION main: loop through files in classic UNIX filter style */
main (argc, argv)
int 	argc;     /* argument count */
char	**argv;   /* argument vector */
	{
	if (argc > 1)
		{
		if (isdigit (argv[1][0]))
			Context = atoi (argv[1]);
		else
			fprintf (stderr, "Usage: postlint [number of context lines]\n");
		}
	
	dolines ();
	exit (0);
	}

/*FUNCTION dolines: process the lint error messages */
dolines ()
	{
	char	line[BUFSIZ];
	int 	lineno;
	char	*ptr;
	char	buf[BUFSIZ];     /* buffer to hold names */
	FILE	*ioptr = NULL;

	while (gets (ptr = line))
		{
		putchar ('\n');
		puts (line);
		if (onefield (line)) /* file name */
			{
			for (ptr = line; isspace (*ptr); ptr++)
				continue;
			(void) strcpy (buf, ptr);
			for (ptr = buf; *ptr && !isspace (*ptr) && *ptr != ':'; ptr++)
				continue;
			*ptr = EOS;
			ioptr = getioptr (buf, ioptr);
			}
		else
			for (ptr = line; *ptr; ptr++)
				if (*ptr == '(')
					{
					*ptr = EOS;
					if (lineno = atoi (ptr+1))
						{
						if (getname (line, ptr, buf))
							ioptr = getioptr (buf, ioptr);
						printlines (ioptr, lineno);
						}
					}
		}
	}


/*FUNCTION printlines: print the appropriate lines from source files */
#define	printit(line,lineno,what) printf ("%7d %-7s %s", lineno, what, line)
printlines (ioptr, lineno)
FILE	*ioptr;
int 	lineno;
	{
	char	line[BUFSIZ];
	int 	nlines = Context;    /* print this many lints before target */

	if (ioptr == NULL)
		return;

	if (!strncmp (LINTLIB, Lintfile, strlen (LINTLIB))) /* print one line */
		nlines = 0;

	if (lineno-nlines < Linecount) /* we are too far into the file */
		{
		rewind (ioptr);
		Linecount = 0;
		}

	while (fgets (line, BUFSIZ, ioptr))
		{
		Linecount++;
		if (Linecount == lineno)
			{
			printit (line, Linecount, ">>>");
			return;
			}
		else if (Linecount >= (lineno-nlines))
			printit (line, Linecount, "");
		}
	}

/*UTILITIES*/
/*FUNCTION getname: get last field on line and stuff in buf */
getname (line, eptr, buf)
char	*line, *eptr, *buf;
	{
	while (eptr > line && !isspace (*(eptr-1)) && *(eptr-1) != EOS)
		eptr--;
	(void) strcpy (buf, eptr);
	return (*eptr != EOS);
	}

/*FUNCTION getiooptr: gets new ioptr based on passed filename */
/*
	once this procedure is done,
		the global filename Lintfile is set
*/
FILE *
getioptr (filename, ioptr)
char	*filename;
FILE	*ioptr;
	{
	char	fullpath[BUFSIZ];
	FILE	*newioptr;

	if (!strcmp (filename, Lintfile)) /* same as current file */
		return (ioptr);
	if (*filename == EOS) /* no file? */
		return (ioptr);
	newioptr = fopen (filename, "r");
	if (newioptr == NULL)
		{
		(void) sprintf (fullpath, "%s/%s", LINTLIB, filename);
		newioptr = fopen (fullpath, "r");
		filename = fullpath;
		}
	if (newioptr != NULL)
		{
		(void) strcpy (Lintfile, filename);
		if (ioptr)
			(void) fclose (ioptr);
		}
	else /* leave thinsg as they were */
		newioptr = ioptr;
	Linecount = 0;
	return (newioptr);
	}

/*FUNCTION onefield: returns true if arg line has one name field */
onefield (line)
char	*line;
	{
	int 	alpha = 0;       /* number of alpha chars on line */
	int 	nonalpha = 0;    /* number of nonalpha chars on line */
	while (isspace (*line))
		line++;
	if (*line == EOS)
		return (0);
	while (*line && !isspace (*line))
		{
		if (isalpha (*line))
			alpha++;
		else
			nonalpha++;
		line++;
		}
	while (isspace (*line))
		line++;
	return (*line == EOS && alpha > nonalpha);
	}
