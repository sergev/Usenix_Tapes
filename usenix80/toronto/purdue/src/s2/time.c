#

/*
 * time [-NN[s]] [command]
 *
 *	% time			prints current time/date
 *	% time who		run "who" and print time used
 *	% time -10 who	run "who" with cpu time limit of 10 seconds
 */

#define ERR	19
#define ENOEXEC	8
#define TIMELIM		/* timelim() system call:
			   after n seconds, sends a signal
			   16 to the process */
long lmuls(), ldivs();
#ifdef TIMELIM
int	ticks	{0};
#endif

main(argc, argv)
char **argv;
{
	extern exit();
	struct {
		int user;
		int sys;
		long childuser;
		long childsys;
	} buffer;
	char status[2];
	register char *c;
	register i, p;
	long before, after;

	time(&before);
	signal(ERR, exit);
#ifdef TIMELIM
	if (argc > 1 && *argv[1] == '-') {
		--argc;
		c = ++*++argv;
		p = 0;
		while ('0' <= *c && *c <= '9')
			p = p * 10 + *c++ - '0';
		if (*c && *c != 's') {
			prints("bad digit in time\n");
			return(-1);
		}
/*
		if (*c)
			p *= 60;
*/
		if (p < 0 || p > 1092) {
			p = 1092;
			printf("1092 seconds is maximum time limit\n");
		}
		ticks = p;
	}
#endif
	if (argc <= 1) {
		prints(ctime(&before));
		return(0);
	}
	i = getpid();	/* parent */
	p = fork();
	if (p == -1) {
		prints("Try again.\n");
		return;
	}
	if (p == 0) {
		argv[argc] = 0;
	/* If you know what these do, deal with them as you see fit
		if (argv[argc-1] == -1)
			argv[argc-1] = -2;	*/
		execute(argv[1], &argv[1]);
		prints("Command not found.\n");
		kill(i, ERR);
		exit(1);
	}
	signal(1, 1);
	signal(2, 1);
	signal(3, 1);
	while (wait(status) != p);
	time(&after);
	if (status[0] != 0)
		perr(status[0]);
	times(&buffer);
	prints("\n");
	printt("real", lmuls(after-before, 60));
	printt("user", buffer.childuser);
	printt("sys ", buffer.childsys);
	exit(0);
}

#define MAXSIG	(sizeof tbl / sizeof tbl[0] - 1)
char	*tbl[] {
	0,			 /* 0 */
	"Hangup",		 /* 1 */
	"INTERRUPT!",		 /* 2 */
	"Quit",			 /* 3 */
	"Illegal instruction",	 /* 4 */
	"Trace/BPT trap",	 /* 5 */
	"IOT trap",		 /* 6 */
	"EMT trap",		 /* 7 */
	"Floating exception",	 /* 8 */
	"Killed",		 /* 9 */
	"Bus error",		/* 10 */
	"Memory fault",		/* 11 */
	"Bad system call",	/* 12 */
	"Broken pipe",		/* 13 */
	"Alarm clock",		/* 14 */
	"System shutdown",	/* 15 */
	"Time limit",           /* 16 */
};

perr(asig) {
	register sig, core;
	register char *p;
	char buf[4];

	sig = asig & 0177;
	core = asig & 0200;
	if (sig > MAXSIG) {
		p = &buf[4];
		*--p = '\0';
		do {
			*--p = sig % 10 + '0';
			sig /= 10;
		} while (sig);
		prints("Signal ");
		prints(p);
	} else
		prints(tbl[sig]);
	if (core)
		prints(" -- Core dumped\n");
	else
		prints("\n");
}

char quant[] { 6, 10, 10, 6, 10, 6, 10, 10, 10 };
char *pad  "000      ";
char *sep  "\0\0.\0:\0:\0\0";
char *nsep "\0\0.\0 \0 \0\0";

printt(s, a)
char *s;
long a;
{
	char digit[9];
	long b;
	register i;
	int c;
	int nonzero;

	for (i=0; i<9; i++) {
		b = ldivs(a, quant[i]);
		digit[i] = a - lmuls(b, quant[i]);
		a = b;
	}
	prints(s);
	nonzero = 0;
	while (--i>0) {
		c = digit[i]!=0 ? digit[i]+'0':
		    nonzero ? '0':
		    pad[i];
		prints(&c);
		nonzero =| digit[i];
		c = nonzero?sep[i]:nsep[i];
		prints(&c);
	}
	prints("\n");
}

prints(s)
char *s;
{
	register char *p;

	for (p = s; *p; p++);
	write(2, s, p - s);
}

/* long=long*short, long=long/short, to avoid floating point */
struct { int hi, lo; };

long lmuls(l, s)
long l;
{
	long p;
	if (l.lo < 0) l.hi++;
	p.hi = l.hi*s;
	p.lo = l.lo*s;
	p.hi =+ hmul(l.lo, s);
	return(p);
}

long ldivs(l, s)
long l;
{
	long q;
	long d;
	long mone;
	mone.hi = 0;
	mone.lo = 32768;
	d = lmuls(mone, s);
	for (q=0; l>=d; q=+mone)
		l =- d;
	q =+ ldiv(l.hi, l.lo, s);
	return(q);
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
#ifdef TIMELIM
	extern ticks;
#endif
	extern errno;

#ifdef TIMELIM
	if (ticks)
		timelim(ticks);
#endif
	execv(as, aargv);
#ifdef TIMELIM
	if (ticks)
		timelim(0);
#endif
	if (errno == ENOEXEC)
		exec3(as, aargv);
	return(errno);
}

static exec3(as, aargv)
char *as, **aargv;
{
#ifdef TIMELIM
	extern ticks;
#endif
	register char **p1, **p2;
	char *argt[512];

	p1 = &aargv[1];
	p2 = argt;
	*p2++ = "sh";
	*p2++ = as;
	while (*p1)
		*p2++ = *p1++;
#ifdef TIMELIM
	if (ticks)
		timelim(ticks);
#endif
	execv("/bin/sh", argt);
#ifdef TIMELIM
	if (ticks)
		timelim(0);
#endif
}
