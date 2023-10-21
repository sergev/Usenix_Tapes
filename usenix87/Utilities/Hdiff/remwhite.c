/*
 * f=remwhite.c
 * author - dennis bednar  8 30 84
 *
 * library and standalone routine to remove excess white space from a string.
 * If there are multiple white spaces together they are transformed into
 * one blank character.  New lines (if any) in the string are not touched.
 * A string can consist of more than one line (ie multiple '\n' newline
 * separators in the string), but usually a string will
 * consist of one line followed by newline, and then terminated.
 *
 * White space at the beginning of each line in the string is reduced to
 * one blank, if rem1stwhite=0;
 * White space at the beginning of each line in the string is reduced to
 * no-blanks, if rem1stwhite=1;
 * White space for between 2nd, 3rd, etc. non-whites is in each line is
 * reduced to one blank.
 * White space at the end of the line in the string is REMOVED.
 */

#include <stdio.h>
#include "stripnl.h"

char *fgets ();

/* max strlen (number of chars) returned by remwhite */
#define MAXSTRING 1024	

/* max line length that main can handle */
#define	LINESIZE	1024		/* user sees max line length */
#define _LINESIZE	LINESIZE+2	/* declare buffer size room for "\n\0" at end*/


/* last char at the end is for the NULL terminator */
/* this is the output buffer returned by remwhite() */
static	char	obuffer [MAXSTRING+1];

char *
remwhite (inbuf, rem1stwhite)
	char *inbuf;
	char	rem1stwhite;	/* 0 = leave leading white space if any */
				/* 1 = remove leading white space if any */

{
	register	char	*src,	/* current pointer into input buf */
				*dst,	/* current pointer into output buf */
				*end;	/* first char AFTER end of output buffer */
	int		inwhite;	/* true iff in middle of white space */


	inwhite = 0;

	/* initialize the src and dest ptrs */
	src = inbuf;
	dst = obuffer;

nextline:

	/* skip over leading white space in this 'line' in input buffer */
	/* DONT treat newlines as white space, otherwise, if there were
	 * multiple lines in the input string, then we would remove them,
	 * and we don't want to.
	 * src is positioned at the beginning of a line within inbuf.
	 */
	if (rem1stwhite)
		for (; *src; ++src)
			if (*src == ' ' || *src == '\t')
				continue;
			else
				break;	/* found first non-white input char */

	/* this logic is implemented to output the blank char AFTER
	 * an inwhite to !inwhite state transition.
	 * The reason is so that white space at the end
	 * of the string will be removed.
	 */
	for (end = &obuffer[sizeof(obuffer)]; *src; ++src)
		{
		if (*src == ' ' || *src == '\t')
			inwhite = 1;	/* don't output anything, just remember we're seeing white space */
		else if (inwhite)	/* transition, time to output the old blank */
			{
			inwhite = 0;
			*dst++ = ' ';
			if (dst >= end)
				goto error;
			*dst++ = *src;
			if (*src == '\n')	/* found end of a line in the input buffer */
				goto nextline;	/* work on beginning of next line */
			}
		else
			*dst++ = *src;

		/* prevent output buffer overflows */
		if (dst >= end)
			{
error:
			fprintf (stderr, "remwhite: overran buffer.  More than %d chars.\n", MAXSTRING);
			exit (5);
			}
		}


	/* terminate the string */
	*dst = '\0';

	return obuffer;
}

#ifdef STAND

/*
 * test out the remwhite function
 */

static	char	*cmd;	/* name of this command */


/*
 * usage:
cmd [-a] [file ...]	# -a means remove leading white space in each line
			# if no file(s) given, use stdin
 */



main (argc, argv)
	int argc;
	char **argv;
{
	register	int i;
	int	striplead = 0;	/* default is DONT zap leading blanks */
				/* ie default is to keep leading white space */
				/* 1 would change " 1   2" to "1 2" */
				/* 0 would leave  " 1   2" as " 1 2" */

	cmd = argv [0];

	/* first process possible options */
	for (i = 1; i < argc; ++i)
		if (strcmp(argv[i], "-a") == 0)
			{
			striplead = 1;
			continue;
			}
		else
			break;

	/* i now is index of first file in arg list, if any */

	if (i == argc)
		dofile ("", striplead);	/* read from stdin */
	else for ( ;i < argc; ++i)	/* read each file */
		dofile (argv[i], striplead);

	exit (0);	/* normal */
}

/*
 * process a file
 */
dofile (filename, zapflag)
	char *filename;
	int	zapflag;	/* 1 = zap leading white space */
{
	FILE	*infp,		/* input file	*/
		*fopen ();	/* fopen (3)	*/

	if (*filename == '\0')
		{
		filename = "[stdin]";	/* in case we need to print file name */
		infp = stdin;
		}
	else
		{
		infp = fopen (filename, "r");
		if (infp == (FILE *)NULL)
			{
			sprintf (obuffer, "%s: can't open %s", cmd, filename);
			perror (obuffer);
			exit (2);
			}
		}

	dolines (infp, filename, zapflag);


	fclose (infp);
}


/*
 * process the lines
 */
dolines (infp, filename, zapflag)
	FILE	*infp;		/* stream pointer to read the input file	*/
	char	*filename;	/* name of file we are reading from (for msg) */
	int	zapflag;	/* 1 = zap leading white space */
{
	char	buffer [_LINESIZE],	/* hold line from stdin here */
		*cp;		/* get pointer to compressed line */
	int	stat,		/* status after stripping newline */
		linenum;	/* current line number we are reading */

	/* read lines from infp and take out the blanks */
	/* print them to show the effect */
	linenum = 0;
	while (fgets (buffer, sizeof(buffer), infp) != (char *) NULL)
		{
		++linenum;

		stat = stripnl (buffer, sizeof(buffer) );

		if (stat == L_SUCCESS)
			;		/* okay */
		else if (stat == L_BADFORM)
			fprintf (stderr, "%s: Warning, line %d in file %s not terminated by newline.\n", cmd, linenum, filename);
		else
			{
			fprintf (stderr, "%s: Line %d in file %s longer than %d chars\n", cmd, linenum, filename, LINESIZE);
			exit (4);
			}

		cp = remwhite (buffer, zapflag);

		printf ("%s\n", cp);
		}
}




#endif
