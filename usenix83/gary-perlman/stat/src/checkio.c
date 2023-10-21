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
canwrite (filename) char *filename;
	{
	struct stat	status;
	if (stat (filename, &status)) return (1); /* no such file */
	if (status.st_size == 0) return (1); /* nothing in it */
	return (confirm ("Destroy the contents of %s?", filename));
	}

#include <sgtty.h>
#ifndef	CBREAK
#define	CBREAK RAW
#endif
#include <signal.h>
confirm (fmt, args) char *fmt;
	{
	char ch;
	struct sgttyb normal, raw;
	gtty (fileno (stderr), &normal);
	gtty (fileno (stderr), &raw);
	raw.sg_flags |= CBREAK;
	raw.sg_flags &= ~ECHO;
    getconfirm:
	_doprnt(fmt, &args, stderr);
	fprintf (stderr, " (y/n) ");
	signal (SIGINT, SIG_IGN);
	stty (fileno (stderr), &raw);
	read (2, &ch, 1);
	stty (fileno (stderr), &normal);
	signal (SIGINT, SIG_DFL);
	switch (ch)
		{
		case 'Y': case 'y': fprintf (stderr, "yes\n"); return (1);
		case 'N': case 'n': fprintf (stderr, "no\n"); return (0);
		default: fprintf (stderr, "y for yes, n for no\n");
		}
	goto getconfirm;
	}

checkstdin (program)
char *program;
	{
	struct	sgttyb ttybuf;
	if (gtty (fileno (stdin), &ttybuf) == 0)
	    {
	    fprintf (stderr,"%s: Reading input from tty keyboard\n", program);
	    fprintf (stderr, "End with CTRL-d\n");
	    return (1);
	    }
	return (0);
	}
