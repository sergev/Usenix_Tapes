/*
** cxref.c
**
** C driver for Cxref program.
** does argument handling, then builds the right
** shell commands for passing to the system() routine.
**
** A possible but difficult speed improvement would be to
** set up the argument vectors ourselves, the i/o with a pipe()
** call, and do all the forking and execing ourselves.
** But, in keeping w/the philosophy of "Let someone else do
** the hard part", we leave well enough alone and let the shell do it.
**
** Arnold Robbins, Information and Computer Science, Georgia Tech
**	gatech!arnold
** Copyright (c) 1984 by Arnold Robbins
** All rights reserved
** This program may not be sold, but may be distributed
** provided this header is included.
*/

#include <stdio.h>
#include <ctype.h>

#ifdef DEBUG
#define system(str)	printf("%s\n", str)
#endif

#ifdef TESTOUT
dosystem(str)
char *str;
{
	int pid;

	fprintf(stderr, "%s\n", str);
	system(str);
}

#define system(str)	dosystem(str)	/* takes effect after above routine */
#endif

#define TRUE	1
#define FALSE	0

char *name;		/* save command name */

int xargc;		/* make argc and argv available globally */
char **xargv;

int width = 0;		/* output width */

int sepflag = FALSE;	/* do each one separately */

int iflag = TRUE;	/* print out ints */
int fflag = TRUE;	/* print out floats */
int cflag = TRUE;	/* print out chars */
int sflag = TRUE;	/* print out strings */

char *filename();	/* turns "-" into "stdin" */

main(argc, argv)
int argc;
char **argv;
{
	int i;
	int extra_arg = FALSE;

	name = filename(argv[0]);

	for(argv++, argc--; argc > 0; argv++, argc--)
		if (argv[0][0] != '-')
			break;
		else if(argv[0][1] == '\0')	/* filename of "-" */
			break;
		else
			for(i = 1; argv[0][i] != '\0'; i++)
			{
				switch(argv[0][i]) {
				case 'S':
					sepflag = TRUE;
					break;
				
				case 'C':
					/* leave out all constants */
					cflag =
					iflag =
					fflag =
					sflag = FALSE;
					break;

				case 'c':
					cflag = FALSE;
					break;
				
				case 'i':
					iflag = FALSE;
					break;
				
				case 'f':
					fflag = FALSE;
					break;
				
				case 's':
					sflag = FALSE;
					break;
				
				case 'w':
					if (isdigit(argv[0][i+1]))
					{
						width = 0;
						for(i++; isdigit(argv[0][i]); i++)
							width = width * 10 + argv[0][i] - '0';
						i--;
					}
					else
					{
						width = atoi(argv[1]);
						extra_arg = TRUE;
					}
					break;

				default:
					usage();
					break;
				}

				if (extra_arg)	/* skip column width */
				{
					extra_arg = FALSE;
					/* do this only once */
					/* inside the for loop */
					argv++;
					argc--;
				}
			}
	
	if (width != 0)
		if (width < 51)
			width = 80;
		else if (width > 132)
			width = 132;

	xargc = argc;
	xargv = argv;

	runprogs();
}

char command[BUFSIZ * 10];	/* may need LOTS of room */
char com_buf[BUFSIZ * 10];	/* use short name for portability */

char *docxref();	/* functions to generate commands with args */
char *filter();
char *fmtxref();

#define ONLYONE		1
#define ALLOFTHEM	2

runprogs()		/* execute the programs */
{
	int i;

	if (sepflag)	/* do each file separately */
	{
		for (i = 0; i < xargc; i++)
		{
			printf("\tC Cross Refence Listing of %s\n\n",
					filename(xargv[i]));
			fflush(stdout);
			/* send to ouput before commands start */
			sprintf(command,
				"%s | sort -u +0b -2 +2n | %s | %s",
				docxref(i, ONLYONE), filter(), fmtxref());
			system(command);
			if (iflag)
			{
				sprintf(com_buf,
	"sort -u +0n -1 +1b -2 +2n < /tmp/cxr.%d.1 | %s/cxrfilt -i | %s",
					getpid(), SRCDIR, fmtxref());
				sprintf(command,
			"if test -s /tmp/cxr.%d.1 ; then %s ; fi",
					getpid(), com_buf);
				system(command);
			}
			if (fflag)
			{
				sprintf(com_buf,
"sort -u +0n +1n -2 +2b -3 +3n < /tmp/cxr.%d.2 | %s/cxrfilt -f | %s",
					getpid(), SRCDIR, fmtxref());
				sprintf(command,
			"if test -s /tmp/cxr.%d.2 ; then %s ; fi",
					getpid(), com_buf);
				system(command);
			}
			fflush(stdout);
			if (! isatty(fileno(stdout)))
				putchar('\f');
		}
	}
	else
	{
		if (xargc == 1)
			printf("\tC Cross Refence Listing of %s\n\n",
					filename(xargv[0]));
		else
			printf("\tC Cross Reference Listing\n\n");
		fflush(stdout);
		sprintf(command, "%s | sort -u +0b -2 +2n | %s | %s",
			docxref(0, ALLOFTHEM), filter(), fmtxref());
		system (command);
		if (iflag)
		{
			sprintf(com_buf,
	"sort -u +0n -1 +1b -2 +2n < /tmp/cxr.%d.1 | %s/cxrfilt -i | %s",
				getpid(), SRCDIR, fmtxref());

			sprintf(command,
				"if test -s /tmp/cxr.%d.1 ; then %s ; fi",
				getpid(), com_buf);
			system(command);
		}
		if (fflag)
		{
			sprintf(com_buf,
"sort -u +0n +1n -2 +2b -3 +3n < /tmp/cxr.%d.2 | %s/cxrfilt -f | %s",
				getpid(), SRCDIR, fmtxref());
			sprintf(command,
				"if test -s /tmp/cxr.%d.2 ; then %s ; fi",
				getpid(), com_buf);
			system(command);
		}
		fflush(stdout);
		if (! isatty(fileno(stdout)))
			putchar('\f');
	}

	sprintf(command, "rm -f /tmp/cxr.%d.[12]", getpid());
	system(command);
}

char *docxref(index, howmany)
int index, howmany;
{
	static char buf[BUFSIZ * 10];
	int i, j;

	if (howmany == ONLYONE)
		sprintf(buf, "%s/docxref %s", SRCDIR, xargv[index]);
	else
	{
		/* put all the args on one command line */
		sprintf(buf, "%s/docxref ", SRCDIR);

		i = strlen(buf);
		for(; xargc > 0; xargc--, xargv++)
		{
			for(j = 0; xargv[0][j] != '\0'; j++)
				buf[i++] = xargv[0][j];
			buf[i++] = ' ';
		}
		buf[i] = '\0';
	}

	return (buf);
}

char *filter()		/* command line for splitting off ints and floats */
{
	static char buf[40];

	if (! cflag && sflag)
		sprintf(buf, "%s/cxrfilt -c %d", SRCDIR, getpid());
	else if (cflag && ! sflag)
		sprintf(buf, "%s/cxrfilt -s %d", SRCDIR, getpid());
	else if (! cflag && ! sflag)
		sprintf(buf, "%s/cxrfilt -cs %d", SRCDIR, getpid());
	else
		sprintf(buf, "%s/cxrfilt %d", SRCDIR, getpid());

	return (buf);
}

char *fmtxref()
{
	static char buf[40];

	if (width != 0)
		sprintf(buf, "%s/fmtxref -w %d", SRCDIR, width);
	else
		sprintf(buf, "%s/fmtxref", SRCDIR);
	
	return(buf);
}

usage()
{
	fprintf(stderr, "usage: %s [-SCcsif] [-w width] [files]\n", name);
	exit (1);
}

char *filename(fname)
char *fname;
{
	char *cp, *basename();

	cp = basename(fname);

	return ( strcmp(cp, "-") == 0 ? "stdin" : cp);
}

#include "basename.c"
