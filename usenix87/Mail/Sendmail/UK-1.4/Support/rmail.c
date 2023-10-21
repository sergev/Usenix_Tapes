#ifndef lint
static char sccsid[] =	"@(#)rmail.c	4.4 (Berkeley) 8/11/83";
#endif

/*
**  RMAIL -- UUCP mail server.
**
**	This program reads the >From ... remote from ... lines that
**	UUCP is so fond of and turns them into something reasonable.
**	It calls sendmail giving it a -f option built from these
**	lines.
**
**	Modified to set the sender's hostname (deduced from first
**	"remote from host" line) and convert the final "user" part
**	from an address with '@' and '%' (which mmdf is so fond of)
**	in to a 'pure' uucp address.	-Jim Crammond, (hwcs!jim) 29/11/84
*/

# include <stdio.h>
# include <sysexits.h>

typedef char	bool;
#define TRUE	1
#define FALSE	0

extern FILE	*popen();
extern char	*index();
extern char	*rindex();
char	*conv_at();

bool	Debug;

# define MAILER	"/usr/lib/sendmail"

main(argc, argv)
	char **argv;
{
	FILE *out;	/* output to sendmail */
	char lbuf[512];	/* one line of the message */
	char from[512];	/* accumulated path of sender */
	char ufrom[64];	/* user on remote system */
	char sys[64];	/* a system in path */
	char sysname[64]; /* system received from */
	char cmd[2000];
	register char *cp;
	register char *uf;	/* ptr into ufrom */
	int firstline = 1;
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
	(void) strcpy(sysname, "");
	(void) strcpy(ufrom, "/dev/null");

	while (fgets(lbuf, sizeof lbuf, stdin) != NULL)
	{
		if (strncmp(lbuf, "From ", 5) != 0 && strncmp(lbuf, ">From ", 6) != 0)
			break;
		(void) sscanf(lbuf, "%*s %s", ufrom);
		cp = lbuf;
		uf = ufrom;

		while ((cp = index(cp, 'r')) != NULL)
		{
#ifdef DEBUG
			if (Debug)
				printf("cp='%s'\n", cp);
#endif
			if (sscanf(cp, "remote from %s", sys) == 1)
			{	(void) strcat(from, sys);
				(void) strcat(from, "!");
				if (firstline)
					(void) strcpy(sysname, sys);
				break;
			}
			cp++;
		}
#ifdef DEBUG
		if (Debug)
			printf("ufrom='%s', sys='%s', from now '%s'\n", uf, sys, from);
#endif
		firstline = 0;
	}
	(void) strcat(from, conv_at(uf));

	(void) sprintf(cmd, "%s -em -f%s", MAILER, from);
	if (*sysname != '\0')
	{	(void) strcat(cmd, " -oMs");
		(void) strcat(cmd, sysname);
	}

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

/*
**  CONV_AT  --  converts an address with 'at' symbols in to uucp style
**
**	This converts addresses such as a%b%c@d to d!c!b!a style
**	so that when the path deduced from the "remote from" lines is
**	prepended, the address might still be valid.
**	If a domain contains ".uucp" or ".UUCP" this gets stripped.
**	e.g.  c!user%b.uucp@a.uucp -> a!b!c!user
**	      user%d.dec%c.arpa%b.uucp@a.uucp -> a!b!c.arpa!d.dec!user
*/	
char	*
conv_at(addr)
char	*addr;
{
	char	buf[512];
	char	*bp = buf;
	char	*p;

#ifdef DEBUG
	if (Debug)
		printf("conv_at(%s) ", addr);
#endif
	while ((p = rindex(addr,'@')) != NULL || (p = rindex(addr,'%')) != NULL)
	{
		*p++ = '\0';
		while (*p)
		{
			if (strcmp(p, ".uucp") == 0 || strcmp(p, ".UUCP") == 0)
				break;

			*bp++ = *p++;
		}
		*bp++ = '!';
	}

	strcpy(bp, addr);
#ifdef DEBUG
	if (Debug)
		printf("returns %s\n", buf);
#endif
	return(buf);
}
