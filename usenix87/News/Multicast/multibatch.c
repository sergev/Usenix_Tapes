/*	multibatch.c - program to send news batches to multiple systems.
 *
 *	86/06/14.	2.1	Shane P. McCarron (MECC)
 *
 *	Copyright (C) Shane McCarron, 1987.
 *
 *	Multibatch will take a batch file created with the Multicasting 
 *	option of news 2.10.3 and later, and distribute common articles to
 *	sites using the broadcasting facility of uucast (also included in
 *	this software package).
 *
 *	The format of this command is:
 *
 *	multibatch [-s size] [ -c ] [ -c7 ] [ -obBC ] multicast
 *
 *	all parameters are like those of sendbatch except multicast, which
 *	is used in place of site.  Multicast is the name of the file which 
 *	contains the inews generated lines about article name and destination
 *	sites.  Multibatch will cluster those articles which need to go to the
 *	same sites, and feed that list to a sendbatch like script.
 *
 *	Since the parameters (options) are not really used by this
 *	program, but are used by the script it calls, the options are
 *	just ripped out of argv and placed in a string.
 */

#include  <stdio.h>
#include  <errno.h>

#ifndef	PATH_MAX
#define	PATH_MAX	255
#endif	PATH_MAX

#ifndef	LIBDIR
#define	LIBDIR		"/usr/lib/news"
#endif	LIBDIR

#ifndef	BATCHDIR
#define	BATCHDIR	"/usr/spool/batch"
#endif	BATCHDIR

extern	int	errno;

typedef	struct	node
{
	struct	node	*link;		/* forward link */
	char	*line;			/* line from file */
} NODE;

typedef	NODE	*NODEPTR;

FILE	*outfile;

main(argc, argv)
int	argc;
char	*argv[];
{
	FILE	*fopen();
	char	*getsys();
	char	*malloc();
	void	error();
	void	printart();

	NODE	arts;
	NODEPTR	last = &arts;
	char	argstr[BUFSIZ];
	char	filename[PATH_MAX];
	char	flags[BUFSIZ];
	char	instr[BUFSIZ];
	char	oldname[PATH_MAX];
	char	*systems;

	int	artsleft;
	int	i = 1;
	int	numarts = 0;

	arts.link = NULL;
	while ((i < argc) && (argv[i][0] == '-'))
	{
		strcat(flags, argv[i]);
		strcat(flags, " ");
		i++;
	} /* end while */
	if (i == argc) 
	{
		fprintf(stderr, "usage: %s options batch\n", argv[0]);
		exit(2);
	} /* end if */

	fclose(stdin);	/* we don't need this */
	(void) sprintf(oldname, "%s/%s", BATCHDIR, argv[i]);
	(void) sprintf(filename, "%s.work", oldname);
	if (link(oldname, filename) == -1)
	{
		if (errno == ENOENT)
			exit(1);
		else
			error(errno);
	}  /* end if */

	/* get rid of the original file */

	(void) unlink(oldname);

	if (fopen(filename, "r") == NULL)
		error(errno);
	while (gets(instr) != NULL)
	{
		if ((last->link = (NODEPTR) malloc(sizeof(NODE))) == NULL)
			error(2);
		last = last->link;
		if ((last->line = malloc((unsigned) strlen(instr) + 1)) == NULL)
			error(2);
		strcpy(last->line, instr);
		numarts++;
	} /* end while */

	/* If you are testing this, then comment out the unlink */

	unlink(filename);

	/* start processing these articles */

	artsleft = numarts;
	while (artsleft > 0)
	{
		last = arts.link;		/* start with first node */
		while (last->line == NULL)
			last = last->link;
		systems = getsys(last->line);
		strcpy(filename, "/tmp/mbXXXXXX");
		if (mktemp(filename) == NULL)
			error(254);
		if ((outfile = fopen(filename, "w")) == NULL)
			error(errno);
		printart(last->line);
		last->line = NULL;
		last = last->link;
		artsleft--;
		while (last)
		{
			if (last->line != NULL)
				if (strcmp(systems, getsys(last->line)) == 0)
				{
					printart(last->line);
					last->line = NULL;
					artsleft--;
				} /* end if */
			last = last->link;
		} /* end while */
		fclose(outfile);
		(void) sprintf(argstr, "%s/multisend %s -S %s %s", LIBDIR, flags, filename, systems);
		i = system(argstr); 
		unlink(filename);
		if (i != 0)
			error(i);
	} /* end while */

	return(0);
}

char	*getsys(line)
char	*line;
{
	char	*s = line;

	while (*s != ' ') s++;
	return(++s);
}

void	printart(line)
char	*line;
{
	int	i = 0;

	while (line[i] != ' ') 
	{
		putc(line[i], outfile);
		i++;
	} /* end while */
	putc('\n', outfile);
}

void	error(err)
int	err;
{
	fprintf(stderr, "Program blew off.  Error = %d.\n", err);
	exit(err);
}
