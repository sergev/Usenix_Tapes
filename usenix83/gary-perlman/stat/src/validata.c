/*		Copyright (c) 1982 Gary Perlman
This software may be copied freely provided:  (1) it is not used for
personal or material gain, and (2) this notice accompanies each copy.

Disclaimer:  No gauarantees of performance accompany this software,
nor is any responsbility assumed on the part of the author.  All the
software has been tested extensively and every effort has been made to
insure its reliability.*/

/* validata looks at a data file to see what sort of columns it has.
	It complains if there are not an equal number of columns
	for each line in the input.  At the end, it prints the
	type of each column depending on what types of data appeared
	in them.
*/
#include <stdio.h>
#include <ctype.h>
#define	MAXCOLS		100
#define	MAXSTRING	50
int	nalnum[MAXCOLS];
int	nalpha[MAXCOLS];
int	nint[MAXCOLS];
int	nfloat[MAXCOLS];
int	nother[MAXCOLS];
int	ntotal[MAXCOLS];
main (argc, argv) int argc; char **argv;
    {
    char	line[BUFSIZ];
    char	col[MAXCOLS][MAXSTRING];
    FILE	*ioptr;
    int	other;
    int linecount = 0;
    int ncols;
    int maxcols = 0;
    int	colcount;
    int colno;
    char *s;
    if (argc == 1)
	{
	checkstdin (argv[0]);
	ioptr = stdin;
	}
    else if (argc > 2)
	{
	fprintf (stderr,"%s: Can only %s 1 file at a time\n", argv[0], argv[0]);
	}
    else if ((ioptr = fopen (argv[1], "r")) == NULL)
	{
	fprintf (stderr, "%s: Can't open %s\n", argv[0], argv[1]);
	exit (1);
	}
    while (fgets (line, BUFSIZ, ioptr))
	{
	linecount++;
	ncols = sstrings (line, col, MAXCOLS, MAXSTRING);
	if (ncols > maxcols) maxcols = ncols;
	if (ncols != colcount)
	    {
	    if (linecount != 1)
		printf ( "Variable number of columns at line %d.\n", linecount);
	    colcount = ncols;
	    }
	if (ncols == 0)
	    printf ("Line %d is empty.\n", linecount);
	other = 1;
	for (colno = 0; colno < ncols; colno++)
	    {
	    s = col[colno];
	    ntotal[colno]++;
	    if (stralnum (s))
		{
		other = 0;
		nalnum[colno]++;
		if (stralpha (s))
		    nalpha[colno]++;
		}
	    if (strfloat (s))
		{
		other = 0;
		nfloat[colno]++;
		if (strint (s))
		    nint[colno]++;
		}
	    if (other) nother[colno]++;
	    else other = 1;
	    }
	}

    if (ioptr != stdin) fclose (ioptr);
    printf ("%d lines read\n", linecount);
    printf ("%10s%10s%10s%10s%10s%10s%10s\n",
	"Column", "total", "alphanum", "alpha", "integer", "float", "other");
    for (colno = 0; colno < maxcols; colno++)
    printf ("%10d%10d%10d%10d%10d%10d%10d\n",
	colno+1,  ntotal[colno],
        nalnum[colno], nalpha[colno], nint[colno],
	nfloat[colno], nother[colno]);
    }

stralnum (s) char *s;
	{
	while (isspace (*s)) s++;
	while (*s)
		if (!isalnum (*s)) return (0);
		else s++;
	return (1);
	}

stralpha (s) char *s;
	{
	while (isspace (*s)) s++;
	while (*s)
		if (!isalpha (*s)) return (0);
		else s++;
	return (1);
	}

strint (s) char *s;
	{
	while (isspace (*s)) s++;
	if (*s == '+' || *s == '-') s++;
	while (*s)
		if (!isdigit (*s)) return (0);
		else s++;
	return (1);
	}

strfloat (s) char *s;
	{
	while (isspace (*s)) s++;
	if (!*s) return (0);
	if (*s == '+' || *s == '-') s++;
	while (isdigit (*s)) s++;
	if (*s == '.') s++;
	while (isdigit (*s)) s++;
	if (*s == 'E' || *s == 'e')
		{
		s++;
		if (*s == '+' || *s == '-') s++;
		while (isdigit (*s)) s++;
		}
	while (isspace (*s)) s++;
	return (*s == NULL);
	}
