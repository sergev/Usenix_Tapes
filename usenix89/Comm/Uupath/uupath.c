/*
 * uupath.c
 * rlgvax!dennis (Dennis Bednar) 07 02 86
 * Please send any modification back to the author.
 * 4.2 BSD version
 *	requires 4.2 dbm(3)
 *	requires 4.2 index() or USG strchr()
 *
 * Short program that prints the full UUCP path name of the remote site
 * by consulting the PATHALIASES file.  Here is real output:
 *	$ uupath sun
 *	seismo!sun!%s
 *	$ uupath alien sun
 *	alien	seismo!sun!conejo!alien!%s
 *	sun	seismo!sun!%s
 *	$
 *
 * For example, if you type "uupath remote"
 * uupath will *quickly* look up the remote host and print the
 * full UUCP path name in ! (bang) notation.  This is functionally
 * equivalent to grep'ing for the host in the /usr/lib/pathaliases
 * file, only *much* faster, since it uses dbm(3).
 *
 * Here at rlgvax we have the 4.2 dbm(3) library, and has USG's strchr(3)
 * instead of 4.XBSD's index() routine.  That will explain why
 * I've #ifdef'ed it for USG.  At rlgvax, I cc -DUSG.
 * (Rlgvax is running a hybrid 4.2/System V.)
 *
 * NOTES FOR PURE USG SITES:
 * This version probably requires modifications for pure USG sites, since
 * they don't have dbm(3).  Pure USG sites store their history file
 * in /usr/lib/news/history.d/[0-9].  The algorithm used to search
 * the correct file 0-9 would have to be added here.
 *
 * PS,
 * I borrowed Honeyman's article.c and used it as the basis for this
 * program.  In particular, the char *formats[] array could be removed.
 */

#define PATHALIASES "/usr/lib/pathaliases"

char	*formats[] = {		/* add as appropriate */
	"%s",
	0
};

#include <stdio.h>
#include <sysexits.h>
#include <ctype.h>

typedef struct {
	char	*dptr;
	int	dsize;
} datum;

#if defined(USG)
/* S3 or S5 both call strchr() what 4.X BSD calls index */
#define index strchr
#endif

long	lseek();
char	*index();

/* dbm(3) */
datum	fetch(), dofetch();
datum	firstkey(), nextkey();

/* globals */
char	*cmd;

main(argc, argv)
	int	argc;
	char **argv;
{
	int	fd;
	datum	content;
	int	onehost;	/* true iff asking for > 1 host */

	cmd = argv[0];

	if (argc == 1)
		{
		fprintf(stderr, "usage: %s remote_uucp_site [... remote ]\n", cmd);
		exit( EX_USAGE );
		}

	if ((fd = open(PATHALIASES, 0)) < 0) {
		perror(PATHALIASES);
		exit(EX_UNAVAILABLE);
	}
	if (dbminit(PATHALIASES) < 0) {
		fprintf(stderr, "dbm error\n");
		exit(EX_UNAVAILABLE);
	}

#if 0
	/* commented out */
	/* I used this code the first time I experimented with dbm(3).
	 * This piece of code sequentially walks the file, printing
	 * both the key (abbreviated host name), and the value (full
	 * uucp path name with %s format).
	 */
	{
	datum	key;
	datum	result;

	for (key = firstkey(); key.dptr != NULL ; key = nextkey(key))
		{
		printf("key = %s  ", key.dptr);
		result = fetch(key);
		if (result.dptr != NULL)
			printf("%s\n", result.dptr);
		else
			printf("NULL\n");
		}
	exit (EX_OK);
	}
#endif

	if (argc > 2)
		onehost = 0;	/* asking for multiple hosts */
	else
		onehost = 1;	/* asking for only one host */

	for (--argc, argv++; argc; --argc, argv++) {
		content = dofetch(*argv);
		if (content.dptr == 0) {
			printf("%s: Can't find host %s\n", cmd, *argv);
			continue;
		}

		if (onehost)
			printf("%s\n", content.dptr);
		else
			printf("%s\t%s\n",*argv, content.dptr);
		continue;

	}
	exit(EX_OK);
}

datum
dofetch(str)
char	*str;		/* the name of a remote site */
{
	datum	key, content;
	char	buf[BUFSIZ], **fmt;
	register char *rcp;

	for (fmt = formats; *fmt; fmt++) {
		sprintf(buf, *fmt, str);
#if 0
	/* upper to lower case conversion is commented out because
	 * 'uupath BLUE' does not work.
	 */
		rcp = buf - 1;
		while (*++rcp)
			if (isupper(*rcp))
				*rcp = tolower(*rcp);
#endif
#ifdef DEBUG
		printf("searching for '%s'\n", buf);
#endif
		key.dptr = buf;
		key.dsize = strlen(buf) + 1;
		content = fetch(key);
		if (content.dptr)
			break;
	}
	return(content);
}
