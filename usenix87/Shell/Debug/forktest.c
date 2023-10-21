/* Fork Test: display args, open files, signals, etc.
 *
 * Simple as this program is, it has found bugs in the
 * way a number of programs fork off children.  To test
 * how a program is invoking its children, run this
 * program as a child.
 *
 * Generally, processes should be created with:
 *
 * - a reasonable arg count & list
 * - arg 0 should look like the name of the command
 *
 * - real and effective UIDs and GIDs should be reasonable.
 *   Beware setuid programs that fork children!
 *
 * - no pending alarm.  Version 7 apparently does not
 *   reset alarms upon an exec!
 *
 * - file descriptors 0 (STDIN), 1 (STDOUT), and 2 (STDERR)
 *   opened reasonably
 * - all other file descriptors closed (this program will
 *   describe all open channels)
 *
 * - all signals (except SIGKILL) set to SIG_DFL (this
 *   program will print all signals set otherwise)
 *
 * The output is fairly simple to understand.  When in
 * doubt, read the code (and a UNIX manual: exec(2),
 * getuid(2), alarm(2), signal(2), stat(2)).
 *
 * Room for Improvement:
 *
 * - strings should be printed in a way that shows funny characters.
 * - show misc. other bits of state
 *	- PID (who cares?)
 *	- umask
 *	- ulimit (System V)
 *	- stty settings of open TTYs
 *
 * Copyright (c) 1986 March 11  D. Hugh Redelmeier
 *
 * This program may be distributed and used without restriction.
 */

#include <stdio.h>

extern unsigned alarm();	/* should be unsigned, but may be int */

#include <sys/types.h>
#include <sys/stat.h>

struct stat sb;

#include <errno.h>
extern int errno;
extern char *sys_errlist[];

#include <signal.h>

int (*signal())();

main(argc, argv, envp)
int argc;
char **argv, **envp;
{
	register int i;
	unsigned al = alarm(0);	/* get it while it is hot */

	printf("%d arg(s):", argc);
	for (i=0; i<argc; i++)
		if (argv[i] == NULL)
			printf(" NULL!");
		else
			printf(" \"%s\"", argv[i]);
	printf("\n");
	if (argv[argc] != NULL)
		printf("Arg list is not ended with a NULL!\n");

	printf("Real UID = %d, GID = %d; Effective UID = %d, GID = %d.\n",
		getuid(), getgid(), geteuid(), getegid());

	if (al)
		printf("Alarm set to go off in %u seconds.\n", al);

	printf("File Descriptors:\n");
	for (i=0; i!=40; i++)	/* I hope 40 is enough. */
		if (fstat(i, &sb) == -1) {
			if (errno != EBADF)
				printf("\t%d error: %s\n", i, sys_errlist[errno]);
		} else {
			printf("\t%d: dev=%d, ino=%d, perm=0%04o, ",
				i, sb.st_dev, sb.st_ino, sb.st_mode&07777);
			switch (sb.st_mode & S_IFMT) {
			case S_IFREG:
				printf("pipe or regular file\n");
				break;
			/* extend as desired */
			default:
				printf("IFMT=0%o\n", sb.st_mode>>12);
				break;
			}
		}

	printf("Signals:\n");
	for (i=1; i!=40; i++) {	/* I hope 40 is enough. */
		register int n = (int) signal(i, SIG_IGN);
		switch (n) {
		case -1:
		case SIG_DFL:
			break;
		case SIG_IGN:
			printf("\t%d: SIG_IGN\n", i);
			break;
		default:
			printf("\t%d: %d\n", i, n);
			break;
		}
	}

	printf("Environment:\n");
	for (i=0; envp[i]!=NULL; i++)
		printf("\t\"%s\"\n", envp[i]);

	exit(0);
}
