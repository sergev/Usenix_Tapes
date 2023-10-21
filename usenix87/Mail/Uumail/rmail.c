#ifndef lint
static char rcsid[]="$Header: rmail.c,v 4.0 86/11/17 16:02:32 sob Exp $";

#endif

/*
**  RMAIL -- UUCP mail server.
**
**	This program reads the >From ... remote from ... lines that
**	UUCP is so fond of and turns them into something reasonable.
**	It calls uumail giving it a -f option built from these
**	lines.
***************************************************************************
This work in its current form is Copyright 1986 Stan Barber
with the exception of opath, gethostname and the original getpath which
as far as I know are in the Public Domain. This software may be distributed
freely as long as no profit is made from such distribution and this notice
is reproducted in whole.
***************************************************************************
This software is provided on an "as is" basis with no guarantee of 
usefulness or correctness of operation for any purpose, intended or
otherwise. The author is in no way liable for this software's performance
or any damage it may cause to any data of any kind anywhere.
***************************************************************************
*/

#define _DEFINE

#include "uuconf.h"
extern FILE *popen();
extern char *index();
extern char *rindex();

main(argc, argv)
	char **argv;
{
	FILE *out;	/* output to mail handler */
	char lbuf[512];	/* one line of the message */
	char from[512];	/* accumulated path of sender */
	char ufrom[64];	/* user on remote system */
	char sys[64];	/* a system in path */
	char junk[512];	/* scratchpad */
	char cmd[2000];
	register char *cp;
	register char *uf;	/* ptr into ufrom */
	int i;

# ifdef DEBUG
	if (argc > 1 && strcmp(argv[1], "-T") == 0)
	{
		Debug = TRUE;
		argc--;
		argv++;
	}
# endif DEBUG

	if (argc < 2)
	{
		fprintf(stderr, "Usage: rmail user ...\n");
		exit(EX_USAGE);
	}

	(void) strcpy(from, "");
	(void) strcpy(ufrom, "/dev/null");
	uf = NULL;

	for (;;)
	{
		(void) fgets(lbuf, sizeof lbuf, stdin);
		if (strncmp(lbuf, "From ", 5) != 0 && strncmp(lbuf, ">From ", 6) != 0)
			break;
		(void) sscanf(lbuf, "%s %s", junk, ufrom);
		cp = lbuf;
		uf = ufrom;
		for (;;)
		{
			cp = index(cp+1, 'r');
			if (cp == NULL)
			{
				register char *p = rindex(uf, '!');

				if (p != NULL)
				{
					*p = '\0';
					if (uf != NULL) 
						(void) strcpy(sys, uf);
					else
						gethostname(sys,32);
					uf = p + 1;
					break;
				}
				cp = "remote from somewhere";
			}
#ifdef DEBUG
			if (Debug)
				printf("cp='%s'\n", cp);
#endif
			if (strncmp(cp, "remote from ", 12)==0)
				break;
		}
		if (cp != NULL)
			(void) sscanf(cp, "remote from %s", sys);
		(void) strcat(from, sys);
		(void) strcat(from, "!");
#ifdef DEBUG
		if (Debug)
			printf("ufrom='%s', sys='%s', from now '%s'\n", uf, sys, from);
#endif
	}
	if (uf != NULL)
		(void) strcat(from, uf);

/*	(void) sprintf(cmd, "exec %s -oMrUUCP -em -f%s", MAILER, from);*/
	if (from[0] == '\0')
		(void) sprintf(cmd, "exec %s", MAILER);
	else
		(void) sprintf(cmd, "exec %s -f%s", MAILER, from);
	while (*++argv != NULL)
	{
		(void) strcat(cmd, " '");
		if (**argv == '(')
			(void) strncat(cmd, *argv + 1, strlen(*argv) - 2);
		else
			(void) strcat(cmd, *argv);
		(void) strcat(cmd, "'");
	}
#ifdef DEBUG
	if (Debug)
		printf("cmd='%s'\n", cmd);
#endif
	out = popen(cmd, "w");
	fputs(lbuf, out);
	while (fgets(lbuf, sizeof lbuf, stdin))
		fputs(lbuf, out);
	i = pclose(out);
	if ((i & 0377) != 0)
	{
		fprintf(stderr, "pclose: status 0%o\n", i);
		exit(EX_OSERR);
	}

	exit((i >> 8) & 0377);
}
