#
#include	"ext.h"

/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
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

/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
/* print error msg on new line */
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

/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
prs(as)
char *as;
{
        register char *s;

        s = as;
        while(*s)
                putc(*s++);
}

/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
putc(c)
{

        write(2, &c, 1);
}

/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
prn(n)
int n;
{
        register a;

        if(a=ldiv(0,n,10))
                prn(a);
        putc(lrem(0,n,10)+'0');
}

/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
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

/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
/* check strings for equality */
/* right string determines limit of search */
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

/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
/* wait for specific child process to terminate */
/* do accounting for terminated process */
/* do any 'batch' (&) processes, too */
/* don't return until specific one exits */
pwait(i, t)
int i, *t;
{
        register p, e;
        int s;
	register char *cp1, *cp2;

        if(i != 0)
        for(;;) {
                times(&timeb);
                time(timeb.proct);
                p = wait(&s);
                if(p == -1)
                        break;
		if(((s>>8)&0377) == EFLAG) {	/* if illegal flag, call 'usage' */
			cp1 = t[DCOM];
			for(cp2 = cp1;*cp1 != '\0';) {
				if(*cp1++ == '/')
					cp2 = cp1;
			}
			*cp1++ = '?';
			*cp1++ = '\0';
			if((e = fork()) == 0) {	/* child */
				execl(globnam,"usage",cp2,0);
				prs("cannot give usage\n");
				exit(ENOEXEC);
			}
			while(wait() != e);
		}
                e = s&0177;		/* print bad status of terminated child */
                if(mesg[e] != 0) {
                        if(p != i) {
                                prn(p);
                                prs(": ");
                        }
                        prs(mesg[e]);
                }
                if(e != 0)
                        err("");
	
		if(topshell)
                	acct();
		if(i == p)
			break;
        }
}

/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
/* set up for accounting */
acct()
{
        struct stime timbuf;
	struct shactbuf tbuf;
        register i;
	int ldummy[2];
	if(acctf < 0)
		return;
        times(&timbuf);
        lsub(tbuf.shtime, timbuf.cputim, timeb.cputim);
        lsub(ldummy,timbuf.systim, timeb.systim);
	ladd(tbuf.shtime, tbuf.shtime, ldummy);
        tbuf.shuid = uid;
	tbuf.shgid = gid;
	copyit(projno,tbuf.shproj);
	copyit(oprno,tbuf.shopr);
	ladd(cputotime, cputotime, tbuf.shtime);
	seek(acctf,0,2);
        write(acctf, &tbuf, 16);
}

/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
setdflt()
{
        register int i;
        register char *s, *t;
	int tvec[2];
	int *timeinfo;		/* localtime buffer */
	struct systemid sid;

	time(logtotime);
	topshell = 0;
	chgflag = 0;
	time(tvec);			/* get time in seconds */
	timeinfo = localtime(tvec);	/* change to local time array */
	dayweek = ctime(tvec);
	dayweek[3] = '\0';
	s = &daybuf;		/* set pointers */
	t = locv(0,timeinfo[3]);		/* get day */
	while((*s++ = *t++) != '\0');
	s = &monbuf;
	timeinfo[4] =+ 1;			/* make month 1-12, not 0-11 */
	t = locv(0,timeinfo[4]);		/* get month */
	while ((*s++ = *t++) != '\0');
	s = &yearbuf;
	t = locv(0,timeinfo[5]);
	while((*s++ = *t++) != '\0');
	day = &daybuf;	/* set pointer */
	month = &monbuf;
	year = &yearbuf;
        if(pwuid(getuid()&0377,pwbuf)) {
                pwbuf[0] = 0;
                return;
        }
	s = &usernm;		/* set pointers */
	t = pwbuf;
	while((*s++ = *t++) != ':');	/* copy user name */
	*--s = '\0';		/* set end of string */
	username = &usernm;	/* set variable pointer */
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
	homedir = &pwbuf;		/* save home directory name */
        t = pwbuf;
        while (*t)
                *s++ = *t++;  /* pick up name */
        for (t = "/bin/"; *s++ = *t++;);  /* add on /bin/ */
	s = mailbx;
	t = pwbuf;
	while(*t)
		*s++ = *t++;	/* pick up home dir */
	for(t = MAILBOX;*s++ = *t++;);	/* add on /mailbox */
	curwkdir = &cwkdir;
	copyit(&pwbuf,curwkdir);
	sysid(&sid);
	t = &syscbuf[0];
	*t++ = sid.si_class[0];
	*t++ = '\0';
	sysclass = &syscbuf[0];
	sysnbuf[0] = sid.si_type[0];
	sysnbuf[1] = sid.si_type[1];
	s = putn(sid.si_num);
	i = 2;
	while(*s)
		sysnbuf[i++] = *s++;
	sysnbuf[i] = '\0';
	sysname = &sysnbuf[0];
}

/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
/* hash all user bin cmds into core table */
newbin()
{       int a[9], f;
        register int c, i;
        register char *s;

        if(pwbuf[0] == 0)
                return;
        a[8] = 0;
        for(i=0; i<32; i++) bin[i] = 0; /* clear        all the bits */
        if ((f = open(dfltfile,0)) >= 0) {
                seek(f,32,0); /* skip . and .. entries */
                while (read(f,a,16) == 16) {
                        if (a[0]) {
                                s = &a[1];  i = 0;
                                while (c = *s++) i =+ c;
                                bin[(i>>3)&037] =| (1<<(i&07));
                        }
                }
        }
}


/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
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

/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
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

/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
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

/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
/* more of putn */
putn2(an)
{

        if(an > 9)
                putn2(an/10);
        *putp++ = an%10 + '0';
}

/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
/* actual set command */
doset(av)
char *av[];
{
        register char **v;
        register r;
        char *p;
	int setcnt,  setfil;
	char setc;

        v = av;
        if(v[1] == 0) {
yuck:
                err("Bad set command");
                return;
        }
        r = v[1][0];
        if(r < 'A' || r > 'Z' && r < 'a' || r > 'z')	/* not a variable */
                goto yuck;
        if(r > 'Z')			/* reduce to index */
                r =+ 'Z'+1 - 'a';
        r =- 'A';				/* relative to 'A' */
        if(v[2] == 0)
                return;
	if(v[3] == 0) {		/* if third arg is missing */
		if((v[2][0] == '=') || (v[2][0] == ':')) {	/* and operator is colon or equal */
			xfree(vbls[r]);	/* free previous value */
			vbls[r] = 0;	/* reset pointer */
			return;
		}
		else {
			goto yuck;
		}
	}
        switch(v[2][0]) {
	/* get value from file */
	case ':':
		if((setfil = open(&v[3][0],0)) < 0) {	/* open named file */
			err("can't open set file");
			return(0);
		}
		setcnt = 0;
		while((read(setfil,&setc,1) == 1) && (setc != '\n'))	/* get first line */
			v[3][setcnt++] = setc;	/* save each char but last */
		v[3][setcnt++] = '\0';	/* finish v3 string */
		close(setfil);	/* close and continue as usual */
	/* get value from args */
        case '=':
                p = alloc(size(v[3])+1);
                copyit(v[3],p);
stash:
/*
 *      kludge to allow setting the prompt inside next files 
 */
                if(r == 'M'-'A' && sinfil) {
                        xfree(rpromp);
                        rpromp = p;
                        return;
                }
                xfree(vbls[r]);
                vbls[r] = p;
                return;
	/* add args to variable */
        case '+':
                p = putn(getn(vbls[r])+getn(v[3]));
                goto stash;
	/* subtract args from variable */
        case '-':
                p = putn(getn(vbls[r])-getn(v[3]));
                goto stash;
        default:
                goto yuck;
        }
}

/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
/* fast string to string copy */
copyit(ap,aq)
char *ap, *aq;
{
        register char *p, *q;

        p = ap;
        q = aq;

        while(*q++ = *p++);
}

/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
/* special free routine */
xfree(c)
char *c;
{
        extern char end[];

        if(c >= end)
                free(c);
}

/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
/* interpret the "next" command */
next(f)
{
        int unnext();

        sinfil = dup(0);        /* save real standard input */
        close(0);
        dup(f);			/* set new std input */
        close(f);
        rpromp = promp;         /* save prompt string */
        promp = 0;		/* suppress prompt */
        if(setintr)
                signal(INTR,unnext);
}

/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
unnext()
{

        seek(0,0,2);
        setexit(INTR,1);			/* restore things */
}
/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
/*
 *	change tty mode for command edit
 *	no echo and raw
 */
chgmode()
{
	oldmode = ttys[2];
	ttys[2] =| 040;
	ttys[2] =& ~010;
	stty(0,ttys);
}
/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
/*
 *	change tty to normal whole line mode
 */
normode()
{
	ttys[2] = oldmode;
	stty(0,ttys);
}
/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
batch(){
	register int ibat;
	int bpid, bcpid;
	if(topshell) {
		/* ignore all signals*/
		for(ibat=1;ibat<13;ibat++)
			signal(ibat,1);
		for(ibat=0;ibat<7;ibat++) {
			if(argv[0][ibat] == 0)
				break;
			argv[0][ibat] = "-Batch("[ibat];
		}
		if(stat(mailbx,&statbuf) >= 0 && statbuf.size1) {
			prs("You have mail.\n");
		}
		if((bpid = fork()) == 0) {
			execl("/bin/quota","logout quota",&pwbuf,0);
			prs("unable to check disk quota\n");
		}
		else
			while(wait() != bpid);	/* wait for quota to finish */
		if(chgflag)
			prtchg();
		prs("\nThis terminal is AVAILABLE for use.\n");
		sleep(5);
	}
	exit();
}
/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
prtchg() {
	int pip[2];
	int pn;
	pipe(pip);
	if((pn = fork()) == 0) {	/* child */
		close(0);
		dup(pip[0]);		/* change std input */
		execl("/sys/prog/cost/","-",0);
		prs("unable to compute costs\n");
	}
	else {
		write(pip[1],&logtotime,4);
		write(pip[1],&cputotime,4);
		write(pip[1],projno,6);
		write(pip[1],oprno,6);
		close(pip[0]);
		close(pip[1]);
		while(wait() != pn);
	}
}
/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
rstchg() 
{
	if(chgflag)
		prtchg();
	time(logtotime);
	cputotime[0] = cputotime[1] = 0;
}
/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
newproj() {
	int pjpid, pjstat;
	int pjpipa[2], pjpipb[2];
	extern int rstchg();
	rstchg();
	pipe(pjpipa);
	pipe(pjpipb);
	if((pjpid = fork()) == 0) {	/* child */
		close(0);
		dup(pjpipa[0]);
		close(1);
		dup(pjpipb[1]);
		close(pjpipa[1]);
		close(pjpipb[0]);
		execl("/sys/prog/newproj","newproj",0);
		prs("cannot execute newproj\n");
		exit(ENOEXEC);
	}
	close(pjpipa[0]);
	close(pjpipb[1]);
	write(pjpipa[1],&projbuf,6);
	write(pjpipa[1],&usernm,9);
	write(pjpipa[1],&ttynm[8],1);
	write(pjpipa[1],&uid,1);
	write(pjpipa[1],&gid,1);
	close(pjpipa[1]);
	while(wait(&pjstat) != pjpid);
	if((pjstat>>8)&0177 == ENOEXEC)
		return;
	read(pjpipb[0],&projbuf,6);
	read(pjpipb[0],&oprbuf,5);
	close(pjpipb[0]);
}
