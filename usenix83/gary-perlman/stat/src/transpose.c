/*		Copyright (c) 1982 Gary Perlman
This software may be copied freely provided:  (1) it is not used for
personal or material gain, and (2) this notice accompanies each copy.

Disclaimer:  No gauarantees of performance accompany this software,
nor is any responsbility assumed on the part of the author.  All the
software has been tested extensively and every effort has been made to
insure its reliability.*/

/* this program transposes the columns and rows of its ascii input */
#include <stdio.h>
#define copy(s) ((char *)strcpy((char *)malloc(sizeof(s)+1),s))
#define MAXCOLS 100
#define MAXLINES 100
#define MAXLEN  20
main (argc, argv) char **argv;
	{
	char	array[MAXCOLS][MAXLEN];
	char	*a[MAXLINES][MAXCOLS];
	int	c, l;
	int	ncols = 0;
	int	lines = 1;
	char	format[10];
	char	line[BUFSIZ];
	FILE	*ioptr = stdin;
	strcpy (format, "%s\t");
	for (c = 1; c < argc; c++)
		if (number (argv[c]) && access (argv[c], 4))
			{
			format[0] = '%';
			strcpy (format+1, argv[1]);
			strcat (format, "s ");
			}
		else if ((ioptr = fopen (argv[c], "r")) == NULL)
			eprintf ("%s: Can't open %s\n", argv[0], argv[c]);
	if (ioptr == stdin) checkstdin (argv[0]);
	if (fgets (line, BUFSIZ, ioptr))
		{
		ncols = sstrings (line, array, MAXCOLS, MAXLEN);
		if (ncols > MAXCOLS)
			eprintf ("%s: Too many columns (%d)\n", argv[0], ncols);
		if (ncols == 0)
			eprintf ("%s: Blank lines not allowed\n", argv[0]);
		for (c = 0; c < ncols; c++)
			a[0][c] = copy (array[c]);
		}
	else	eprintf ("%s: No input lines\n", argv[0]);
	while (fgets (line, BUFSIZ, ioptr))
		{
		if (lines == MAXLINES)
			eprintf ("%s: Too many lines (%d)\n", argv[0], lines);
		if (sstrings (line, array, MAXCOLS, MAXLEN) != ncols)
		    eprintf ("%s: Ragged input at line %d\n", argv[0], lines+1);
		for (c = 0; c < ncols; c++)
			a[lines][c] = copy (array[c]);
		lines++;
		}
	for (c = 0; c < ncols; c++)
		{
		for (l = 0; l < lines; l++)
			printf (format, a[l][c]);
		putchar ('\n');
		}
	}
