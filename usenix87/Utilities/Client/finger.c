#ifndef lint
static char *sccsid = "@(#) finger.c 1.4 85/10/19";
#endif

/*
 * If there's an @ in the first argument, must be a network finger,
 * so use client to do it.  Otherwise, just exec /usr/ucb/finger.
 */
static char *FINGER = "/usr/ucb/finger";
main(argc, argv)
int argc;
char **argv;
{
	extern char *index();
	register char *host;

	if (argc == 2 && (host = index(argv[1], '@')) != 0) {
		*host++ = '\0';
		execlp("client", "client", "-p", "finger", host,
			argv[1], (char *)0);
		perror("client");
		exit(1);
	}
	execv(FINGER, argv);
	perror(FINGER);
	exit(1);
}
