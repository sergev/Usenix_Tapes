#include "../local.h"
/*
 * (c) copyright 1979 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * Explicit permission is hereby granted to universities to use or duplicate
 * this program for educational or research purposes.  All other use or dup-
 * lication  by universities,  and all use or duplication by other organiza-
 * tions is expressly prohibited unless written permission has been obtained
 * from the Vrije Universiteit. Requests for such permissions may be sent to
 *      Dr. Andrew S. Tanenbaum
 *      Wiskundig Seminarium
 *      Vrije Universiteit
 *      Postbox 7161
 *      1007 MC Amsterdam
 *      The Netherlands
 */

/*
 * put all the pieces of the pascal part of the EM1 project together
 *	author: Johan Stevenson, Vrije Universiteit, Amsterdam
 */

#define	void		int

#define NOEXPANDYET	1

#define	DIRSIZ		14

int	lastpass	4;
char	*passpath[5];
char	*passflag[5];

int	argerr;
int	toterr;
int	parent;

char	*pemflag;
int	pemflen;

char	*Dflag;
char	*Eflag;
#ifdef INT_ONLY
char	*Iflag		"-I";
#endif
#ifndef INT_ONLY
char	*Iflag;
#endif
char	*Lflag;
char	*Oflag;
char	*Pflag;
char	*Sflag;
char	*Cflag;
char	*eflag;
char	*fflag;
char	*oflag;
char	*sflag		"-sm";
char	*tflag;
char	*wflag;

#define	CALLSIZE	60
#define	LOADSIZE	50
char	*callvector[CALLSIZE];
char	*loadvector[LOADSIZE];
char	**loadv		loadvector;
char	**av;
int	ac;
int	fileargs;	/* number of recognized, processed args */
int	flagargs;
char	*progname;
char	*libfile;

#define	CHARSIZE	2500
#define CHARMARG	50
char	charbuf[CHARSIZE];
char	*charp		charbuf;

extern		fout[259];
extern		errno;
extern	char	*sys_errlist[];

char	*pdptail[] {
	LIBPR_PATH,
	LIBEM_PATH,
	"-l",
	BSS_PATH,
	0
};
char	*em1tail[] {
	EM1PR_PATH,
	0
};

char	*tmp_dir	TMP_DIR;
char	*unique		"pcXXXXXX";

void	finish();

main(argc,argv) char **argv; {
	register char *p;

	if ((signal(2,1) & 1) == 0)
		signal(2,&finish);
	ac = argc;
	av = argv;
	progname = *av++;
	if (--ac == 0) {
		info();
		exit(-1);
	}
	init();
	while (--ac >= 0) {
		p = *av++;
		if (*p == '-') {
			flagargs++;
			p = flag(p);
		} else {
			fileargs++;
			p = process(p);
		}
		if (p) {
			*loadv++ = p;
			if (loadv >= &loadvector[LOADSIZE])
				fatal("too many load arguments");
		}
	}
	if (Pflag || Dflag || Sflag || Lflag)
		finish();
	if (fileargs == 0)
		fatal("pascal programs must have suffix .p");
	if (lastpass >= 4 && toterr == 0)
		load();
	if (lastpass >= 5 && toterr == 0)
		go();
	finish();
}

void	pem();
void	opt();
void	pdp();
void	as();
void	encode();
void	decode();
#ifndef NOEXPANDYET
void	expand();
#endif
void	lib();

char *process(arg) char *arg; {
	register char *p,*q;
	register last;
	int fd,c,c1,suf;
	void (*func)();

	p = arg;
	argerr = 0;
	for (;;) {
		switch(c = getsuf(p)) {
		case 'p':
			c1 = '\0';
			if ((fd = open(p,0)) > 0) {
				read(fd,&c1,1);
				close(fd);
			}
			if (c1 == '#') {
#ifdef NOEXPANDYET
				fatal("Preprocessor not yet available");
			}
#endif
#ifndef NOEXPANDYET
				func = expand;
				last = Pflag;
				suf = 'i';
				break;
			}
		case 'i':
			if (c == 'i' && p == arg) {
				fileargs--;
				return(p);
			}
			if (Pflag)
				return(0);
#endif
			func = pem;
			last = (lastpass <= 1);
			suf = 'k';
			break;
		case 'e':
			if (Dflag && p != arg)
				return(0);
			func = encode;
			last = (lastpass <= 1);
			suf = 'k';
			break;
		case 's':
			if (lastpass<=2 || Sflag)
				return(0);
			func = as;
			last = (lastpass <= 3);
			suf = 'o';
			break;
		case 'l':
			if (lastpass<=2 || Lflag)
				return(0);
			if (Dflag) {
				func = decode;
				last = 1;
				suf = 'e';
				break;
			}
			return(p);
		case 'k':
			if (lastpass <= 1)
				return(0);
			if (Oflag || Cflag || Lflag) {
				func = opt;
				last = (lastpass <= 2);
				suf = 'm';
				break;
			}
		case 'm':
			if (lastpass <= 2)
				return(0);
			if (Cflag) {
				func = pdp;
				last = Sflag;
				suf = 's';
				break;
			}
			if (Dflag) {
				func = decode;
				last = 1;
				suf = 'e';
				break;
			}
			if (Lflag) {
				if (p == arg)
					return(0);
				func = lib;
				last = 1;
				suf = 'l';
				break;
			}
		case 'o':
			return(p);
		default:
			fileargs--;
			return(p);
		}
		q = newfile(last,arg,suf);
		(*func)(p,q);
		toterr =+ argerr;
		if (argerr)
			return;
		donewith(p);
		p = q;
	}
}

char *flag(f) char *f; {
	register char *p;
	register c;

	p = f+1;
	switch (c = *p++) {
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
		if (*p == '=') {
			passpath[c-'1'] = p+1;
			return(0);
		}
		if (*p) {
			passflag[c-'1'] = p;
			return(0);
		}
		lastpass = c-'0';
		break;
	case 'e':
		eflag = f;
		break;
	case 'E':
		Eflag = f;
		break;
	case 'w':
		wflag = f;
		break;
	case 'r':
		lastpass = 5;
		break;
	case 'o':
		if (ac--)
			oflag = *av++;
		else
			fatal("-o flag may not be last arg");
		break;
	case 'f':
		fflag = f;
		break;
	case 't':
		tflag = f;
		break;
	case '{':
		pemflag = p;
		while (*p)
			p++;
		if (*--p == '}')
			*p++ = '\n';
		pemflen = p-pemflag;
		break;
	case 'C':
		Cflag = f;
		break;
	case 'D':
		Dflag = f;
		break;
	case 'I':
		Iflag = f;
		break;
	case 'L':
		Lflag = f;
		break;
	case 'O':
		Oflag = f;
		break;
#ifndef NOEXPANDYET
	case 'P':
		Pflag = f;
		break;
#endif
	case 'S':
		Sflag = f;
		break;
	case 's':
		if (*p == 's' || *p == 'm' || *p == 'l') {
			sflag = f;
			p++;
			break;
		}
	default:
		return(f);
	}
	if (*p)
		fatal("bad flag %s",f);
	return(0);
}

/* ------------------ calling sequences -------------------- */

pem(p,q) char *p,*q; {
	register char **v,*d; register fd;

	if (Iflag)
		v = em1vector(1,PEMI_PATH);
	else
		v = initvector(1,PEM_PATH);
	d = tempfile('d');
	if ((fd = creat(q,0600)) < 0)
		syserr(q);
	if (pemflag)
		if (write(fd,pemflag,pemflen) != pemflen)
			syserr(q);
	close(fd);
	*v++ = q;
	call(v,p,d);
	if (argerr == 0)
		if (list(p,d) < 0)
			argerr++;
	donewith(d);
}

#ifndef NOEXPANDYET
expand(p,q) char *p,*q; {
	register char **v;

	v = callvector;
	*v++ = EXP_PATH;
	*v++ = progname;
	*v++ = p;
	call(v,0,q);
}
#endif

encode(p,q) char *p,*q; {
	register char **v;

	if (Iflag)
		v = em1vector(1,ENCI_PATH);
	else
		v = initvector(1,ENC_PATH);
	call(v,p,q);
}

decode(p,q) char *p,*q; {
	register char **v;

	if (Iflag)
		v = em1vector(3,DECI_PATH);
	else
		v = initvector(3,DEC_PATH);
	call(v,p,q);
}

as(p,q) char *p,*q; {
	register char **v;

	v = callvector;
	*v++ = AS_PATH;
	*v++ = progname;
	*v++ = "-o";
	*v++ = q;
/*	*v++ = "-";		/* will be unnecessary shortly */
	*v++ = p;
	call(v,0,0);
}

opt(p,q) char *p,*q; {
	register char **v;

	v = initvector(2,OPT_PATH);
	*v++ = sflag;
	if (Lflag) {
		*v++ = Lflag;
		*v++ = libfile = tempfile('x');
	}
	*v++ = p;
	call(v,0,q);
}

pdp(p,q) char *p,*q; {
	register char **v;

	v = initvector(3,PDP_PATH);
	*v++ = "-o";
	*v++ = q;
	call(v,p,0);
}

lib(p,q) char *p,*q; {
	register char **v;

	v = initvector(3,LIB_PATH);
	*v++ = p;
	*v++ = libfile;
	call(v,0,q);
	donewith(libfile);
}

load() {
	register char **v,**lv;

	v = initvector(4,(Cflag ? LD_PATH : ASS_PATH));
	if (Cflag) {
		*v++ = "-X";
		*v++ = (fflag ? FRT0_PATH : RT0_PATH);
	} else
		*v++ = sflag;
	if (oflag) {
		*v++ = "-o";
		*v++ = oflag;
	}
	for (lv=loadvector; lv<loadv; )
		*v++ = *lv++;
	for (lv = (Cflag ? pdptail : em1tail); *lv; )
		*v++ = *lv++;
	call(v,0,0);
}

go() {
	register char **v;

	if (Cflag) {
		v = callvector;
		*v++ = (oflag ? oflag : "a.out");
		*v++ = progname;
	} else {
		v = initvector(5,EM1_PATH);
		*v++ = (oflag ? oflag : "e.out");
	}
	while ((parent = fork()) < 0)
		sleep(1);
	if (parent==0) {
		parent++;
		finish();
	}
	parent = 0;
	*v = 0;
	execv(callvector[0],callvector+1);
	syserr(callvector[0]);
}

/* ------------------- pascal listing ----------------------- */

#define MAXERNO		300
#define MAXERRLIST	10

#define	N1		0
#define N2		6
#define	N3		12
#define	NN		18
#define NC		8

char	errstr[NN+1];
int	errfd;
int	*index;
int	maxerno;

int	erno;
int	lino;
int	chno;
char	name[NC];
int	peekerno;
int	peeklino;
int	peekchno;
char	peekname[NC];

int	errerr;
int	errfat;

int	listline;

struct iobuf {
	int	fildes;
	int	nleft;
	char	*nextp;
	char	buff[512];
}	*ibuf,*ebuf;

char	err_path[]	ERR_PATH;

list(p,q) char *p,*q; {

	if ((errfd = open(q,0)) < 0)
		syserr(q);
	if (gettriple()) {
		if (Eflag == 0)
			printf("%s:\n",p);
	} else {
		if (Eflag==0) {
			close(errfd);
			return(0);
		}
	}
	if (ebuf == 0)
		ebuf = sbrk(sizeof *ebuf);
	if (index == 0) {
		index = sbrk(MAXERNO * sizeof index[0]);
		fillindex();
	}
	if (Eflag || eflag) {
		if (ibuf == 0)
			ibuf = sbrk(sizeof *ibuf);
		if (fopen(p,ibuf) < 0)
			syserr(p);
	}
	errerr = errfat = listline = 0;
	if (Eflag) {
		if (fout[0] <= 2)
			fout[0] = dup(1);
		listfull();
	} else if (eflag)
		listpartial();
	else
		listshort();
	close(errfd);
	flush();
	return(errfat ? -1 : 1);
}

listshort() {

	while (nexterror()) {
		printf("%5d:\t",lino);
		string(erno,name);
	}
}

listfull() {

	if (nexterror())
		do {
			do {
				if (printline() == 0)
					fatal("error line number too high");
			} while (listline < lino);
		} while (errorline());
	while (printline());
}

listpartial() {

	if (nexterror())
		do {
			do {
				if (((listline<lino-2) ?
						skipline() : printline()) == 0)
					fatal("error line number too high");
			} while (listline < lino);
		} while (errorline());
}

printline() {
	register ch;

	if ((ch=getc(ibuf))>=0) {
		printf("%5d\t",++listline);
		do {
			putchar(ch);
			if (ch=='\n')
				return(1);
		} while((ch=getc(ibuf))>=0);
	}
	return(0);
}

skipline() {
	register ch;

	++listline;
	while((ch=getc(ibuf))!='\n' && ch >= 0 )
		;
	return(ch == '\n');
}

errorline() {
	register n,i,c;
	int	errlist[MAXERRLIST];
	char	errname[MAXERRLIST][NC];
	int	goon;

	printf("*** ***\t");
	n = 0;
	c = 0;
	do {
		if (c < chno)
			printf("%*c",chno-c,'^');
		c = chno;
		if (n < MAXERRLIST) {
			strcpy(name,errname[n],NC);
			errlist[n++] = erno;
		}
		goon = nexterror();
	} while (goon && lino==listline);
	putchar('\n');
	for (i=0; i<n; i++)
		string(errlist[i],errname[i]);
	putchar('\n');
	return(goon);
}

int gettriple() {
	register n;

	peekerno = 0;
	if ((n = read(errfd,errstr,NN+1)) == 0)
		return(0);
	if (n != NN+1)
		fatal("bad error format");
	if (errstr[NN] != '\n') {
		strcpy(errstr,peekname,NC);
		strcpy(errstr+NC,errstr,NN-NC+1);
		if (read(errfd,errstr+NN-NC+1,NC) != NC || errstr[NN] != '\n')
			fatal("bad error format");
	} else
		peekname[0] = '\0';
	peekerno = atoi(errstr+N1);
	peeklino = atoi(errstr+N2);
	peekchno = atoi(errstr+N3);
	return(1);
}

int nexterror() {
	do {	/* skip warnings if wflag */
		if ((erno = peekerno) == 0)
			return(0);
		lino = peeklino;
		chno = peekchno;
		strcpy(peekname,name,NC);
		for (;;) {
			if (gettriple() == 0)
				break;
			if (peekname[0] != '\0')
				break;
			if (peeklino != lino || peekchno != chno)
				break;
			if (erno < 0 && peekerno > 0)
				/* promote warnings if they cause fatals */
				erno = -erno;
		}
	} while (erno < 0 && wflag != 0);
	return(1);
}

fillindex() {
	register *ip,n,c;

	if (fopen(err_path,ebuf) < 0)
		syserr(err_path);
	ip = index;
	*ip++ = 0;
	n = 0;
	while ((c = getc(ebuf)) >= 0) {
		n++;
		if (c == '\n') {
			*ip++ = n;
			if (ip > &index[MAXERNO])
				fatal("too many errors on %s",err_path);
		}
	}
	maxerno = ip - index;
}

string(err,nam) char *nam; {
	register off,n,e;

	errerr++;
	if ((e = err) < 0) {
		e = -e;
		printf("Warning: ");
	} else
		errfat++;
	if (e == 0 || e >= maxerno)
		fatal("bad error number %d",e);
	off = index[e-1];
	n = index[e] - off;
	if (n >= 512)
		n = 511;
	if (seek(ebuf->fildes,off,0) < 0)
		;
	if (read(ebuf->fildes,ebuf->buff,n) < 0)
		;
	ebuf->buff[n] = '\0';
	print(ebuf->buff,nam);
}

print(err,nam) char *err,*nam; {
	register char *p,c; register i;

	p = err;
	while (c = *p++) {
		if (c == '%') {
			for (i=0; i<NC; i++)
				if ((c = nam[i]) != ' ')
					putchar(c);
		} else
			putchar(c);
	}
}

strcpy(s1,s2,n) char *s1,*s2; {
	while (--n >= 0)
		*s2++ = *s1++;
}

/* ------------------- miscellaneous routines --------------- */

int getsuf(path) char *path; {
	register n;
	register char *p;

	p = path;
	n = 0;
	while (*p)
		if (*p++ == '/')
			n = 0;
		else
			n++;
	if (n <= DIRSIZ && n > 2 && p[-2] == '.')
		return(*--p);
	return(0);
}

char *newfile(last,old,suf) char *old; {

	if (charp > &charbuf[CHARSIZE-CHARMARG])
		fatal("charbuf overflow");
	if (last || tflag)
		return(setsuf(old,suf));
	else
		return(tempfile(suf));
}

char *setsuf(path,c) char *path; {
	register char *p,*q;

	p = q = path;
	while (*p)
		if (*p++ == '/')
			q = p;
	p = charp;
	while (*charp++ = *q++)
		;
	charp[-2] = c;
	return(p);
}

char *tempfile(suf) {
	register char *p,*q;
	register i;

	p = charp; q = tmp_dir;
	while (*p = *q++)
		p++;
	*p++ = '/';
	q = unique;
	while (*p = *q++)
		p++;
	i = fileargs;
	do
		*p++ = i % 10 + '0';
	while (i =/ 10);
	*p++ = '.'; *p++ = suf; *p++ = '\0';
	q = charp; charp = p;
	return(q);
}

call(v,in,out) char **v,*in,*out; {
	register pid;
	register char *p;
	int	status;

	while ((parent = fork()) < 0)
		sleep(1);
	if (parent == 0) {
		if (in) {
			close(0);
			if (open(in,0) != 0)
				syserr(in);
		}
		if (out) {
			close(1);
			if (creat(out,0600) != 1)
				syserr(out);
		}
		*v = 0;
		execv(callvector[0],callvector+1);
		syserr(callvector[0]);
	}
	while ((pid = wait(&status)) != parent) {
		if (pid == -1)
			fatal("process %d disappeared",parent);
		message("unknown child %d died",pid);
	}
	if (status & 0177 > 3) {
		if ((status & 0200) && tflag==0)
			unlink("core");
		fatal("fatal error %d in %s. Call for help on an expert",
				status&0177,callvector[0]);
	}
	if (status & 0177400)
		argerr++;
}

char **initvector(pass,path) char *path; {
	register char *p,**v;
	register n;

	n = pass-1;
	v = callvector;
	*v++ = ((p = passpath[n]) ? p : path);
	*v++ = progname;
	if (p = passflag[n])
		*v++ = p;
	return(v);
}

char **em1vector(pass,path) char *path; {
	register char *p,**v;
	register n;

	n = pass-1;
	v = callvector;
	*v++ = EM1_PATH;
	*v++ = progname;
	*v++ = ((p = passpath[n]) ? p : path);
	if (p = passflag[n])
		*v++ = p;
	return(v);
}

syserr(s) char *s; {
	fatal("%s: %s",s,sys_errlist[errno]);
}

fatal(s,a1,a2,a3,a4) {
	message(s,a1,a2,a3,a4);
	toterr++;
	finish();
}

message(s,a1,a2,a3,a4) {
	register fd;

	flush();
	fd = fout[0];
	fout[0] = 2;
	printf("%s: %r",progname,&s);
	putchar('\n');
	flush();
	fout[0] = fd;
}

info() {
	register fd,n;
	char buf[512];

	if ((fd = open(INFO_PATH,0)) < 0)
		syserr(INFO_PATH);
	while ((n = read(fd,buf,512)) > 0)
		write(1,buf,n);
	close(fd);
}

finish() {
	register char *p,*q;
	register fd;
	struct {
		int	inode;
		char	name[DIRSIZ];
	} dir;

	signal(2,1);
	if (parent != 0) {
		chdir(tmp_dir);
		fd = open(".",0);
		while (read(fd,&dir,sizeof dir) == sizeof dir) {
			if (dir.inode == 0)
				continue;
			p = unique;
			q = dir.name;
			while (*p++ == *q++)
				if (*p == '\0') {
					unlink(dir.name);
					break;
				}
		}
		close(fd);
	}
	exit(toterr ? -1 : 0);
}


donewith(p) char *p; {

	if (tflag)
		return;
	if (p >= charbuf && p < &charbuf[CHARSIZE])
		unlink(p);
}

init() {
	register char *p;
	register i,fd;

	if ((fd = open(tmp_dir,0)) < 0)
		tmp_dir = ".";
	close(fd);
	p = unique+2;
	parent = i = getpid();
	do
		*p++ = i % 10 + '0';
	while (i =/ 10);
	*p++ = '.'; *p = '\0';
}
