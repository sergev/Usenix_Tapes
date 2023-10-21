#

/*
 *   Improved shell - John Levine, Yale University
 *     Starting '77:
 *
 *	Kids command - John Bruner
 *	  Modified to fork and wait for a dummy process
 *	  in order to collect all defunct processes.
 *	Time prompt - TGI	[ 06/18 02:44 ]
 *	Exec command - TGI	[ 06/24 19:12 ]
 *	Log command - TGI	[ 06/28 00:41 ]
 *	Fixed process status message
 *	  subscript error - TGI	[ 07/24 15:35 ]
 *	Disable auto-startup on "user" account
 *		- TGI		[ 07/28 14:59 ]
 *	Added time parameter $C
 *		- TGI		[ 09/05 17:37 ]
 *	Added chdir pathname stuff
 *		- TGI		[ 09/20 07:54 ]
 *	Added "vars" command to show $ parameters
 *	Added unquoting stuff to eliminate problems
 *	  with stuff like:  cat > 'filename'
 *		- from JJB	[ 10/14 10:09 ]
 *	Fixed "exec" command to do it right so
 *	  I/O redirection would work
 *	  Also, now one can "exec" a command file
 *		- TGI		[ 10/14 10:09 ]
 *	Modified quoting code to permit non-closure
 *	  of trailing quote before new-line
 *		- TGI		[ 10/18 08:22 ]
 *	Changed interrupt stuff so a little
 *	  more intelligent - if no args,
 *	  interrupts inhibited, if args,
 *	  interrupts not caught, etc...
 *		- from JJB	[ 10/24 10:38 ]
 *	Added $S and $R.  Fixed bug in newkid() which
 *	  was stepping all over core when his brat
 *	  table got full
 *		- TGI		[ 12/24 02:46 ]
 *     '78:
 *	Added "par" command to permit interactive
 *	  prompting/entry of $ parameters from command
 *	  files
 *		- TGI		[ 02/13 13:53 ]
 *	Added timeout exit feature - before tty read
 *	  an alarm() call is made to break out of read()
 *	  after $W seconds (default 15 minutes)
 *	  A warning message is issued, and after another
 *	  $V second delay, the shell exits
 *		- TGI		[ 02/17 19:32 ]
 *	Changed "chdir" so default is $D, not necessarily
 *	  login directory
 *	Added $B, default directory for "newbin"
 *		- TGI		[ 03/03 13:45 ]
 *	Attempt to fix random "Sig 108 -- Core dumped"
 *	  stuff	- TGI		[ 03/07 16:57 ]
 *	Added $E: exit() status, $Z: signal number, and
 *	  $O: pid of last collected process.
 *	  If $Z > 200, core file was created, and
 *	  $Z-200 is signal number.
 *		- TGI		[ 03/20 14:56 ]
 *	Added Signal 15: "System shutdown" message.
 *	  Also shell will print this message when signal
 *	  15 is received by self.
 *		- TGI		[ 03/23 15:47 ]
 *	Changed .startup stuff to use from $D/.startup
 *	  instead of current directory.
 *		- TGI		[ 04/13 11:04 ]
 *	Fixed newbin() to close directory when done
 *		- TGI		[ 04/24 15:11 ]
 *	Added Signal 16: "Time limit" message.
 *	  Added -X=string flag stuff to allow setting any
 *	  shell $ variable from the call.
 *		- TGI		[ 10/23 18:12 ]
 *
 *   System parameters:
 *	$B	default newbin directory
 *	$C	current time - HH:MM	("Clock")
 *	$D	default chdir directory
 *	$E	exit() status of most recently collected process
 *	$K	current date - MM/DD    ("Kalendar" ???)
 *	$M	prompt
 *	$N	number of arguments in shell call
 *	$O	pid of most recently collected process
 *	$P	most recently forked process-id (via '&')
 *	$R	random (decimal) digit
 *	$S	user name
 *	$T	last char of tty name
 *	$U	uid (decimal)
 *	$V	secondary timeout (seconds)
 *	$W	timed tty read delay before exit() (alarm() sys call)
 *	$Z	signal that caused last collected process to blow up
 *		  if >200, core file created, $Z-200 is signal
 *	$0-9	arguments to shell call
 *	$$	process-id of shell
 */

/*#define YALE		/* turn on special Yale nonsense */
/*#define KID_FORK	/* inhibit fork/wait in "kids" */
#define USER	13	/* "user" account */
#define UID16		/* 16-bit uids, no gids */
#define ALARM		/* timed read from tty using alarm() */
#define PAR		/* enable "par" command */

#ifdef ALARM
#define TIMLOG	"900"	/* number of sec's until timeout logoff */
#define TIMWRN	"120"	/* number of sec's of warning before logoff */
#endif

#define SHNAM	"/bin/sh"
#define GLOBNAM	"/bin/glob"
#define ACNAM	"/usr/adm/sha"
#define PROMP	"% "
#define SUPROMP	"# "

#define HANGUP	1
#define SIGINT	2
#define SIGQIT	3
#define SIGCLK	14
#define SIGSHU	15
#define LINSIZ	1000
#define ARGSIZ	50
#define TRESIZ	100
#define PBUF	128
#define PCNT	30

#define QUOTE	0200
#define FAND	1
#define FCAT	2
#define FPIN	4
#define FPOU	8
#define FPAR	16
#define FINT	32
#define FPRS	64
#define TCOM	1
#define TPAR	2
#define TFIL	3
#define TLST	4
#define DTYP	0
#define DLEF	1
#define DRIT	2
#define DFLG	3
#define DSPR	4
#define DCOM	5
#define ENOMEM	12
#define ENOEXEC	8
#define MAXKIDS	64	/* this MUST be a power of two */

#define echo(xchar)	if (eflag) putc(xchar)

#ifdef PAR
char	*par_table[PCNT];	/* for par command */
#endif

char	shnam[] SHNAM;	/*name of shell */
char	globnam[] GLOBNAM;		/* name of glob */
char	bin[32];	/* 256 bits that are set for users bin dir */
char	pwbuf[80];  /* for password file entry and then current dir */
char	namebuf[9];	/* for $S - user name */
char	d_dfltfile[20];		/* for default users bin directory */
char	o_dfltfile[64];		/* for another users bin directory */
char	*dfltfile  d_dfltfile;	/* either d_dfltfile or $B */
char	*vbls[52];		/* shell variables */
char	*dolp;
char	pidp[6];
int	ldivr;
char	**dolv;
int	dolc;
#define promp	vbls['M'-'A']	/* prompt is now $M */
char	*linep;
char	*elinep;
char	**argp;
char	**eargp;
int	*treep;
int	*treeend;
char	peekc;
char	gflg;
char	error;
char	acctf;
int	uid;
char	setintr;
int	sinfil;		/* real standard input during a xeq */
int	eflag;		/* -e echo flag */
char	*rpromp;	/* real prompt string during a xeq */
char	*arginp;
int	onelflg;
char	execflg;	/* if command was "exec", don't fork */
char	intf	0;	/* if interrupt was intercepted by unxeq() */

#define MAXSIG  16
char	*mesg[] {
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

char	bad_dir[]	"chdir: bad directory";
char	no_kids[]	"No known children\n";
char	nl[]	"\n";

struct stime {
	int proct[2];
	int cputim[2];
	int systim[2];
} timeb;

char	line[LINSIZ];
char	*args[ARGSIZ];
int	trebuf[TRESIZ];
int	kids[MAXKIDS];	/* hashed table of known child jobs */
int	numkids	0;	/* number of known, live children */

char	tty	0;	/* tty num, if any, of filedes 0 */
#ifdef ALARM
char	new_line  1;	/* for timed read stuff */
#endif
char	warned	0;	/* aborted read() by alarm() or sig 15 */

main(c, av)
int c;
char **av;
{
#ifdef ALARM
	int warn();
#endif
	int shut_down();
	register f;
	register char *p, **v;
	char *pp;

	for (f = 2; f < 15; f++)
		close(f);
	dup(1);
	vbls['T' - 'A'] = "x";
	if ((tty = *vbls['T' - 'A'] = ttyn(0)) == 'x')
		tty = 0;
	else {
#ifdef ALARM
		signal(SIGCLK, warn);
#endif
		signal(SIGSHU, shut_down);
	}
	vbls['D' - 'A'] = pwbuf;
	vbls['B' - 'A'] = dfltfile = d_dfltfile;
#ifdef UID16
	uid = getuid();
#endif
#ifndef UID16
	uid = getuid() & 0377;
#endif
	vbls['U' - 'A'] = putn(uid);
#ifdef ALARM
	vbls['W' - 'A'] = TIMLOG; /* default timed read delay 15 min */
	vbls['V' - 'A'] = TIMWRN; /* 2-minute warning */
#endif
	setdflt();
	close(3);	/* in case getpw() doesn't */
	vbls['S' - 'A'] = namebuf;
	newbin(0);
	dolc = getpid();
	for (f = 4; f >= 0; f--) {
		dolc = ldiv(0, dolc, 10);
		pidp[f] = ldivr + '0';
	}
	v = av;
	p = ACNAM;
	if (uid == 0)
		promp = SUPROMP;
	else
		promp = PROMP;
	acctf = open(p, 1);
	intf = 1;
loop:
	if (c > 1) {
		if (v[1][0] == '-') {
			f = v[1][1];
			if (v[1][2] == '=' && ('A' <= f && f <= 'Z' ||
			    'a' <= f && f <= 'z')) {
				vbls[f - 'A'] = &v[1][3];
				goto advance;
			}
			switch(f) {
			case 'e':
				eflag = 1;
			case '\0':
				intf = 0;
			advance:
				for (f = 2; f < c; f++)
					v[f - 1] = v[f];
				c--;
				goto loop;
			case 't':
				**v = '-';
				onelflg = 2;
				promp = 0;
				break;
			case 'c':
				**v = '-';
				if (c > 2) {
					arginp = v[2];
					promp = 0;
					break;
				}
			default:
				err("bad flag");
				exit(1);
			}
		} else {
			close(0);
			if (open(v[1], 0)) {
				prs(v[1]);
				err(": cannot open");
				exit(1);
			}
			if (!eflag)
				promp = 0;
		}
	}
out:
	if (intf)
		**v = '-';
	intf = 0;
	if (**v == '-') {
		setintr++;
		signal(SIGQIT, 1);
		signal(SIGINT, 1);
		if (v[0][1] == 0 && uid
#ifdef USER
		    && uid != USER
#endif
				  ) {
			f = p = alloc(size(vbls['D'-'A']) + 10);
			for (pp = vbls['D'-'A']; *pp;)
				*p++ = *pp++;
			for (pp = "/.startup"; *p++ = *pp++;);
			p = f;
			if ((f = open(p, 0)) >= 0) {
				xeq(f);
				close(f);
			}
		}
		xfree(p);
	}
	dolv = v + 1;
	dolc = c - 1;
	vbls['N'-'A'] = putn(dolc);

	setexit();

	for (;;) {
		if (promp) {
			if (*promp)
				prs(promp);
			else
				tpromp();
		}
		peekc = getch();
		main1();
	}
}

main1() {
	register char c, *cp;
	register *t;

	argp = args;
	eargp = args + ARGSIZ - 5;
	linep = line;
	elinep = line + LINSIZ - 5;
	error = 0;
	gflg = 0;
	do {
		cp = linep;
		word();
	} while (*cp != '\n');
	treep = trebuf;
	treeend = &trebuf[TRESIZ];
	if (gflg == 0) {
		if (error == 0)
			t = syntax(args, argp);
		if (error)
			err("Syntax error");
		else
			execute(t);
	}
}

word() {
	register char c, c1;

	*argp++ = linep;

loop:
	switch (c = getch()) {

	case ' ':
	case '\t':
		goto loop;

	case '\'':
	case '"':
		c1 = c;
		while ((c = readc()) != c1) {
			if (c == '\n') {
				peekc = c;
				goto pack;
			}
			*linep++ = c|QUOTE;
		}
		goto pack;

	case '&':
	case ';':
	case '<':
	case '>':
	case '(':
	case ')':
	case '|':
	case '^':
	case '\n':
		*linep++ = c;
		*linep++ = '\0';
		return;
	}

	peekc = c;

pack:
	for (;;) {
		c = getch();
		if (any(c, " '\"\t;&<>()|^\n")) {
			peekc = c;
			if (any(c, "\"'"))
				goto loop;
			*linep++ = '\0';
			return;
		}
		*linep++ = c;
	}
}

tree(n) {
	register *t;

	t = treep;
	treep =+ n;
	if (treep > treeend) {
		prs("Command line overflow\n");
		error++;
		reset();
	}
	return(t);
}

getch() {
	register char c;

	if (peekc) {
		c = peekc;
		peekc = 0;
		return(c);
	}
	if (argp > eargp) {
		argp =- 10;
		while ((c = getch()) != '\n');
		argp =+ 10;
		err("Too many args");
		gflg++;
		return(c);
	}
	if (linep > elinep) {
		linep =- 10;
		while ((c = getch()) != '\n');
		linep =+ 10;
		err("Too many characters");
		gflg++;
		return(c);
	}
getd:
	if (dolp) {
		c = *dolp++;
		if (c != '\0') {
			echo(c);
			return(c);
		}
		dolp = 0;
		echo('}');
	}
	c = readc();
	if (c == '\\') {
		c = readc();
		if (c == '\n')
			return(' ');
		return(c|QUOTE);
	}
	if (c == '$') {
		c = readc();
		if (c>='0' && c<='9') {
			if (c-'0' < dolc) {
				dolp = dolv[c-'0'];
				echo('{');
			}
			goto getd;
		}
		if (c == 'C' && (!vbls['C'-'A'] || vbls['C'-'A'] &&
		    !*vbls['C'-'A'])) {
			dolp = clock(0);
			echo('{');
			goto getd;
		}
		if (c == 'K' && (!vbls['K'-'A'] || vbls['K'-'A'] &&
		    !*vbls['K'-'A'])) {
			dolp = clock(1);
			echo('{');
			goto getd;
		}
		if (c == 'R' && (!vbls['R'-'A'] || vbls['R'-'A'] &&
		    !*vbls['R'-'A'])) {
			dolp = randigit(10);
			echo('{');
			goto getd;
		}
		if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z') {
			if (c >= 'a')
				c =+ 'Z'+1 - 'a';
			if (vbls[c-'A'] == 0)
				goto getd;	/* undefined */
			dolp = vbls[c-'A'];
			echo('{');
			goto getd;
		}
		if (c == '$') {
			dolp = pidp;
			echo('{');
			goto getd;
		}
	}
	return(c&0177);
}

readc() {
#ifdef ALARM
	int warn();
#endif
	char cc;
	register c;

	if (arginp) {
		if (arginp == 1)
			exit(0);
		if ((c = *arginp++) == 0) {
			arginp = 1;
			c = '\n';
		}
		echo(c);
		return(c);
	}
	if (onelflg == 1)
		exit(0);
#ifdef ALARM
	if (new_line && tty && vbls['W' - 'A'] && *vbls['W' - 'A'])
		if (c = getn(vbls['W' - 'A'])) {
			signal(SIGCLK, warn);
			alarm(c);
			warned = 0;
		} else {
			xfree(vbls['W' - 'A']);
			vbls['W' - 'A'] = 0;
		}
#endif
    tread:
	if (read(0, &cc, 1) != 1) {
		if (warned) {
			warned = 0;
			goto tread;
		}
		if (sinfil) {	/* end of xeq file */
			close(0);
			dup(sinfil);
			close(sinfil);
			sinfil = 0;
			if (setintr)
				signal(SIGINT, 1);
			if (promp = rpromp) {
				if (*promp)
					prs(promp);
				else
					tpromp();
			}
			return(readc());
		}
		exit(0);
	}
	if (cc == '\n')
#ifdef ALARM
			{
		if (tty) {
			alarm(0);
			new_line++;
		}
#endif
		if (onelflg)
			onelflg--;
#ifdef ALARM
	} else
		new_line = 0;
#endif
	echo(cc);
	return(cc);
}

/*
 * syntax
 *	empty
 *	syn1
 */

syntax(p1, p2)
char **p1, **p2;
{

	while (p1 != p2) {
		if (any(**p1, ";&\n"))
			p1++; else
			return(syn1(p1, p2));
	}
	return(0);
}

/*
 * syn1
 *	syn2
 *	syn2 & syntax
 *	syn2 ; syntax
 */

syn1(p1, p2)
char **p1, **p2;
{
	register char **p;
	register *t, *t1;
	int l;

	l = 0;
	for (p = p1; p != p2; p++)
	switch (**p) {

	case '(':
		l++;
		continue;

	case ')':
		l--;
		if (l < 0)
			error++;
		continue;

	case '&':
	case ';':
	case '\n':
		if (l == 0) {
			l = **p;
			t = tree(4);
			t[DTYP] = TLST;
			t[DLEF] = syn2(p1, p);
			t[DFLG] = 0;
			if (l == '&') {
				t1 = t[DLEF];
				t1[DFLG] =| FAND|FPRS|FINT;
			}
			t[DRIT] = syntax(p+1, p2);
			return(t);
		}
	}
	if (l == 0)
		return(syn2(p1, p2));
	error++;
}

/*
 * syn2
 *	syn3
 *	syn3 | syn2
 */

syn2(p1, p2)
char **p1, **p2;
{
	register char **p;
	register int l, *t;

	l = 0;
	for (p = p1; p != p2; p++)
	switch (**p) {

	case '(':
		l++;
		continue;

	case ')':
		l--;
		continue;

	case '|':
	case '^':
		if (l == 0) {
			t = tree(4);
			t[DTYP] = TFIL;
			t[DLEF] = syn3(p1, p);
			t[DRIT] = syn2(p+1, p2);
			t[DFLG] = 0;
			return(t);
		}
	}
	return(syn3(p1, p2));
}

/*
 * syn3
 *	( syn1 ) [ < in  ] [ > out ]
 *	word word* [ < in ] [ > out ]
 */

syn3(p1, p2)
char **p1, **p2;
{
	register char **p;
	char **lp, **rp;
	register *t;
	int n, l, i, o, c, flg;

	flg = 0;
	if (**p2 == ')')
		flg =| FPAR;
	lp = 0;
	rp = 0;
	i = 0;
	o = 0;
	n = 0;
	l = 0;
	for (p = p1; p != p2; p++)
	switch (c = **p) {

	case '(':
		if (l == 0) {
			if (lp)
				error++;
			lp = p+1;
		}
		l++;
		continue;

	case ')':
		l--;
		if (l == 0)
			rp = p;
		continue;

	case '>':
		p++;
		if (p != p2 && **p == '>')
			flg =| FCAT; else
			p--;

	case '<':
		if (l == 0) {
			p++;
			if (p == p2) {
				error++;
				p--;
			}
			if (any(**p, "<>("))
				error++;
			if (c == '<') {
				if (i)
					error++;
				i = *p;
				continue;
			}
			if (o)
				error++;
			o = *p;
		}
		continue;

	default:
		if (l == 0)
			p1[n++] = *p;
	}
	if (lp) {
		if (n)
			error++;
		t = tree(5);
		t[DTYP] = TPAR;
		t[DSPR] = syn1(lp, rp);
		goto out;
	}
	if (n == 0)
		error++;
	p1[n++] = 0;
	t = tree(n+5);
	t[DTYP] = TCOM;
	for (l=0; l < n; l++)
		t[l+DCOM] = p1[l];
out:
	t[DFLG] = flg;
	unquote(i);
	unquote(o);
	t[DLEF] = i;
	t[DRIT] = o;
	return(t);
}

scan(at, f)
int *at;
int (*f)();
{
	register char *p, c;
	register *t;

	t = at+DCOM;
	while (p = *t++)
		while (c = *p)
			*p++ = (*f)(c);
}

tglob(c) {

	if (any(c, "[?*"))
		gflg = 1;
	return(c);
}

trim(c) {

	return(c&0177);
}

execute(t, pf1, pf2)
int *t, *pf1, *pf2;
{
	int i, f, pv[2];
#ifndef KID_FORK
	int kidid;
#endif
	register *t1;
	register char *cp1, *cp2;
	char *p1, *p2;
	extern errno;
	int unxeq();
	int pid;

	execflg = 0;
	if (t)
	switch (t[DTYP]) {

	case TCOM:
		cp1 = t[DCOM];
		if (equal(cp1, "chdir") || equal(cp1, "cd")) {
			if (t[DCOM+1])
				curdir(t[DCOM+1]);
			else if (vbls['D' - 'A'] && *vbls['D' - 'A'])
				curdir(vbls['D' - 'A']);
			else if (chdir(pwbuf) < 0)
				err(bad_dir);
			return;
		}
		if (equal(cp1, "newbin")) {
			if (t[DCOM+1])
				newbin(t[DCOM+1]);
			else
				newbin(0);
			return;
		}
		if (equal(cp1, "set")) {
			doset(t+DCOM);
			return;
		}
		if (equal(cp1, "shift")) {
			if (dolc < 1) {
				prs("shift: no args\n");
				return;
			}
			if (dolv > &i)
				dolv[1] = dolv[0];
			dolv++;
			dolc--;
			xfree(vbls['N'-'A']);
			vbls['N'-'A'] = putn(dolc);
			return;
		}
		if (equal(cp1, "login")) {
			if (promp) {
				close(acctf);
				execv("/bin/login", t+DCOM);
			}
			prs("login: cannot execute\n");
			return;
		}
		if (equal(cp1, "qsh") || equal(cp1, "vars")) {
			vars(&t[DCOM+1]);
			return;
		}
		if (equal(cp1, "kids")) {
			if (numkids <= 0) {
				numkids = 0;
				prs(no_kids);
			} else {
#ifndef KID_FORK
				switch (kidid = fork()) {
				case -1:
					break;
				case  0:
					execl("/bin/yyyxxx","kids",0);
					exit(0);
				default:
					pwait(kidid, 0);
					if (numkids == 0) {
						prs(no_kids);
						return;
					}
				}
#endif
				prn(numkids);
				prs(" known child");
				if (numkids != 1)
					prs("ren");
				prs(nl);
				t1 = kids;
				for (i=0; i < numkids; i++) {
					while (*t1++ == 0);
					prn(*(t1-1));
					prs("  ");
				}
				prs(nl);
			}
			return;
		}
		if (equal(cp1, "exec")) {
			if (t[DCOM+1]) {
				execflg = 1;
				goto reglr;
			}
			return;
		}

#ifndef YALE		/* Yale doesn't have newgrp */
/*	Neither do we
		if (equal(cp1, "newgrp")) {
			if (promp) {
				close(acctf);
				execv("/bin/newgrp", t+DCOM);
			}
			prs("newgrp: cannot execute\n");
			return;
		}
 */
#endif
#ifdef YALE
		if (equal(cp1, "home")) {
			execl("/bin/home", "home", 0);
			prs("home: cannot execute\n");
			return;
		}
#endif
		if (equal(cp1, "logout") || equal(cp1, "log"))
			exit(0);
		if (equal(cp1, "xeq")) {	/* next file */
			if (!t[DCOM+1]) {
				err("xeq: arg count");
				return;
			}
			if (sinfil) {
				err("xeq: nesting not allowed");
				return;
			}
			if ((f = open(t[DCOM+1], 0)) < 0) {
				err("xeq: no file");
				return;
			}
			xeq(f);
			return;
		}
		if (equal(cp1, "wait")) {	/* wait by pid */
			if (t[DCOM+1]) {
				i = getn(t[DCOM+1]);
				if (i == 0) {
					err("Bad pid");
					return;
				}
			}
			else
				i = -1;
			if (setintr)
				signal(SIGINT, unxeq);
			pwait(i, 0);
			if (setintr)
				signal(SIGINT, 1);
			return;
		}
#ifdef PAR
		if (equal(cp1, "par")) {
			if (t[DCOM+1])
				par(t[DCOM+1]);
			else
				par("? ");
			return;
		}
#endif
		if (equal(cp1, ":"))
			return;

reglr:
	case TPAR:
		f = t[DFLG];
		i = 0;
		if ((f&FPAR) == 0)	/* don't fork if "exec" */
			i = execflg? 0 : fork();
		if (i == -1) {
			err("can't fork -- try again");
			return;
		}
		if (i) {
			newkid(i);
			if (f&FPIN) {
				close(pf1[0]);
				close(pf1[1]);
			}
			if (f&FPRS) {
				prn(i);
				prs(nl);
				xfree(vbls['P'-'A']);
				vbls['P'-'A'] = putn(i);
			}
			if (f&FAND)
				return;
			if ((f&FPOU) == 0)
				pwait(i, t);
			return;
		}
		if (t[DLEF]) {
			if ((i = open(t[DLEF], 0)) < 0) {
				prs(t[DLEF]);
				err(": cannot open");
				if (execflg) {
					close(i);
					return;
				}
				exit(1);
			}
			close(0);
			dup(i);
			close(i);
		}
		if (t[DRIT]) {
			if (f&FCAT) {
				if ((i = open(t[DRIT], 1)) > 0) {
					seek(i, 0, 2);
					goto f1;
				}
			}
			if ((i = creat(t[DRIT], 0644)) < 0) {
				prs(t[DRIT]);
				err(": cannot create");
				if (execflg) {
					close(i);
					return;
				}
				exit(1);
			}
		f1:
			close(1);
			dup(i);
			close(i);
		}
		if (f&FPIN) {
			close(0);
			dup(pf1[0]);
			close(pf1[0]);
			close(pf1[1]);
		}
		if (f&FPOU) {
			close(1);
			dup(pf2[1]);
			close(pf2[0]);
			close(pf2[1]);
		}
		if (f&FINT && t[DLEF] == 0 && (f&FPIN) == 0) {
			close(0);
			open("/dev/null", 0);
		}
		if ((f&FINT) == 0 && setintr) {
			signal(SIGINT, 0);
			signal(SIGQIT, 0);
		}
		if (t[DTYP] == TPAR) {
			if (t1 = t[DSPR])
				t1[DFLG] =| f&FINT;
			execute(t1);
			if (execflg)
				return;
			exit(1);
		}
		close(acctf);
		gflg = 0;
		scan(t, &tglob);
		if (gflg) {
	/* tell glob about user bin dir if needed */
			t[DFLG] = globnam;
			t[DSPR] = hashme(t[DCOM])? (*o_dfltfile?
			    o_dfltfile : dfltfile) : "";
			execv(t[DFLG], t+DFLG);
			prs("glob: cannot execute\n");
			if (execflg)
				return;
			exit(1);
		}
		scan(t, &trim);
		*linep = 0;
		cp2 = t[DCOM + execflg];	/* khd added */
		texec(cp2, t + execflg);
		if (*cp2 != '/') {	/* khd added */
			if (hashme(cp2)) {
				cp1 = linep;
				cp2 = *o_dfltfile? o_dfltfile :
				    dfltfile;
				while (*cp1 = *cp2++) cp1++;
				cp2 = t[DCOM + execflg];
				while (*cp1++ = *cp2++) ;
				texec(linep, t + execflg);
			}
			cp1 = linep;
			cp2 = "/usr/bin/";
			while (*cp1 = *cp2++) cp1++;
			cp2 = t[DCOM + execflg];
			while (*cp1++ = *cp2++);
			texec(linep+4, t + execflg);
			texec(linep, t + execflg);
		}
		prs(t[DCOM + execflg]);
		err(": not found");
		if (execflg)
			return;
		exit(1);

	case TFIL:
		f = t[DFLG];
		pipe(pv);
		t1 = t[DLEF];
		t1[DFLG] =| FPOU | (f&(FPIN|FINT|FPRS));
		execute(t1, pf1, pv);
		t1 = t[DRIT];
		t1[DFLG] =| FPIN | (f&(FPOU|FINT|FAND|FPRS));
		execute(t1, pv, pf2);
		return;

	case TLST:
		f = t[DFLG]&FINT;
		if (t1 = t[DLEF])
			t1[DFLG] =| f;
		execute(t1);
		if (t1 = t[DRIT])
			t1[DFLG] =| f;
		execute(t1);
		return;

	}
}

texec(f, at)
int *at;
{
	extern errno;
	register int *t;

	t = at;
	execv(f, t+DCOM);
	if (errno == ENOEXEC) {
		if (*linep)
			t[DCOM] = linep;
		t[DSPR] = shnam;
		execv(t[DSPR], t+DSPR);
		prs("No shell!\n");
		if (execflg)
			return;
		exit(1);
	}
	if (errno == ENOMEM) {
		prs(t[DCOM]);
		err(": too large");
		if (execflg)
			return;
		exit(1);
	}
}

/* hashing function for user bin dir */
hashme(ap)
char *ap;
{
	register char c, *p;
	register i;

	p = ap;
	i = 0;
	while (*p)
		i =+ *p++;
	return(bin[(i >> 3)&037] & 1 << (i&07));
}

err(s)
char *s;
{

	prs(s);
	prs(nl);
	if (promp == 0) {
		seek(0, 0, 2);
		if (rpromp == 0)
			exit(1);
	}
}

prs(as)
char *as;
{
	register char *s;

	s = as;
	while (*s)
		putc(*s++);
}

putc(c) {

	write(2, &c, 1);
}

prn(n) {
	register a;

	if (a = ldiv(0, n, 10))
		prn(a);
	putc(lrem(0, n, 10) + '0');
}

any(c, as)
char *as;
{
	register char *s;

	s = as;
	while (*s)
		if (*s++ == c)
			return(1);
	return(0);
}

equal(as1, as2)
char *as1, *as2;
{
	register char *s1, *s2;

	s1 = as1;
	s2 = as2;
	while (*s1++ == *s2)
		if (*s2++ == '\0')
			return(1);
	return(0);
}

pwait(i, t)
int *t;
{
	extern char *putp;
	register p, e;
	int s;

	if (i)
	for (;;) {
		times(&timeb);
		time(timeb.proct);
		s = 0;	/* attempted cure for SIG 119 -- Core dumped */
	    twait:
		p = wait(&s);
		if (p == -1) {
#ifdef ALARM
			if (warned) {
				warned = 0;
				goto twait;
			}
#endif
			break;
		}
		deadkid(p);
		e = s&0177;
		/* $Z = signal num of last collected process */
		xfree(vbls['Z'-'A']);
		vbls['Z'-'A'] = putn((s&0200? e+200 : e));
		/* $E = exit status of last collected process */
		xfree(vbls['E'-'A']);
		vbls['E'-'A'] = putn((s>>8)&0377);
		/* $O = pid of last collected process */
		xfree(vbls['O'-'A']);
		vbls['O'-'A'] = putn(p);
		if (e > MAXSIG || mesg[e]) {
			if (p != i) {
				prn(p);
				prs(": ");
			}
			if (e <= MAXSIG)
				prs(mesg[e]);
			else {
				prs("Sig ");
				prn(e);
			}
			if (s&0200)
				prs(" -- Core dumped");
		}
		if (e)
			err("");
		if (i == p) {
			acct(t);
			break;
		} else
			acct(0);
	}
}

acct(t)
int *t;
{
	if (acctf < 0)
		return;		/* don't waste time */
	if (t == 0)
		enacct("**gok"); else
	if (*t == TPAR)
		enacct("()"); else
	enacct(t[DCOM]);
}

enacct(as)
char *as;
{
	register i;
	register char *np, *s;
	struct stime timbuf;
	struct {
		char cname[14];
		char shf;
#ifdef UID16
		int uid;
#endif
#ifndef UID16
		char uid;
#endif
		int datet[2];
		int realt[2];
		int bcput[2];
		int bsyst[2];
	} tbuf;

	s = as;
	times(&timbuf);
	time(timbuf.proct);
	lsub(tbuf.realt, timbuf.proct, timeb.proct);
	lsub(tbuf.bcput, timbuf.cputim, timeb.cputim);
	lsub(tbuf.bsyst, timbuf.systim, timeb.systim);
	do {
		np = s;
		while (*s != '\0' && *s != '/')
			s++;
	} while (*s++ != '\0');
	for (i=0; i < 14; i++) {
		tbuf.cname[i] = *np;
		if (*np)
			np++;
	}
	tbuf.datet[0] = timbuf.proct[0];
	tbuf.datet[1] = timbuf.proct[1];
	tbuf.uid = uid;
	tbuf.shf = 0;
	if (promp == 0)
		tbuf.shf = 1;
	seek(acctf, 0, 2);
	write(acctf, &tbuf, sizeof(tbuf));
}

setdflt() {
	register int i;
	register char *s, *t;
	int tvec[2];

#ifdef YALE
	if (getpwg(getuid(), getgid(), pwbuf)) {
#endif
#ifndef YALE
	if (getpw(getuid(), pwbuf)) {
#endif
		pwbuf[0] = 0;
		return;
	}
	s = pwbuf;
	t = namebuf;
	while (*s && *s != ':')
		*t++ = *s++;
	*t = 0;
	s = pwbuf;
	t = pwbuf;
	i = 5;
	do {
		while (*t++ != ':');
	} while (--i);  /* ignore 5 ':' */
	while (*t != ':')
		*s++ = *t++;  /* pick up name */
	*s = '\0';
	s = *o_dfltfile? o_dfltfile : dfltfile;
	t = pwbuf;
	while (*t)
		*s++ = *t++;  /* pick up name */
	for (t = "/bin/"; *s++ = *t++;);  /* add on /bin/ */
	time(tvec);	/* for $R */
	srand(tvec[1]);
	srand(rand() ^ getpid());
}

newbin(ad)
char *ad;
{
	int a[9], f;
	register int c, i;
	register char *s;
	char *dir;

	if (ad) {
		dir = o_dfltfile;
		while (*dir++ = *ad++);
		if (dir[-2] != '/') {
			*--dir = '/';
			*++dir = 0;
		}
	}
	else {
		*o_dfltfile = 0;
		if (vbls['B' - 'A'] && *vbls['B' - 'A'])
			dfltfile = vbls['B' - 'A'];
		else
			dfltfile = d_dfltfile;
	}
	if (pwbuf[0] == 0)
		return;
	a[8] = 0;
	for (i=0; i < 32; i++) bin[i] = 0; /* clear all the bits */
	dir = *o_dfltfile? o_dfltfile : dfltfile;
	if ((f = open(dir, 0)) >= 0) {
		seek(f, 32, 0); /* skip . and .. entries */
		while (read(f, a, 16) == 16) {
			if (a[0]) {
				s = &a[1];  i = 0;
				while (c = *s++) i =+ c;
				bin[(i >> 3)&037] =| (1 << (i&07));
			}
		}
		close(f);
	} else if (ad)
		err("Bad newbin directory");
}

/* get a number from a string */
getn(as)
char *as;
{
	register n;
	register char *s;
	int sign;

	sign = 0;
	s = as;
	if (s == 0)
		return(0);
	if (*s == '-') {
		s++;
		sign++;
	}
	n = 0;
	while (*s)
		if (*s >= '0' && *s <= '9')
			n = n * 10 + *s++ - '0';
		else {
			err("Bad number");
			return(0);
		}
	if (sign)
		return(-n);
	return(n);
}

/* find length of a string */
size(as)
char *as;
{
	register n;
	register char *s;

	s = as;
	n = 0;
	while (*s++)
		n++;
	return(n);
}

char *putp;

/* write a number into a string, clumsily */
putn(n) {
	char *s;
	register m;

	if (n == 0100000)  /* -32768 is special */
		return("-32768");
	s = putp = alloc(7);
	if ((m = n) < 0) {
		*putp++ = '-';
		m = -m;
	}
	putn2(m);
	*putp = 0;
	return(s);
}

/* more of putn */
putn2(an) {
	register n;
	register m;

	if (an > 9)
		putn2(an/10);
	*putp++ = an%10 + '0';
}

/* actual set command */
doset(av)
char *av[];
{
	register char **v;
	register r;
	char *p;

	v = av;
	if (v[1] == 0) {
yuck:
		err("Bad set command");
		return;
	}
	r = *v[1];
	if (r < 'A' || r > 'Z' && r < 'a' || r > 'z')
		goto yuck;
	if (r > 'Z')
		r =+ 'Z' + 1 - 'a';
	r =- 'A';
	if ((!v[2] || !*v[2]) || *v[2] == '=' && (!v[3] || !*v[3])) {
		p = alloc(1);
		*p = 0;
		goto stash;
	}
	unquote(v[3]);
	switch (*v[2]) {
	case '=':
		/* $B special processing: ensure '/' on end */
		p = alloc(size(v[3]) + 1 + (r == 'B' - 'A'));
		copyit(v[3], p);
		if (r == 'B' - 'A' && p[size(p) - 1] != '/') {
			p[size(p) + 1] = '\0';
			p[size(p)] = '/';
		}
stash:
/*
 *	kludge to allow setting the prompt inside xeq files 
 */
		if (r == 'M' - 'A' && sinfil) {
			xfree(rpromp);
			rpromp = p;
			return;
		}
		if (r != 'M' - 'A' || vbls[r] != -1)
			xfree(vbls[r]);
		vbls[r] = p;
		return;
	case '+':
		p = putn(getn(vbls[r])+getn(v[3]));
		goto stash;
	case '-':
		p = putn(getn(vbls[r])-getn(v[3]));
		goto stash;
	default:
		goto yuck;
	}
}

/* fast string to string copy */
copyit(ap, aq)
char *ap, *aq;
{
	register char *p, *q;

	p = ap;
	q = aq;

	while (*q++ = *p++);
}

/* special free routine */
xfree(c)
char *c;
{
	int x;
	extern char end[];

	c =& ~01;
	if (c >= end && c < &x)
		free(c);
}

/* interpret the "xeq" command */
xeq(f) {
	int unxeq();

	sinfil = dup(0);	/* save real standard input */
	close(0);
	dup(f);
	close(f);
	rpromp = promp;		/* save prompt string */
	promp = 0;
	if (setintr)
		signal(SIGINT, unxeq);
}

unxeq() {

	signal(SIGINT, 1);
	seek(0, 0, 2);
	intf = 1;
}

deadkid(pid) {
	register int p, pp, *k;

	if (pid <= 0)
		return;
	numkids--;
	pp = pid;
	p = pp&(MAXKIDS - 1);
	if (kids[p] == pp) {
		kids[p] = 0;
		return;
	} else {
		for (k = &kids[p + 1]; k < &kids[MAXKIDS]; k++)  {
			if (*k == pp) {
				*k = 0;
				return;
			}
		}
		for (k = kids; k < &kids[p]; k++) {
			if (*k == pp) {
				*k = 0;
				return;
			}
		}
	}
	numkids++;    /* didn't delete one like we thought we would */
}

newkid(pid) {
	register int p, i, *k;

	numkids++;
	p = pid&(MAXKIDS - 1);
	if (kids[p] == 0) {
		kids[p] = pid;
		return(0);
	} else {
		for (k = &kids[p + 1]; k < &kids[MAXKIDS]; k++)
			if (*k == 0) {
				*k = pid;
				return(0);
			}
		for (k = kids; k < &kids[p]; k++)
			if (*k == 0) {
				*k = pid;
				return(0);
			}
		i = kids[p];
		kids[p] = pid;
		numkids--;	/* didn't add one after all... */
		return(i);
	}
}

tpromp() {
	if (uid) {
		prs(clock(0));
		prs("> ");
	} else {
		prs("[ ");
		prs(clock(0));
		prs(" ] ");
	}
}

clock(af) {
	register int *tp;
	register char *cp;
	int tvec[2];
	static char tbuf[6];

	time(tvec);
	tp = localtime(tvec) + 2 * sizeof cp;
	cp = tbuf;

	if (af) {
		tp =+ 2;
		++*tp;
		*cp++ = *tp / 10 + '0';
		*cp++ = *tp-- % 10 + '0';
		*cp++ = '/';
		*cp++ = *tp / 10 + '0';
		*cp++ = *tp % 10 + '0';
	} else {
		*cp++ = *tp / 10 + '0';
		*cp++ = *tp-- % 10 + '0';
		*cp++ = ':';
		*cp++ = *tp / 10 + '0';
		*cp++ = *tp % 10 + '0';
	}
	*cp = 0;
	return(tbuf);
}

randigit(ab) {
	register char *rbuf;

	rbuf = "0";
	*rbuf = (rand() >> 9) % ab + '0';
	return(rbuf);
}

curdir(as)
char *as;
{
	register char *s, *p;

	unquote(as);
	if (chdir(as) < 0) {
		s = p = as;
		while (*p)
			if (*p++ == '/')
				s = p;
		if (s == as || (s[-1] = 0) == *as) {
			err(bad_dir);
			return(-1);
		}
		if (curdir(as) == 0) {
			prs("chdir: ");
			err(as);
		}
		return(-1);
	}
	return(0);
}

/*
 * unquote -- remove quotes from a string
 *
 * this was added to eliminate problems like:
 *
 *	% prog >'file'
 *	% set v = 'file'
 *	% chdir 'dir'
 */

unquote(s)
char *s;
{
	register char *p;

	if (!s)
		return;
	for (p = s; *p; p++)
		*p =& 0177;
}

vars(ap)
char **ap;
{
	int unxeq();
	register i, low, high;
	int n;
	char **tbl, c;

	intf = 0;
	if (setintr)
		signal(SIGINT, unxeq);
	low = 0;
	high = 61;
	if (ap[0] && (c = *ap[0])) {
		high = low = val(c);
		if (ap[1] && (c = *ap[1]))
			high = val(c);
	}
	for (i = low; i <= high && !intf; i++) {
		if (0 <= i && i <= 9) {
			if (i >= dolc)
				continue;
			tbl = dolv;
			c = (n = i) + '0';
		} else if (10 <= i && i <= 61) {
			tbl = vbls;
			n = i - 10;
			if (i <= 35)
				c = n + 'A';
			else
				c = n + 'A' + 6;
		} else
			return;
		if (tbl[n] && *tbl[n])
			printf("$%c = %s\n", c, tbl[n]);
	}
	if (setintr)
		signal(SIGINT, 1);
}

val(c)
char c;
{
	if ('0' <= c && c <= '9')
		return(c - '0');
	if ('A' <= c && c <= 'Z')
		return(c - 'A' + 10);
	if ('a' <= c && c <= 'z')
		return(c - 'a' + 36);
	return(-1);
}

#ifdef PAR
par(ps)
char *ps;
{
	register char *p, c;
	register n;
	char c1, pbuf[PBUF];

	p = dolv;
	dolv = par_table;
	if (p > &c1) {
		for (n = 0; n < PCNT; n++)
			dolv[n] = 0;
		dolc = 0;
	}
	prs(ps);	/* issue prompt */
	p = pbuf;
	n = 0;
	while (c = get2()) {
	    loop:
		if (c == ' ' || c == '\t' || c == '\n') {
			if (p == pbuf && c == '\n')
				break;
			*p = '\0';
			p = alloc(size(pbuf) + 1);
			copyit(pbuf, p);
			xfree(dolv[n]);
			dolv[n++] = p;
			while (c == ' ' || c == '\t')
				c = get2();
			if (c == '\n')
				break;
			p = pbuf;
			goto loop;
		}
		if (c == '"' || c == '\'') {
			c1 = c;
			while ((c = get2()) && c != c1 && c != '\n') {
				*p++ = c;
				if (p >= &pbuf[PBUF - 2])
					goto too_long;
			}
			if (c == '\n')
				goto loop;
			continue;
		}
		if (c == '\\' && (c = get2()) == '\n')
			c = ' ';
		*p++ = c;
		if (p >= &pbuf[PBUF - 2]) {
	    too_long:	err("line too long");
			while ((c = get2()) && c != '\n');
			return;
		}
	}
	dolc = n;
	while (n < PCNT) {
		xfree(dolv[n]);
		dolv[n++] = 0;
	}
	xfree(vbls['N'-'A']);
	vbls['N'-'A'] = putn(dolc);
}

get2() {
	char c;

	if (read(2, &c, 1) != 1)
		return(0);
	return(c);
}
#endif

#ifdef ALARM
warn() {
	int bye();
	register n;

	signal(SIGCLK, bye);
	if (vbls['V' - 'A'] && *vbls['V' - 'A'] &&
	    (n = getn(vbls['V' - 'A'])) > 0) {
		prs("\n\7Timeout logout in ");
		printf("%d seconds\7\n", n);
		warned++;
		alarm(n);
	} else
		bye();
}

bye() {
	prs("\nTerminal timeout.\n");
	sleep(5);
	exit(0);
}
#endif

shut_down() {
	signal(SIGSHU, shut_down);
	prs("\nSystem shutdown requested by system manager...\n");
	prs("Please save files and log out.\n");
	warned++;
}
