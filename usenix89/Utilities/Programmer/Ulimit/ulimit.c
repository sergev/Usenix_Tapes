/*
 *	ulimit.c  -  get/set user filesize limits
 *
 *	Copyright (C) 1987 by Paul Sutcliffe, Jr.  (devon!paul)
 *
 *	Permission is hereby granted to copy, reproduce, redistribute or
 *	otherwise use this software as long as: there is no monetary
 *	profit gained specifically from the use or reproduction or this
 *	software, it is not sold, rented, traded or otherwise marketed, and
 *	this copyright notice is included prominently in any copy made.
 */

#ifndef lint
static char *sccsid = "@(#)ulimit.c	2.1  (devon)  1/7/86";
#endif

#include <sys/ulimit.h>

extern int getuid();
extern long ulimit();

main(argc, argv)
int argc;
char *argv[];
{
	char	*myname, *file;
	long	curlimit, newlimit;

	myname = argv[0];		/* program name */

	if (argc == 1) {	/* show current ulimit */
		if ((curlimit = ulimit(UL_GFILLIM, (long) 0)) < 0) {
			perror(myname);
			exit(1);
		}
		(void) printf("%ld\n", curlimit);

	} else if (argc >= 3) {		/* set new ulimit and run command */
		newlimit = atol((++argv)[0]);
		if (ulimit(UL_SFILLIM, newlimit) < 0) {
			perror(myname);
			exit(1);
		}
		(void) setuid(getuid());  /* run as real user, not root! */
		file = (++argv)[0];
		execvp(file, argv);
		perror(myname);		/* in case execvp fails */
		exit(1);

	} else	exit(2);
}
