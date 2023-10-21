/*		Copyright (c) 1982 Gary Perlman
This software may be copied freely provided:  (1) it is not used for
personal or material gain, and (2) this notice accompanies each copy.

Disclaimer:  No gauarantees of performance accompany this software,
nor is any responsbility assumed on the part of the author.  All the
software has been tested extensively and every effort has been made to
insure its reliability.*/

/* strings reads from ioptr into abase, an array of most maxstrings strings,
   at most maxstrings strings, each of length at most maxchars-1 chars.
   It returns the number of strings read in, or maxstrings + 1 if some
   information is discarded.
*/
#include <stdio.h>
#include <ctype.h>
#ifndef ESCAPE
#define	ESCAPE '\\'
#endif
fstrings (ioptr, abase, maxstrings, maxchars)
	FILE	*ioptr;
	char	*abase;
	{
	char	line[BUFSIZ];

	if (fgets (line, BUFSIZ, ioptr) == NULL) return (EOF);
	return (sstrings (line, abase, maxstrings, maxchars));
	}
sstrings (line, abase, maxstrings, maxchars)
	char	*line;
	char	*abase;
	{
	int	nstrings = 0;
	int	nchars;

	while (isspace (*line)) line++;
	while (*line)
		{
		nchars = 0;
		while (*line && !isspace (*line) && nchars<maxchars-1)
			if ((abase[nchars++] = *line++) == ESCAPE)
				abase[nchars-1] = *line++;
		abase[nchars] = NULL;
		abase += maxchars;
		while (*line && !isspace (*line)) line++;
		while (isspace (*line)) line++;
		if (++nstrings == maxstrings)
			return (maxstrings + (*line ? 1 : 0));
		}
	return (nstrings);
	}
