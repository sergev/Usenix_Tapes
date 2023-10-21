/*		Copyright (c) 1982 Gary Perlman
This software may be copied freely provided:  (1) it is not used for
personal or material gain, and (2) this notice accompanies each copy.

Disclaimer:  No gauarantees of performance accompany this software,
nor is any responsbility assumed on the part of the author.  All the
software has been tested extensively and every effort has been made to
insure its reliability.*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sgtty.h>
#define ttyin !pipein
#define ttyout !pipeout
char	*rindex ();
int	flowmeter;
int	forceio;
int	appendio;
char	pgm[100];
main (argc, argv) char **argv;
	{
	struct	sgttyb	tty;
	int	pipein = gtty (fileno (stdin), &tty);
	int	pipeout = gtty (fileno (stdout), &tty);
	strcpy (pgm, argv[0]);
	argc = checkoptions (argc, argv);
	if (pipein && pipeout)		/* in the middle of a pipeline */
	    {
	    if (argc == 1)
		{
		flowmeter = 1;
		foutput (stdin, stdout, 0);
		exit (0);
		}
	    else if (argc > 2) eprintf ("Can only output to one file");
	    output (0, argv);
	    }
	else if (pipein && ttyout)	/* at the end of the pipeline */
	    {
	    if (argc == 1) eprintf ("No file to output to");
	    output (argc, argv);
	    }
	else if (ttyin)			/* at beginning of pipeline */
	    {
	    if (argc == 1) eprintf ("No input files");
	    if (ttyout && flowmeter)
		{
		execv ("/usr/ucb/more", argv);
		printf ("%s: No flowmeter for tty output\n", pgm);
		flowmeter = 0;
		}
	    input (argc, argv);
	    }
	exit (0);
	}

checkoptions (argc, argv)
char **argv;
	{
	int	i, j;
	char	*optr;
	for (i = 1; i < argc; i++)
	    if (*argv[i] == '-') /* flag argument */
		{
		optr = &argv[i][1];
		while (*optr)
		    {
		    switch (*optr)
			{
			case 'a': appendio = 1; break;
			case 'm': flowmeter = 1; break;
			case 'f': forceio = 1; break;
			default: eprintf ("Unknown flag option '%c'", *optr);
			}
		    optr++;
		    }
		for (j = i; j < argc-1; j++) argv[j] = argv[j+1];
		i--, argc--;
		}
	argv[argc] = 0;
	return (argc);
	}

begins (s1, s2) char *s1, *s2;
	{
	while (*s1)
		if (*s1++ != *s2++) return (0);
	return (1);
	}
substr (s1, s2) char *s1, *s2;
	{
	while (*s2)
	      if (begins (s1, s2)) return (1);
	      else s2++;
	return (0);
	}

foutput (in, out, pipeout) FILE *in, *out;
	{
	char	*flowchrs = "=*#@$&+";
	char	flowchar = flowchrs[getpid () % 7];
	char	buffer[BUFSIZ];
	int	nbytes;
	while ((nbytes = fread (buffer, 1, BUFSIZ, in)) == BUFSIZ)
		{
		fwrite (buffer, 1, BUFSIZ, out);
		if (pipeout) fwrite (buffer, 1, BUFSIZ, stdout);
		if (flowmeter) fputc (flowchar, stderr);
		}
	fwrite (buffer, 1, nbytes, out);
	if (pipeout) fwrite (buffer, 1, nbytes, stdout);
	if (flowmeter) fputc ('\n', stderr);
	}

char *
directory (file)
char *file;
	{
	static char dir[100];
	if (!strcmp (file, "/")) return (file);
	strcpy (dir, file);
	if (file = rindex (dir, "/")) *file = NULL;
	else strcpy (dir, ".");
	return (dir);
	}

output (argc, argv) char **argv;
	{
	char	tmp[50];
	char	*filename;
	FILE	*ioptr;
	char	copycomm[100];
	char	*sprintf ();
	int	pipeout = (argc == 0);
	if (argc == 1)
		{
		foutput (stdin, stdout, 0);
		exit (0);
		}
	if (argc > 2) eprintf ("Can only output to one file");
	if (access (argv[1], 4) == 0) /* must create tmp file */
	    {
	    if (access (argv[1], 2) || access (directory (argv[1]), 2))
		/* must be able to overwrite file and directory */
		eprintf ("Can't overwrite %s", argv[1]);
	    if (forceio == 0)
		if (!confirm ("delayed safe overwrite of %s?", argv[1]))
		    eprintf ("Exiting to avoid overwriting %s", argv[1]);
	    if (filename = rindex (argv[1], '/')) filename++;
	    else filename = argv[1];
	    sprintf (tmp, "/tmp/%d%s", getpid (), filename);
	    filename = tmp;
	    if ((ioptr = fopen (filename, "w")) == NULL)
		    eprintf ("Can't create temporary file %s", filename);
	    }
	else
	    {
	    filename = argv[1];
	    if ((ioptr = fopen (filename, "w")) == NULL)
		    eprintf ("Can't create %s", filename);
	    }
	foutput (stdin, ioptr, pipeout);
	fclose (ioptr);
	if (filename == tmp)
		{
		system (sprintf (copycomm, "cat %s %s %s",
			tmp, appendio ? ">>" : ">", argv[1]));
		unlink (tmp);
		}
	exit (0);
	}

input (argc, argv) char **argv;
	{
	int	i;
	int	fatalerror = 0;
	struct	stat statbuf;
	FILE	*ioptr;
	char	c;
	long	size = 0, sofar = 0;
	for (i = 1; i < argc; i++)
	    {
	    if (stat (argv[i], &statbuf))
		{
		fprintf (stderr, "%s: Can't find %s\n", pgm, argv[i]);
		fatalerror = 1;
		}
	    else size += statbuf.st_size;
	    }
	if (fatalerror) exit (1);
	size /= 10;
	for (i = 1; i < argc; i++)
	    {
	    if ((ioptr = fopen (argv[i], "r")) == NULL)
		eprintf ("Can't open %s", argv[i]);
	    while ((c = fgetc (ioptr)) != EOF)
		{
		putchar (c);
		if (flowmeter && (++sofar%size == 0))
			fprintf (stderr, "%4D%%",10*sofar/size);
		}
	    fclose (ioptr);
	    }
	if (flowmeter) fputc ('\n', stderr);
	exit (0);
	}

eprintf(fmt, args) char *fmt;
	{
	fprintf (stderr, "%s: ", pgm);
	_doprnt(fmt, &args, stderr);
	fputc ('\n', stderr);
	exit(1);
	}
