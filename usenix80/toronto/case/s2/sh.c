#

/*
 *	Case Shell
 *
 *	Bill Shannon, Greg Ordy - Case Western Reserve University
 *
 *	Based on Yale Shell by John Levine
 *
 *
 *		set command fixed to release variables correctly
 *		.login (or .profile) invoked on login
 *		.logout invoked on logout
 *		.path file as in PWB shell
 *
 *		The following shell variables are defined:
 *
 *			$B - directory search list for commands
 *			$E - name of effective uid
 *			$G - name of glob
 *			$I - login name
 *			$M - prompt string
 *			$N - number of args to shell
 *			$P - processes not waited for
 *			$S - login directory, effects cd without arg
 *			$T - login tty
 *			$U - user dir, .. from login dir
 *			$W - where, first component of login dir
 *			$Z - name of shell
 */

#include "errnos.h"

#define CASE	0	/* turn on special Case commands */

#define HANGUP	1
#define INTR	2
#define QUIT	3
#define LINSIZ 1000
#define ARGSIZ 50
#define TRESIZ 100

#define QUOTE 0200
#define FAND 1
#define FCAT 2
#define FPIN 4
#define FPOU 8
#define FPAR 16
#define FINT 32
#define FPRS 64
#define TCOM 1
#define TPAR 2
#define TFIL 3
#define TLST 4
#define DTYP 0
#define DLEF 1
#define DRIT 2
#define DFLG 3
#define DSPR 4
#define DCOM 5
#define echo(xchar) if(eflag) putc(xchar)

char	lilobuff[40];	/* login and logout buffer for .login and .logout */
char	logoflg;	/* logging out flag */
char    *vbls[52];      /* shell variables */
char    *dolp;
char    pidp[6];
int     ldivr;
char    **dolv;
int     dolc;

#define	pathstr		vbls['B'-'A']
#define	ename		vbls['E'-'A']
#define	globnam		vbls['G'-'A']
#define	lname		vbls['I'-'A']
#define	promp		vbls['M'-'A']
#define	nargs		vbls['N'-'A']
#define	procs		vbls['P'-'A']
#define	ldir		vbls['S'-'A']
#define	ltty		vbls['T'-'A']
#define	udir		vbls['U'-'A']
#define	where		vbls['W'-'A']
#define	shellnam	vbls['Z'-'A']

char    *linep;
char    *elinep;
char    **argp;
char    **eargp;
int     *treep;
int     *treeend;
char    peekc;
char    gflg;
char    error;
char    acctf;
char    uid;
char    setintr;
int	sinfil;		/* real standard input during a next */
int	eflag;		/* -e echo flag */
char	*rpromp;	/* real prompt string during a next */
char    *arginp;
int     onelflg;

char    *mesg[] {
	0,
	"Hangup",
	0,
	"Quit",
	"Illegal instruction",
	"Trace/BPT trap",
	"IOT trap",
	"EMT trap",
	"Floating exception",
	"Killed",
	"Bus error",
	"Memory fault",
	"Bad system call",
	"Broken Pipe",
	"Alarm Clock",
	"Sig 15",
	"Sig 16",
	0
};

struct stime {
	int proct[2];
	int cputim[2];
	int systim[2];
} timeb;

char    line[LINSIZ];
char    *args[ARGSIZ];
int     trebuf[TRESIZ];

extern char *logdir(), *logtty(), *logname(), *logename();



main(c, av)
int c;
char **av;
{
	register f;
	register char *acname, **v;
	int	i;

	for(f=2; f<15; f++)
		close(f);
	if((f=dup(1)) != 2)
		close(f);
	dolc = getpid();
	for(f=4; f>=0; f--) {
		dolc = ldiv(0, dolc, 10);
		pidp[f] = ldivr+'0';
	}
	v = av;
/*
 * special case for -e
 */
	if(c > 1 && v[1][0] == '-' && v[1][1] == 'e') {
		v[1] = v[0];
		v++;
		c--;
		eflag++;
	}
	acname = "/usr/adm/sha";
	promp = "% ";
	if(((uid = getuid())&0377) == 0)
		promp = "! ";
	acctf = open(acname, 1);
	if(c > 1) {
		if(!eflag)
			promp = 0;
		if (*v[1]=='-') {
			**v = '-';
			if (v[1][1]=='c' && c>2)
				arginp = v[2];
			else if (v[1][1]=='t')
				onelflg = 2;
		} else {
			close(0);
			f = open(v[1], 0);
			if(f < 0) {
				prs(v[1]);
				err(": cannot open");
				return(1);
			}
		}
	}

	ldir = logdir();
	ltty = logtty();
	lname = logname();
	ename = logename();
	globnam = "/etc/glob";
	pexinit();
	setwhere();

	if(**v == '-') {
		setintr++;
		signal(QUIT, 1);
		signal(INTR, 1);
		/* mods to set up .login and .logout */
		if(v[0][1] == 0)
		{
			copyit(ldir,lilobuff);
			i = 0;
			while(lilobuff[i]) i++;
			lilobuff[i] = '/';
			copyit(".login",&lilobuff[i + 1]);
			if ((f = open(lilobuff,0)) >= 0)
				next(f);
			else {
				copyit(".profile", &lilobuff[i + 1]);
				if ((f = open(lilobuff, 0)) >= 0)
					next(f);
			}
			copyit(".logout",&lilobuff[i + 1]);
		};
	}
	dolv = v+1;
	dolc = c-1;
	nargs = putn(dolc);

loop:
	if(promp != 0)
		prs(promp);
	peekc = getch();
	main1();
	goto loop;
}

main1()
{
	register char c, *cp;
	register *t;

	argp = args;
	eargp = args+ARGSIZ-5;
	linep = line;
	elinep = line+LINSIZ-5;
	error = 0;
	gflg = 0;
	do {
		cp = linep;
		word();
	} while(*cp != '\n');
	treep = trebuf;
	treeend = &trebuf[TRESIZ];
	if(gflg == 0) {
		if(error == 0) {
			setexit();
			if (error)
				return;
			t = syntax(args, argp);
		}
		if(error != 0)
			err("syntax error"); else
			execute(t);
	}
}

word()
{
	register char c, c1;

	*argp++ = linep;

loop:
	switch(c = getch()) {

	case ' ':
	case '\t':
		goto loop;

	case '\'':
	case '"':
		c1 = c;
		while((c=readc()) != c1) {
			if(c == '\n') {
				error++;
				peekc = c;
				return;
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
	for(;;) {
		c = getch();
		if(any(c, " '\"\t;&<>()|^\n")) {
			peekc = c;
			if(any(c, "\"'"))
				goto loop;
			*linep++ = '\0';
			return;
		}
		*linep++ = c;
	}
}

tree(n)
int n;
{
	register *t;

	t = treep;
	treep =+ n;
	if (treep>treeend) {
		prs("Command line overflow\n");
		error++;
		reset();
	}
	return(t);
}

getch()
{
	register char c;

	if(peekc) {
		c = peekc;
		peekc = 0;
		return(c);
	}
	if(argp > eargp) {
		argp =- 10;
		while((c=getch()) != '\n');
		argp =+ 10;
		err("Too many args");
		gflg++;
		return(c);
	}
	if(linep > elinep) {
		linep =- 10;
		while((c=getch()) != '\n');
		linep =+ 10;
		err("Too many characters");
		gflg++;
		return(c);
	}
getd:
	if(dolp) {
		c = *dolp++;
		if(c != '\0') {
			echo(c);
			return(c);
		}
		dolp = 0;
		echo('}');
	}
	c = readc();
	if(c == '\\') {
		c = readc();
		if(c == '\n')
			return(' ');
		return(c|QUOTE);
	}
	if(c == '$') {
		c = readc();
		if(c>='0' && c<='9') {
			if(c-'0' < dolc) {
				dolp = dolv[c-'0'];
				echo('{');
			}
			goto getd;
		}
		if(c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z') {
			if(c >= 'a')
				c =+ 'Z'+1 - 'a';
			if(vbls[c-'A'] == 0)
				goto getd;      /* undefined */
			dolp = vbls[c-'A'];
			echo('{');
			goto getd;
		}
		if(c == '$') {
			dolp = pidp;
			echo('{');
			goto getd;
		}
	}
	return(c&0177);
}

readc()
{
	char cc;
	register c;

	if (arginp) {
		if (arginp == 1)
			exit();
		if ((c = *arginp++) == 0) {
			arginp = 1;
			c = '\n';
		}
		echo(c);
		return(c);
	}
	if (onelflg==1)
		exit();
	if(read(0, &cc, 1) != 1) {
		if(sinfil) {	/* end of next file */
			if (logoflg) logout();	/* log him out */
			close(0);
			dup(sinfil);
			close(sinfil);
			sinfil = 0;
			if(setintr)
				signal(INTR,1);
			if(promp = rpromp)
				prs(promp);
			return(readc());
		}
		exit(0);
	}
	if (cc=='\n' && onelflg)
		onelflg--;
	echo(cc);
	return(cc);
}

/*
 * syntax
 *      empty
 *      syn1
 */

syntax(p1, p2)
char **p1, **p2;
{

	while(p1 != p2) {
		if(any(**p1, ";&\n"))
			p1++; else
			return(syn1(p1, p2));
	}
	return(0);
}

/*
 * syn1
 *      syn2
 *      syn2 & syntax
 *      syn2 ; syntax
 */

syn1(p1, p2)
char **p1, **p2;
{
	register char **p;
	register *t, *t1;
	int l;

	l = 0;
	for(p=p1; p!=p2; p++)
	switch(**p) {

	case '(':
		l++;
		continue;

	case ')':
		l--;
		if(l < 0)
			error++;
		continue;

	case '&':
	case ';':
	case '\n':
		if(l == 0) {
			l = **p;
			t = tree(4);
			t[DTYP] = TLST;
			t[DLEF] = syn2(p1, p);
			t[DFLG] = 0;
			if(l == '&') {
				t1 = t[DLEF];
				t1[DFLG] =| FAND|FPRS|FINT;
			}
			t[DRIT] = syntax(p+1, p2);
			return(t);
		}
	}
	if(l == 0)
		return(syn2(p1, p2));
	error++;
}

/*
 * syn2
 *      syn3
 *      syn3 | syn2
 */

syn2(p1, p2)
char **p1, **p2;
{
	register char **p;
	register int l, *t;

	l = 0;
	for(p=p1; p!=p2; p++)
	switch(**p) {

	case '(':
		l++;
		continue;

	case ')':
		l--;
		continue;

	case '|':
	case '^':
		if(l == 0) {
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
 *      ( syn1 ) [ < in  ] [ > out ]
 *      word word* [ < in ] [ > out ]
 */

syn3(p1, p2)
char **p1, **p2;
{
	register char **p;
	char **lp, **rp;
	register *t;
	int n, l, i, o, c, flg;

	flg = 0;
	if(**p2 == ')')
		flg =| FPAR;
	lp = 0;
	rp = 0;
	i = 0;
	o = 0;
	n = 0;
	l = 0;
	for(p=p1; p!=p2; p++)
	switch(c = **p) {

	case '(':
		if(l == 0) {
			if(lp != 0)
				error++;
			lp = p+1;
		}
		l++;
		continue;

	case ')':
		l--;
		if(l == 0)
			rp = p;
		continue;

	case '>':
		p++;
		if(p!=p2 && **p=='>')
			flg =| FCAT; else
			p--;

	case '<':
		if(l == 0) {
			p++;
			if(p == p2) {
				error++;
				p--;
			}
			if(any(**p, "<>("))
				error++;
			if(c == '<') {
				if(i != 0)
				        error++;
				i = *p;
				continue;
			}
			if(o != 0)
				error++;
			o = *p;
		}
		continue;

	default:
		if(l == 0)
			p1[n++] = *p;
	}
	if(lp != 0) {
		if(n != 0)
			error++;
		t = tree(5);
		t[DTYP] = TPAR;
		t[DSPR] = syn1(lp, rp);
		goto out;
	}
	if(n == 0)
		error++;
	p1[n++] = 0;
	t = tree(n+5);
	t[DTYP] = TCOM;
	for(l=0; l<n; l++)
		t[l+DCOM] = p1[l];
out:
	t[DFLG] = flg;
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
	while(p = *t++)
		while(c = *p)
			*p++ = (*f)(c);
}

tglob(c)
int c;
{

	if(any(c, "[?*"))
		gflg = 1;
	return(c);
}

trim(c)
int c;
{

	return(c&0177);
}

execute(t, pf1, pf2)
int *t, *pf1, *pf2;
{
	int i, f, pv[2];
	register *t1;
	register char *cp1, *cp2;
	extern errno;
	int unnext();

	if(t != 0)
	switch(t[DTYP]) {

	case TCOM:
		cp1 = t[DCOM];
		if(equal(cp1, "chdir")||equal(cp1,"cd")) {
			if(t[DCOM+1] != 0) {
				if(chdir(t[DCOM+1]) < 0)
				        err("chdir: bad directory");
			}
			else if(chdir(ldir) < 0)
				err("chdir: bad directory");
			return;
		}
		if(equal(cp1, "set")) {
			doset(t+DCOM);
			return;
		}
		if(equal(cp1, "shift")) {
			if(dolc < 1) {
				prs("shift: no args\n");
				return;
			}
			dolv[1] = dolv[0];
			dolv++;
			dolc--;
			xfree(vbls['N'-'A']);
			vbls['N'-'A'] = putn(dolc);
			return;
		}
		if(equal(cp1, "login")) {
			if(promp != 0) {
				close(acctf);
				execv("/bin/login", t+DCOM);
			}
			prs("login: cannot execute\n");
			return;
		}
		if(equal(cp1, "newgrp")) {
			if(promp != 0) {
				close(acctf);
				execv("/bin/newgrp", t+DCOM);
			}
			prs("newgrp: cannot execute\n");
			return;
		}
		if(equal(cp1,"logo") || equal(cp1,"logout"))  {
			if(promp !=  0) {
				if((lilobuff[0] != 0) && (f = open(lilobuff,0)) >= 0) {
					logoflg++;
					next(f);
					return;
				}
				logout();
			}
			prs("logout: cannot execute\n");
			return;
		}
#ifdef CASE
		if(equal(cp1, "maxp")) {
			if (uid == 0) {
				nice(-120);
				return;
			}
			prs("maxp: not found\n");
			return;
			}
		if(equal(cp1, "normp")) {
			if (uid == 0) {
				nice(0);
			}
			return;
		}
#endif
		if(equal(cp1, "next")) {	/* next file */
			if(!t[DCOM+1]) {
				err("next: arg count");
				return;
			}
			if(sinfil) {
				err("next: nesting not allowed");
				return;
			}
			if((f=open(t[DCOM+1],0))<0) {
				err("next: no file");
				return;
			}
			next(f);
			return;
		}
		if(equal(cp1, "wait")) {        /* wait by pid */
			if(t[DCOM+1]) {
				i = getn(t[DCOM+1]);
				if(i == 0)
				        return;
			}
			else
				i = -1;
			if(setintr)
				signal(INTR,unnext);
			pwait(i, 0);
			if(setintr)
				signal(INTR,1);
			return;
		}
		if(equal(cp1, ":"))
			return;

	case TPAR:
		f = t[DFLG];
		i = 0;
		if((f&FPAR) == 0)
			i = fork();
		if(i == -1) {
			err("try again");
			return;
		}
		if(i != 0) {
			if((f&FPIN) != 0) {
				close(pf1[0]);
				close(pf1[1]);
			}
			if((f&FPRS) != 0) {
				prn(i);
				prs("\n");
				xfree(vbls['P'-'A']);
				vbls['P'-'A'] = putn(i);
			}
			if((f&FAND) != 0)
				return;
			if((f&FPOU) == 0)
				pwait(i, t);
			return;
		}
		if (uid != 0) nice(4);
		if(t[DLEF] != 0) {
			close(0);
			i = open(t[DLEF], 0);
			if(i < 0) {
				prs(t[DLEF]);
				err(": cannot open");
				exit();
			}
		}
		if(t[DRIT] != 0) {
			if((f&FCAT) != 0) {
				i = open(t[DRIT], 1);
				if(i >= 0) {
				        seek(i, 0, 2);
				        goto f1;
				}
			}
			i = creat(t[DRIT], 0644);
			if(i < 0) {
				prs(t[DRIT]);
				err(": cannot create");
				exit();
			}
		f1:
			close(1);
			dup(i);
			close(i);
		}
		if((f&FPIN) != 0) {
			close(0);
			dup(pf1[0]);
			close(pf1[0]);
			close(pf1[1]);
		}
		if((f&FPOU) != 0) {
			close(1);
			dup(pf2[1]);
			close(pf2[0]);
			close(pf2[1]);
		}
		if((f&FINT)!=0 && t[DLEF]==0 && (f&FPIN)==0) {
			close(0);
			open("/dev/null", 0);
		}
		if((f&FINT) == 0 && setintr) {
			signal(INTR, 0);
			signal(QUIT, 0);
		}
		if(t[DTYP] == TPAR) {
			if(t1 = t[DSPR])
				t1[DFLG] =| f&FINT;
			execute(t1);
			exit();
		}
		close(acctf);
		gflg = 0;
		scan(t, &tglob);
		if(gflg) {
			t[DRIT] = globnam;
			t[DFLG] = pathstr;
			t[DSPR] = shellnam;
			execv(t[DRIT], t+DRIT);
			prs("glob: cannot execute\n");
			exit();
		}
		scan(t, &trim);
		pexec(t[DCOM], t+DCOM);
		exit();

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
		if(t1 = t[DLEF])
			t1[DFLG] =| f;
		execute(t1);
		if(t1 = t[DRIT])
			t1[DFLG] =| f;
		execute(t1);
		return;

	}
}


/*
 * pexec - path search and execute a file
 */

pexec(name, argv)
char *name, *argv[];
{
	extern errno;
	register char *cp;
	char tline[48];
	char txe2big, txeacces;
	int txtbsy;	/* kludge cntr for ETXTBSY fix */

	txeacces = txe2big = 0;
	txtbsy = 0;
	cp = pathstr;	/* normal case -- search */
	if (any('/', name)) {
		cp = "";	/* sh: exec only cmd name as given */
	}
	do {
		cp = pcat(cp, name, tline, sizeof tline);
	retry:
		execv(tline, argv);
		switch (errno) {
		case ENOEXEC:
			argv[-1] = shellnam;
			argv[0] = tline;
			execv(shellnam, &argv[-1]);
			die(shellnam, "No shell!");
		case EACCES:
			txeacces++;	/* file there, missing x (probably) */
			break;
		case ENOMEM:
			die(name, "too large");
		case E2BIG:
			txe2big++;
			break;
		case ETXTBSY:
			if ((txtbsy =+ 10) > 60)
				die(name, "text busy");
			sleep(txtbsy);
			goto retry;
		}
	} while(cp);
	if (txe2big)
		die(name, "arg list too long");
	if (txeacces)
		die(name, "file not executable");
	die(name, "not found");
}

die(str1, str2)
char *str1, *str2;
{
	prs(str1);
	prs(": ");
	err(str2);
	exit(1);
}

pcat(so1, so2, si, sz)
register char *so1, *so2;
char *si;
int sz;
{
	register char *s;

	s = si;
	while(*so1 != ':' && *so1 != '\0' && --sz) *s++ = *so1++;
	if(si != s && --sz > 0) *s++ = '/';
	while(*so2 && --sz > 0) *s++ = *so2++;
	if (--sz < 0) {
			*si = '\0';
			err("Command line overflow");
	}
	else *s = '\0';
	return *so1 ? ++so1 : 0;
}




/*
 *	pexinit: fills in pathstr and shellnam.
 *	may be invoked before fork to avoid unnecessary .path opening.
 *	returns 0 if OK, -1 if any error.
 */
pexinit()
{
	char pathbuf[128 + 16];
	register n, f;
	char *newpath, *newshell;
	char *p;
	extern char *logdir();

	newshell = "/bin/sh";
	pcat(logdir(), ".path", pathbuf, sizeof pathbuf);
	if ((f = open(pathbuf, 0)) < 0)
		newpath = getuid() & 0377 ? ":/bin:/usr/bin" : ":/bin:/usr/bin:/etc:/";
	else {
		n = read(f, pathbuf, sizeof pathbuf);
		close(f);
		if (n <= 0) {
			prs("cannot read .path"); prs("\n");
			return(-1);
		}
		if (pexline(pathbuf, pathbuf + n, 128, &newpath, &p)
		|| pexline(p, pathbuf + n, 16, &newshell, &p))
			return(-1);
	}
	xfree(pathstr);
	pathstr = alloc(size(newpath)+1);
	copyit(newpath, pathstr);
	xfree(shellnam);
	shellnam = alloc(size(newshell)+1);
	copyit(newshell, shellnam);
	return(0);
}

/*
 *	pexline: scan for a line (if any) beginning at ptr,
 *	ending at ptrlim -1, for line up to psize bytes long.
 *	convert it to string, return beginning addr in pret
 *	and addr of next line in pnext
 *	return 0 if OK (or not present), -1 if runs off end in middle of line
 *	or if line too long.
 */
pexline(ptr, ptrlim, psize, pret, pnext)
register char *ptr, *ptrlim;
int psize;
char **pret, **pnext;
{
	if (ptr >= ptrlim)
		return(0);
	*pret = ptr;
	if (ptrlim > ptr + psize)
		ptrlim = ptr + psize;
	for (; ptr < ptrlim; ptr++)
		if (*ptr == '\n') {
			*ptr++ = '\0';
			*pnext = ptr;
			return(0);
		}
	prs(".path too long"); prs("\n");
	return(-1);
}

err(s)
char *s;
{

	prs(s);
	prs("\n");
	if(promp == 0) {
		seek(0, 0, 2);
		if(rpromp == 0)
			exit(1);
	}
}

prs(as)
char *as;
{
	register char *s;

	s = as;
	while(*s)
		putc(*s++);
}

putc(c)
{

	write(2, &c, 1);
}

prn(n)
int n;
{
	register a;

	if(a=ldiv(0,n,10))
		prn(a);
	putc(lrem(0,n,10)+'0');
}

any(c, as)
int c;
char *as;
{
	register char *s;

	s = as;
	while(*s)
		if(*s++ == c)
			return(1);
	return(0);
}

equal(as1, as2)
char *as1, *as2;
{
	register char *s1, *s2;

	s1 = as1;
	s2 = as2;
	while(*s1++ == *s2)
		if(*s2++ == '\0')
			return(1);
	return(0);
}

pwait(i, t)
int i, *t;
{
	register p, e;
	int s;

	if(i != 0)
	for(;;) {
		times(&timeb);
		time(timeb.proct);
		p = wait(&s);
		if(p == -1)
			break;
		e = s&0177;
		if(mesg[e] != 0) {
			if(p != i) {
				prn(p);
				prs(": ");
			}
			prs(mesg[e]);
			if(s&0200)
				prs(" -- Core dumped");
		}
		if(e != 0)
			err("");
		if(i == p) {
			acct(t);
			break;
		} else
			acct(0);
	}
}

acct(t)
int *t;
{
	if(acctf<0)
		return;		/* don't waste time */
	if(t == 0)
		enacct("**gok"); else
	if(*t == TPAR)
		enacct("()"); else
	enacct(t[DCOM]);
}

enacct(as)
char *as;
{
	struct stime timbuf;
	struct {
		char cname[14];
		char shf;
		char uid;
		int datet[2];
		int realt[2];
		int bcput[2];
		int bsyst[2];
	} tbuf;
	register i;
	register char *np, *s;

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
	for (i=0; i<14; i++) {
		tbuf.cname[i] = *np;
		if (*np)
			np++;
	}
	tbuf.datet[0] = timbuf.proct[0];
	tbuf.datet[1] = timbuf.proct[1];
	tbuf.uid = uid;
	tbuf.shf = 0;
	if (promp==0)
		tbuf.shf = 1;
	seek(acctf, 0, 2);
	write(acctf, &tbuf, sizeof(tbuf));
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
	if(s==0) {
		err("missing number");
		return(0);
	}
	if(*s == '-') {
		s++;
		sign++;
	}
	n = 0;
	while(*s)
		if(*s >= '0' && *s <= '9')
			n = n*10 + *s++ - '0';
		else {
			err("Bad number");
			return(0);
		}
	if(sign)
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
	while(*s++)
		n++;
	return(n);
}

char *putp;

/* write a number into a string, clumsily */
putn(n)
{
	char *s;
	register m;

	s = putp = alloc(7);
	if((m=n)<0) {
		*putp++ = '-';
		m = -m;
	}
	putn2(m);
	*putp = 0;
	return(s);
}

/* more of putn */
putn2(an)
{
	register n;
	register m;

	if(an > 9)
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
	if(v[1] == 0) {
yuck:
		err("Bad set command");
		return;
	}
	r = v[1][0];
	if(r < 'A' || r > 'Z' && r < 'a' || r > 'z')
		goto yuck;
	if(r > 'Z')
		r =+ 'Z'+1 - 'a';
	r =- 'A';
	if(v[2] == 0)
		return;
	if(v[3] == 0)
	/* mod to release set variables */
	{
		if(vbls[r] != 0)
		{
			xfree(vbls[r]);
			vbls[r] = 0;
			return;
		};
		goto yuck;
	};
	switch(v[2][0]) {
	case '=':
		p = alloc(size(v[3])+1);
		copyit(v[3],p);
stash:
/*
 *	kludge to allow setting the prompt inside next files 
 */
		if(r == 'M'-'A' && sinfil) {
			xfree(rpromp);
			rpromp = p;
			return;
		}
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
copyit(ap,aq)
char *ap, *aq;
{
	register char *p, *q;

	p = ap;
	q = aq;

	while(*q++ = *p++);
}

/* special free routine */
xfree(c)
char *c;
{
	extern char end[];

	if(c >= end)
		free(c);
}

/* interpret the "next" command */
next(f)
{
	int unnext();

	sinfil = dup(0);	/* save real standard input */
	close(0);
	dup(f);
	close(f);
	rpromp = promp;		/* save prompt string */
	promp = 0;
	if(setintr)
		signal(INTR,unnext);
}

unnext()
{

	seek(0,0,2);
	signal(INTR,1);
}

/*
 * logout - cleanup for log out
 */
logout()
{
	close(acctf);
	exit(0);
}


/*
 * setwhere - init $W and $U
 */
setwhere()
{
	register char *p, *s, c;

	for (p = &ldir[1]; *p; p++)
		if (*p == '/') break;
	c = *p;
	*p = 0;
	where = alloc(p - ldir + 1);
	copyit(ldir, where);
	*p = c;
	for (s = p; *p; p++)
		if (*p == '/') s = p;
	c = *s;
	*s = 0;
	udir = alloc(s - ldir + 1);
	copyit(ldir, udir);
	*s = c;
}
