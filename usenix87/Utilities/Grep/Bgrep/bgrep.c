/* bgrep --- grep using Boyer-Moore pattern matcher */

/*
 * All the wrapper code (argument and file handling, printing, etc.) by
 * Arnold Robbins, gatech!arnold
 *
 * Boyer-Moore pattern matching, coded by Roy Mongiovi, gatech!gitpyr!roy
 * and edited some by Arnold Robbins.
 *
 * No warranty of suitability for any purpose, either expressed
 * or implied, is made.
 */

#include <stdio.h>
#include <ctype.h>

#define TRUE	1
#define FALSE	0

#define MAXPATS	120		/* (arbitrary maximum number of patterns */
#define MAXLINE	(256 + 1)

/*
 * command line options
 */

int Allbut = FALSE;		/* print lines that don't match pattern */
int Exact = FALSE;		/* only print lines that match exactly */
int Countlines = FALSE;		/* only print a count of matching lines */
int Listnames = FALSE;		/* only list file names that match */
int Numberlines = FALSE;	/* print relative line number */
int Silent = FALSE;		/* don't print anything, just exit */
int Monocase = FALSE;		/* ignore case distinctions */

/*
 * variables
 */

long Curline = 0;		/* current file input line */
long Lines_matched = 0;		/* how many lines matched the pattern */

int Lotsafiles = FALSE;		/* are there more than one file? */
int Pat_length[MAXPATS];	/* length of pattern */
int Line_length = 0;		/* length of line */
int Couldnt_open_files = FALSE;	/* one or more files could not be opened */
int Exit_val = 0;		/* return code status */
int Curpat = 0;			/* current pattern comparing against */
int Numpats = 0;		/* total number of patterns */

char Inbuf[MAXLINE];		/* input buffer */
char Pattern[MAXPATS][MAXLINE];	/* pattern to be matched; init = NULs */
char *Program = NULL;		/* program name */

int Argc;			/* make argc and argv global */
char **Argv;

extern char *basename();	/* leaf file of a pathname */

main (argc, argv, envp)
int argc;
char **argv, **envp;
{
	register int i, j;
	char *index();

	Program = basename (argv[0]);
	Argc = argc;
	Argv = argv;

	parse_args ();		/* deal with command line */

	if (Pattern[0][0] == '\0')	/* not from -f or -e */
	{
		if (Argv[0] == NULL)	/* no string given */
			usage ();
		setpats (Argv[0]);
		Argc--;
		Argv++;
	}

	for (Curpat = 0; Curpat <= Numpats; Curpat++)
	{
		if (Monocase)
			mapdown (Pattern[Curpat]);

		Pat_length[Curpat] = strlen (Pattern[Curpat]);
		initialize ();		/* set up necessary tables */
	}

	Lotsafiles = (Argc > 1);	/* more than one file left */

	if (Argc == 0)		/* search stdin */
		process ("-");
	else
		for (i = 0; Argv[i] != NULL; i++)
			process (Argv[i]);
	
	if (! Silent && Countlines)
		printf ("%ld\n", Lines_matched);

	if (Lines_matched == 0)
		exit (1);
	else if (Couldnt_open_files)
		exit (2);
	else
		exit (0);
}

/* process --- start doing the work on each file */

process (infile)
register char *infile;
{
	FILE *fp;
	int c;
	int success;
	long prev_lines_matched = Lines_matched;	/* save count */

	Curline = 0;	/* reset for each file */

	if (infile[0] == '-' && infile[1] == '\0')
		fp = stdin;
	else if ((fp = fopen (infile, "r")) == NULL)
	{
		Couldnt_open_files = TRUE;
		perror (infile);
		return;
	}

	while (fgets (Inbuf, sizeof Inbuf, fp) != NULL)
	{
		Curline++;
		if (Monocase)
			mapdown (Inbuf);
		Line_length = strlen (Inbuf);

		/* first, throw away rest of a truncated input line */
		if (Inbuf[Line_length - 1] != '\n')
			while ((c = getc (fp)) != '\n')
				;
		else
			Inbuf[--Line_length] = '\0';
			/* newline is there, nuke it */

		for (Curpat = 0; Curpat <= Numpats; Curpat++)
		{
			if (success = match ())
				Lines_matched++;
			/* do any necessary output */
			if (! Silent && ! Countlines && ! Listnames &&
				((success != FALSE) ^ (Allbut != FALSE)))
				/* either success, or Allbut, but not both,
				   and not neither */
			{
				if (Lotsafiles)
					printf ("%s: ", infile);
				if (Numberlines)
					printf ("%ld: ", Curline);
				printf ("%s\n", Inbuf);
			}
		}
	}

	fclose (fp);
	if (! Silent && Listnames && prev_lines_matched < Lines_matched)
		printf ("%s\n", infile);
}

/* parse_args --- check out command line arguments */

parse_args ()
{
	register int i,j;

	if (Argc == 1)
		usage ();

	for (Argc--, Argv++; Argv[0] != NULL && Argv[0][0] == '-'; Argc--, Argv++)
	{
		int cheat = FALSE;

		for (j = 1; Argv[0][j] != '\0'; j++)
		{
			switch (Argv[0][j]) {
			case 'c':
				Countlines = TRUE;
				break;

			case 'e':
				strcpy (Pattern[0], Argv[1]);
				Pattern[0][sizeof Pattern[0] - 1] = '\0';
				cheat = TRUE;
				continue;

			case 'f':
				patfromfile (Argv[1]);
				cheat = TRUE;
				continue;

			case 'i':
			case 'y':
				Monocase = TRUE;
				break;

			case 'l':
				Listnames = TRUE;
				break;

			case 'n':
				Numberlines = TRUE;
				break;

			case 's':
				Silent = TRUE;
				break;

			case 'v':
				Allbut = TRUE;
				break;

			case 'x':
				Exact = TRUE;
				break;

			default:
				usage ();
				break;
			}
		}
		if (cheat)
		{
			cheat = FALSE;
			Argc--;
			Argv++;
			/* boy is this stuff a kludge! */
		}
	}

	/* check for argument conflicts */
	if (
		(Silent &&
			(Allbut || Exact || Countlines || Listnames ||
				Numberlines))
		||
		(Allbut && Exact)
		||
		(Countlines && Listnames)
	)
	{
		fprintf (stderr, "%s: argument conflict -- see the man page\n",
			Program);
		usage ();	/* will exit */
	}
}

/* mapdown --- remove case distinctions in a string */

mapdown (str)
register char *str;
{
	register int i;

	for (i = 0; str[i] != '\0'; i++)
		if (isupper (str[i]))
			str[i] = tolower (str[i]);
}

/* return basename part of a pathname, if '/'s are present */

char *basename (str)
register char *str;
{
	register int i = 0;
	register int j = 0;

	for (; str[i] != '\0'; i++)
		if (str[i] == '/')
			j = i;
	
	if (j != 0)
		return (& str[++j]);
	else
		return (str);
}

/* usage --- print usage message and die */

usage ()
{
	fprintf (stderr, "usage: %s [-vxclnisef] [string] [files]\n",
			Program);
	exit (2);
}

/* index --- do index by hand so it'll work on any Unix */

char *index (str, c)
char *str, c;
{
	for (; *str; str++)
		if (*str == c)
			return (str);
	
	return (NULL);
}

/* patfromfile --- retrieve the pattern from the named file */

patfromfile (infile)
char *infile;
{
	register int i, j;
	register FILE *fp;
	register int c;

	if ((fp = fopen (infile, "r")) == NULL ||
			(c = getc (fp)) == EOF)
	{
		perror (infile);	/* be like standard grep */
		exit (2);
	}
	else
		ungetc (c, fp);

	for (i = 0; fgets (Pattern[i], MAXLINE, fp) != NULL; i++)
	{
		if (i >= 120)
		{
			fprintf (stderr, "%s: Only %d strings allowed\n",
				Program, MAXPATS);
			exit (2);
		}
		j = strlen (Pattern[i]);
		if (Pattern[i][j - 1] == '\n')
			Pattern[i][--j] = '\0';
	}
	Numpats = i - 1;

	fclose (fp);
}

/* setpats --- set up the patterns from a string */

setpats (str)
register char *str;
{
	register int i, j;

	while (*str == '\n' || *str == '\r')
		str++;

	for (i = j = 0; *str; str++)
	{
		if (*str == '\n')
		{
			Pattern[i][j] = '\0';
			j = 0;
			i++;
		}
		else
			Pattern[i][j++] = *str;
	}
	Numpats = i;
}

/* begin magic stuff for Boyer-Moore pattern matching */

int D1[MAXPATS][128];
int D2[MAXPATS][128];

int F[MAXPATS][128];

initialize ()
{
	register int i, t;

	for (i = 0; i < 128; i++)
		D1[Curpat][i] = Pat_length[Curpat];
	
	for (i = 0; i < Pat_length[Curpat]; i++)
		D1[Curpat][Pattern[Curpat][i]] = Pat_length[Curpat] - i - 1;
	
	for (i = 0; i < Pat_length[Curpat]; i++)
		D2[Curpat][i] = (Pat_length[Curpat] << 1) - i - 1;
	
	for (i = (t = Pat_length[Curpat]) - 1; i >= 0; i--, t--)
		for (F[Curpat][i] = t; t < Pat_length[Curpat]
			&& Pattern[Curpat][i] != Pattern[Curpat][t];
							t = F[Curpat][t])
			if (Pat_length[Curpat] - i - 1 < D2[Curpat][t])
				D2[Curpat][t] = Pat_length[Curpat] - i - 1;

	for (i = 0; i <= t; i++)
		if (Pat_length[Curpat] + t - i < D2[Curpat][i])
			D2[Curpat][i] = Pat_length[Curpat] + t - i;
}

/* match --- do Boyer-Moore pattern search on input buffer */

match ()
{
	register int i, j;

	if (Exact && Pat_length[Curpat] != Line_length)
		return FALSE;

	i = Pat_length[Curpat] - 1;

	while (i < Line_length)
	{
		j = Pat_length[Curpat] - 1;
		while (j >= 0)
		{
			if (Inbuf[i] == Pattern[Curpat][j])
				i--, j--;
			else
				break;
		}

		if (j < 0)
		{
			/* found a match */
			return TRUE;
			/*
			 * note: if we were going to seach for further matches
			 * on the input line, we would do this:
			 *
			 * i += Pat_length[Curpat] + 1;
			 *
			 * which shifts right by Pat_length[Curpat] + 1 places
			 */
		}
		else
		{
			j = (D1[Curpat][Inbuf[i]] >= D2[Curpat][j]) ?
				D1[Curpat][Inbuf[i]]
			:
				D2[Curpat][j];
			i += j;
			/* shift right by j places */
		}
	}

	return FALSE;
}
