#
#define ERROR	-1
#define ENOEXEC	8

/*
 * notty <command>
 *
 *	run <command> with no controlling terminal
 */

main(argc, argv)
char **argv;
{

	if (argc < 2) {
		err("usage: ", *argv, " <command>\n", 0);
		exit(ERROR);
	}
	argv++[argc--] = 0;

	clrtty();

/*      setuid(getuid());       /* non-su can clrtty() at Purdue */

	execute(*argv, argv);
	err(*argv, ": not found\n", 0);
	exit(ERROR);
}

execute(ap, aargv)
char *ap, **aargv;
{
	register char *p1, *p2;
	register n;
	char buf[128];
	static char pwbuf[65] 0;

	exec2(ap, aargv);

	if (*ap == '/')
		return;
	if (*pwbuf == 0) {
		n = dup(0);	/* find fd getpw will use */
		close(n);
		if (getpw(geteuid(), pwbuf))
			return;
		close(n);	/* in case getpw() left open */
	}

	p1 = pwbuf;
	n = 6;	/* tgi::14:1::/a/tgi:/a/tgi/bin/.startup */
	while (--n) /*       ^ */
		while (*p1++ != ':');

	p2 = buf;
	while ((*p2++ = *p1++) != ':');
	p2--;

	p1 = "/bin/";
	while (*p1)
		*p2++ = *p1++;

	p1 = ap;
	while (*p2++ = *p1++);

	exec2(buf, aargv);

	p1 = "/usr/bin/";
	p2 = buf;
	while (*p1)
		*p2++ = *p1++;
	p1 = ap;
	while (*p2++ = *p1++);

	exec2(&buf[4], aargv);
	exec2(buf, aargv);
}

static exec2(as, aargv)
char *as, **aargv;
{
	extern errno;

	execv(as, aargv);
	if (errno == ENOEXEC)
		exec3(as, aargv);
	return(errno);
}

static exec3(as, aargv)
char *as, **aargv;
{
	register char **p1, **p2;
	char *argt[512];

	p1 = &aargv[1];
	p2 = argt;
	*p2++ = "sh";
	*p2++ = as;
	while (*p1)
		*p2++ = *p1++;
	execv("/bin/sh", argt);
}

err(asl) {
	register char **a, *p;

	for (a = &asl; *a; a++) {
		for (p = *a; *p; p++);
		write(2, *a, p - *a);
	}
}
