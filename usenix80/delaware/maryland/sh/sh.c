#
/*
 *   Improved shell - John Levine, Yale University
 */
/*
 *      Improvements and bug fixes made by Robert L. Kirby
 *      on July 12, 1978:
 *      1. Save interrupt status with -c option so that shell created with
 *         interrupts ignored will create processes that do the same.
 *      2. Don't unnecessarily close file 2 so users can see error
 *         messages from within shell command files.
 *         /usr/source/s1/init.c must also be changed by adding
 *         a second "dup(0);" in dfork
 *         and a second "dup(0);" after the "single:" case.
 *      3. Alter output file create (">") mode to 0664.
 *      4. Fix newbin so that it closes file descriptors when done.
 *      5. Use logout even when YALE is not defined.
 *      6. If -c option is used .profile is not.
 *      on July 14, 1978:
 *      7. Allow "next" with -c option.
 *      8. Allow new lines ("\n") to be escaped with -t option
 *      9. Change setexit to signal in unnext() - bug fix.
 *      10. Handle QUIT interrupt in next() and unnext().
 *      On July 24, 1978:
 *      11. Use readnl for input speed improvement (* 2).
 *      On October 2, 1978:
 *      12. Use status of last call when leaving.
 *	On January 11, 1980:
 *	13. Repair setexit-syntax argument mismatch bug.
 */
/* #define YALE    0 RLK */      /* turn on special Yale nonsense */
#define READNL  0       /* Use if readnl() is available */
#define INBUFL  132

#define LINSIZ 1000
#define ARGSIZ 400
#define TRESIZ 500
#define HANGUP	1
#define INTR	2
#define QUIT	3

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
#define ENOMEM	12
#define ENOEXEC 8
#define echo(xchar) if(eflag) putc(xchar)

char    shnam[] "/bin/sh"; /*name of shell */
char	globnam[] "/etc/glob";	/* name of glob */
char    bin[32];        /* 256 bits that are set for users bin dir */
char    pwbuf[80];      /* for password file entry and then current dir */
char    dfltfile[30];   /* for default users bin directory */
char    *vbls[52];      /* shell variables */
char    *dolp;
char    pidp[6];
int     ldivr;
char    **dolv;
int     dolc;
#define promp   vbls['M'-'A']   /* prompt is now $M */
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
int     ostatus;        /* save status returned by last wait */
int     savquit;        /* save quit interrupt handling RLK */
int     savintr;        /* save del interrupt handling RLK */
char    oldin;          /* Original input file descriptor RLK */
#ifdef  READNL
int     icount;
#endif

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
	0,
	"Sig 14",
	"Sig 15",
	"Sig 16",
	"Sig 17",
	"Sig 18",
	"Sig 19",
};

struct stime {
	int proct[2];
	int cputim[2];
	int systim[2];
} timeb;

char    line[LINSIZ];
char    *args[ARGSIZ];
int     trebuf[TRESIZ];

main(c, av)
int c;
char **av;
{
	register f;
	register char *acname, **v;

	setdflt();
	newbin();
/* Changed to close from 3 instead of 2 RLK */
	for(f=3; f<15; f++)
		close(f);
	if((f=dup(1)) != 2)
		close(f);
	/* Save old standard input file RLK */
	oldin = dup(0);
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
		promp = "# ";
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
	if(**v == '-') {
		setintr++;
		/* save initial interrupt handling status (0 or 1) RLK */
		savquit = signal(QUIT, 1);
		savintr = signal(INTR, 1);
		/* If -c option, .profile not used. RLK */
		if(v[0][1] == 0 && arginp == 0 &&
		   (f = open(".profile",0)) >= 0)
			next(f);
	}
	dolv = v+1;
	dolc = c-1;
	vbls['N'-'A'] = putn(dolc);

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
			/* 2 params added as bug fix to match syntax call */
			setexit(args, argp);
			if (error) {
				ostatus = 077; /* Set return status */
				return;
			}
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
		echo("'}'");
	}
	c = readc();
	if(c == '\\') {
		c = readc();
		if(c == '\n') {
		/* Allow new lines to be escaped with -t option. RLK */
			if(onelflg == 1) onelflg++;
			return(' ');
		}
		return(c|QUOTE);
	}
	if(c == '$') {
		c = readc();
		if(c>='0' && c<='9') {
			if(c-'0' < dolc) {
				dolp = dolv[c-'0'];
				echo("'{'");
			}
			goto getd;
		}
		if(c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z') {
			if(c >= 'a')
				c =+ 'Z'+1 - 'a';
			if(vbls[c-'A'] == 0)
				goto getd;      /* undefined */
			dolp = vbls[c-'A'];
			echo("'{'");
			goto getd;
		}
		if(c == '$') {
			dolp = pidp;
			echo("'{'");
			goto getd;
		}
	}
	return(c&0177);
}

readc()
{
#ifndef READNL
	char cc;
#endif
	register c;
#ifdef  READNL
	static char inbuff[INBUFL];
	static char *inbufp;
#endif

	/* "if" condition extended to allow "next" with -c option RLK */
	if (arginp && sinfil == 0) {
		if (arginp == 1)
			exit(ostatus);
		if ((c = *arginp++) == 0) {
			arginp = 1;
			c = '\n';
		}
		echo(c);
		return(c);
	}
	if (onelflg==1)
		exit(ostatus);
#ifndef READNL
	if(read(0, &cc, 1) != 1) {
#endif
#ifdef  READNL
reread:
	if(--icount >= 0) c = *inbufp++;
	else
	if((icount = readnl(0, inbuff, INBUFL)) > 0) {
		inbufp = inbuff;
		goto reread;
	} else {
#endif
		if(sinfil) {	/* end of next file */
			/* Ignore interrupts first. RLK */
			if(setintr) {
				signal(INTR,1);
				signal(QUIT,1);
			}
			close(0);
			dup(sinfil);
			close(sinfil);
			sinfil = 0;
			if(promp = rpromp)
				prs(promp);
			return(readc());
		}
		exit(0);
	}
#ifndef READNL
	if (cc=='\n' && onelflg)
		onelflg--;
	echo(cc);
	return(cc);
#endif
#ifdef  READNL
	if (c=='\n' && onelflg)
		onelflg--;
	echo(c);
	return(c);
#endif
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
		/*
		 * "<<" detected for return to original standard input
		 * "if" condition extended by RLK
		 */
			if(any(**p, "<>(")
				&& !(c == '<' && **p == '<'))
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
			else if(chdir(pwbuf) < 0)
				err("chdir: bad directory");
			return;
		}
		if(equal(cp1, "newbin")) {
			newbin();
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
#ifndef YALE		/* Yale doesn't have newgrp */
		if(equal(cp1, "newgrp")) {
			if(promp != 0) {
				close(acctf);
				execv("/bin/newgrp", t+DCOM);
			}
			prs("newgrp: cannot execute\n");
			return;
		}
#endif
#ifdef YALE
		if(equal(cp1, "home")) {
			execl("/bin/home","home",0);
			prs("home: cannot execute\n");
			return;
		}
#endif
		if(equal(cp1, "logout"))
			exit(0);
/* #endif       Moved up by RLK */
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
			if(setintr) {
				signal(INTR,unnext);
				signal(QUIT,unnext);
			}
			pwait(i, 0);
			if(setintr) {
				signal(INTR,1);
				signal(QUIT,1);
			}
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
		if(t[DLEF] != 0) {
			close(0);
		/*
		 * Decide if the previous standard input or a file
		 * was requested. RLK
		 */
			i = (equal(t[DLEF], "<")?
				dup(oldin): open(t[DLEF], 0));
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
		/* Changed from 0644 by RLK */
			i = creat(t[DRIT], 0664);
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
/*
 *                        signal(INTR, 0);
 *                        signal(QUIT, 0);
 */
	/* Restore initial interrupt handling for new process. RLK */
			signal(INTR, arginp ? savintr : 0);
			signal(QUIT, arginp ? savquit : 0);
	/* End of altered code. */
		}
		if(t[DTYP] == TPAR) {
			if(t1 = t[DSPR])
				t1[DFLG] =| f&FINT;
			execute(t1);
			exit();
		}
		/* Remove extra old standard input file descriptor RLK */
		if(oldin > 2) close (oldin);
		close(acctf);
		gflg = 0;
		scan(t, &tglob);
		if(gflg) {
	/* tell glob about user bin dir if needed */
			t[DFLG] = globnam;
			t[DSPR] = hashme(t[DCOM])? dfltfile: "";
			execv(t[DFLG], t+DFLG);
			prs("glob: cannot execute\n");
			exit();
		}
		scan(t, &trim);
		*linep = 0;
		cp2 = t[DCOM];                          /* khd added */
		texec(cp2, t);
		if (*cp2 != '/') {                      /* khd added */
			if (hashme(cp2)) {
				cp1 = linep;
				cp2 = dfltfile;
				while (*cp1 = *cp2++) cp1++;
				cp2 = t[DCOM];
				while (*cp1++ = *cp2++) ;
				texec(linep, t);
			}
			cp1 = linep;
			cp2 = "/usr/bin/";
			while (*cp1 = *cp2++) cp1++;
			cp2 = t[DCOM];
			while (*cp1++ = *cp2++);
			texec(linep+4, t);
			texec(linep, t);
		}
		prs(t[DCOM]);
		err(": not found");
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

texec(f, at)
int *at;
{
	extern errno;
	register int *t;

	t = at;
	execv(f, t+DCOM);
	if (errno==ENOEXEC) {
		if (*linep)
			t[DCOM] = linep;
		t[DSPR] = shnam;
		execv(t[DSPR], t+DSPR);
		prs("No shell!\n");
		exit();
	}
	if (errno==ENOMEM) {
		prs(t[DCOM]);
		err(": too large");
		exit();
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
	while(*p)
		i =+ *p++;
	return(bin[(i>>3)&037] & 1<<(i&07));
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

	if(i != 0)
	for(;;) {
		times(&timeb);
		time(timeb.proct);
		p = wait(&ostatus);
		if(p == -1)
			break;
		e = ostatus&0177;
		if(mesg[e] != 0) {
			if(p != i) {
				prn(p);
				prs(": ");
			}
			prs(mesg[e]);
			if(ostatus&0200)
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

setdflt()
{
	register int i;
	register char *s, *t;

#ifdef YALE
	if(getpwg(getuid()&0377, getgid()&0377, pwbuf)) {
#endif
#ifndef YALE
	if(getpw(getuid()&0377,pwbuf)) {
#endif
		pwbuf[0] = 0;
		return;
	}
	s = pwbuf;
	t = pwbuf;
	i = 5;
	do {
		while (*t++ != ':');
	} while (--i);  /* ignore 5 ':' */
	while (*t != ':')
		*s++ = *t++;  /* pick up name */
	*s = '\0';
	s = dfltfile;
	t = pwbuf;
	while (*t)
		*s++ = *t++;  /* pick up name */
	for (t = "/bin/"; *s++ = *t++;);  /* add on /bin/ */
}

newbin()
{       int a[9], f;
	register int c, i;
	register char *s;

	if(pwbuf[0] == 0)
		return;
	a[8] = 0;
	for(i=0; i<32; i++) bin[i] = 0; /* clear all the bits */
	if ((f = open(dfltfile,0)) >= 0) {
		seek(f,32,0); /* skip . and .. entries */
		while (read(f,a,16) == 16) {
			if (a[0]) {
				s = &a[1];  i = 0;
				while (c = *s++) i =+ c;
				bin[(i>>3)&037] =| (1<<(i&07));
			}
		}
	close(f);       /* Keep from wasting file descriptors RLK */
	}
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
		goto yuck;
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
	/* if -c option, retain shell entry interrupt handling. RLK */
	if(setintr) {
		if (savintr != 1 || arginp == 0) signal(INTR,unnext);
		/* Added to allow QUIT also. RLK */
		if (savquit != 1 || arginp == 0) signal(QUIT,unnext);
	}
}

unnext()
{

	/* Changed from setexit and put first. RLK */
	signal(INTR,1);
	/* Added to allow QUIT also RLK */
	signal(QUIT,1);
	seek(0,0,2);
#ifdef  READNL
	icount = 0;
#endif
}
