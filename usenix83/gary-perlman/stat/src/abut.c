/*	abut reads from its argument files, one line at a time
	per cycle, and prints all those lines on one line to
	the standard output. -- gary perlman
                  Copyright 1980  Gary Perlman.
Distribution of this program and/or any supporting documentation
for material or personal gain is prohibited.  Copies may be freely
distributed provided this copyright notice is included.

No guarantee of performance in made or this program, but it has
been extensively checked.  Comments and/or complaints are welcome
and should be directed to:  G. Perlman, Psychology C009, UCSD,
La Jolla, CA 92093.
*/
#include <stdio.h>
#include <ctype.h>
#define MAXFILES 20
#define FORMAT	"%s\t"  /* width of printed field */
char	format[100];
int	filenum;
int	nfiles = 0;
FILE	*ioptr[MAXFILES], *fopen ();
char	inline[BUFSIZ];
char	outline[BUFSIZ];
char	tmpline[BUFSIZ];
int	linelen;
int	done = 0;		/* true if output is to stop */
int	doneonce[MAXFILES];	/* true if [file] has been exhausted >= once */
int	numberlines = 0;	/* true if lines are to be numbered */
int	cycle;			/* true if abut should cycle through files */
				/* until all have been doneonce */
int	linenumber = 0;
main (argc, argv) char **argv;
	{
	int	arg;
	char	*ptr;
	if (argc == 1)
		eprintf ("abut: USAGE 'abut file1 file2 ...'\n");

	strcpy (format, FORMAT);
	for (arg = 1; arg < argc; arg++)
		{
		if (number (argv[arg]) && access (argv[arg], 4))
			{
			format[0] = '%';
			strcpy (format+1, argv[arg]);
			strcat (format, "s ");
			}
		else if (*argv[arg] == '-')
		    {
		    ptr = &argv[arg][1];
		    while (*ptr) switch (*ptr)
			{
			case 'n': numberlines = 1; ptr++; break;
			case 'c': cycle = 1; ptr++; break;
			default: eprintf ("abut: Unknown option %c.\n", *ptr);
			}
		    }
		else
		    {
		    if (nfiles == MAXFILES)
			eprintf ("abut: Maximum of %d files.\n", MAXFILES);
		    ioptr[nfiles] = fopen (argv[arg], "r");
		    if (ioptr[nfiles] == NULL)
			    eprintf ("abut: can't open %s\n", argv[arg]);
		    nfiles++;
		    }
		}
	while (!done)
		{
		*outline = NULL;
		if (numberlines) sprintf (outline, "%-4d ", ++linenumber);
		for (filenum = 0; filenum < nfiles; filenum++)
			{
			if (fgets (inline, BUFSIZ, ioptr[filenum]) == NULL)
				{
				doneonce[filenum] = 1;
				done = 1;
				rewind (ioptr[filenum]);
				fgets (inline, BUFSIZ, ioptr[filenum]);
				}
			linelen = strlen (inline) - 1;
			while (isspace (inline[linelen]))
				inline[linelen--] = NULL;
			sprintf (tmpline, format, inline);
			strcat (outline, tmpline);
			}
		for (filenum = 0; filenum < nfiles; filenum++)
			if (cycle && !doneonce[filenum]) done = 0;
			else if (!cycle && doneonce[filenum]) done = 1;
		if (!done) printf ("%s\n", outline);
		}
	}
