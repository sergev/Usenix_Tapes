/*Copyright (c) 1981 Gary Perlman  All rights reserved*/
#include "menu.h"
FILE *
xopen (name, mode) char *name, *mode;
	{
	FILE	*ioptr = fopen (name, mode);
	if (ioptr == NULL)
		{
		fprintf (stderr, "Can't open %s\n", name);
		exit (1);
		}
	}

syscall (command) char *command;
	{
	char	*alias (), *interpolate ();
	char	*c = alias (interpolate (command), "|;");
	char	*getval (), *shell = getval ("shell");
	char	shellcomm[BUFSIZ];
	*shellcomm = NULL;
	if (strcmp (shell, "/bin/sh"))
		sprintf (shellcomm, "%s -c \"%s\"", shell, c);
	nocrmode (); echo ();
	printf ("%s\n", c);
	system (*shellcomm ? shellcomm : c);
	crmode (); noecho ();
	}
char *
copy (string) char *string;
	{
	char	*copy = (char *) malloc (strlen (string) + 1);
	if (copy == NULL)
		{
		clear (); refresh ();
		printf ("You have run out of space\n");
		endwin ();
		exit (1);
		}
	strcpy (copy, string);
	return (copy);
	}

#include <signal.h>
#include <setjmp.h>
jmp_buf env;
timeout () { longjmp (env, 1); }
timegetc (secs)
	{
	int	c;
	extern	timeout ();
	signal (SIGALRM, timeout);
	alarm (secs);
	if (setjmp (env)) return (0);
	c = getchar ();
	signal (SIGALRM, SIG_IGN);
	alarm (0);
	return (c);
	}
