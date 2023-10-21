/*
 *  Zonk - send a signal to all of a user's processes.
 */

#include <sys/types.h>
#include <sys/signal.h>
#include <stdio.h>
#include <pwd.h>

main(argc, argv)
	char **argv;
{
	int uid = -1;
	int sig = -1;
	int i, zot;
	struct passwd *pw;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') switch (argv[i][1]) {

		case 'u':	/* -u uid */
			if (uid >= 0 || ++i == argc) {
			usage:
				fprintf(stderr, "Usage: zonk [-SIGNAL] [name|-u uid]\n"),
				exit(1);
			}
			uid = atoi(argv[i]);
			continue;

		default:	/* signal of the form -SIGNAL or -n */
			if (sig >= 0)
				goto usage;
			sig = getsig(&argv[i][1]);
			if ((unsigned)sig >= NSIG)
				goto usage;
			continue;
		}
		/* login name */
		if (uid >= 0)
			goto usage;
		pw = getpwnam(argv[i]);
		if (pw == NULL) {
			fprintf(stderr, "zonk: No such person: %s\n", argv[i]);
			exit(1);
		}
		uid = pw->pw_uid;
	}
	if (uid < 0)
		uid = getuid();
	if (sig < 0)
		sig = SIGKILL;
	
	/*
	 * If uid looks like a system process, ask user to verify.
	 */
	if (uid <= 15) {
		printf("uid = %d (%s) -- sure you want to zonk? [ny] ",
			uid, (pw = getpwuid(uid)) == 0? "?": pw->pw_name);
		if ((getchar()|' ') != ('y'|' '))
			exit(1);
	}
	
	/* printf("zonk(%d, %d)\n", uid,sig);exit();*/

	if ((zot=zonk(uid,sig)) < 0)
		perror("zonk");
	else
		printf("Zonked processes for uid %d = %d\n", uid, zot );
	exit(zot <= 0);
}

char *sigs[] = { 0,
"HUP", "INT", "QUIT", "ILL", "TRAP", "IOT", "EMT", "FPE",	/* 1-8 */
"KILL", "BUS", "SEGV", "SYS", "PIPE", "ALRM", "TERM", 0,	/* 9-16 */
"STOP", "TSTP", "CONT", "CHLD", "TTIN", "TTOU", "TINT", "XCPU",	/* 17-24 */
"XFSZ", 0, 0, 0, 0, 0, 0, 0,					/* 25-31 */
};
	int
getsig(name)
	char *name;
{
	int i;

	if (*name >= '0' && *name <= '9')
		return atoi(name);
	for (i = 1; i < NSIG; i++)
		if (sigs[i] && strcmp(sigs[i], name) == 0)
			return i;
	return -1;
}
