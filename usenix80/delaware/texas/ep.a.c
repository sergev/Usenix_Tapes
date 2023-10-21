#ifdef ADM
#include "epa.defs"
#endif
#ifdef INFOTON
#include "epi.defs"
#endif
extern int     vflag   1;
extern int     tfile   -1;
extern int     iblock  -1;
extern int     oblock  -1;
/* extern int     *errlab errfunc; */
extern char    TMPERR[] "TMP";
extern char *disable "command disabled from ep";

main(argc, argv)
char **argv;
{
	register char *p1, *p2;
	extern int onintr();

	onquit = signal(SIGQUIT, 1);
	onhup = signal(SIGHUP, 1);
	argv++;
	if (argc > 1 && **argv=='-') {
		vflag = 0;
		/* allow debugging quits? */
		if ((*argv)[1]=='q') {
			signal(SIGQUIT, 0);
			vflag++;
		}
		argv++;
		argc--;
	}
	if (argc>1) {
		p1 = *argv;
		p2 = savedfile;
		while (*p2++ = *p1++);
		globp = "r";
	}
	fendcore = sbrk(0);
	init();
	if ((signal(SIGINTR, 1) & 01) == 0)
		signal(SIGINTR, onintr);
	globf = NO;
	ep();
	unlink(tfname);
}

commands()
{
	int getfile(), gettty();
	register *a1, c, *j;

	if (pflag) {
		pflag = 0;
		addr1 = addr2 = dot;
		goto print;
	}
	addr1 = 0;
	addr2 = 0;

	do {
		addr1 = addr2;
		if ((a1 = address())==0) {
			c = egetchar();
			break;
		}
		addr2 = a1;
		if ((c=egetchar()) == ';') {
			c = ',';
			dot = a1;
		}
	} while (c==',');

	if (addr1==0)
		addr1 = addr2;

	switch(c) {

		case 'a':
			if ( user ) {
				tell(disable);
				tdisable = YES;
				break;
				}
			setdot();
			newline();
			append(gettty, addr2);
			break;

		case 'c':
			if ( user ) {
				tell(disable);
				tdisable = YES;
				break;
				}
			delete();
			append(gettty, addr1-1);
			break;

		case 'd':
			delete();
			if ( user ) {
				display(dot-zero,TOEND);
				col = 0; row = SSIZE/2;
				infosho(LINE);
				}
			break;

		case 'e':
			secondchance();
			setnoaddr();
			if ((peekc = egetchar()) != ' ')
				error;
			savedfile[0] = 0;
			init();
			addr2 = zero;
			user = BOY;
			goto caseread;

		case 'f':
			setnoaddr();
			if ((c = egetchar()) != '\n') {
				peekc = c;
				savedfile[0] = 0;
				filename();
				poscursor(SGENFO,LOCMSG);
				mopup(MAXCOL-1,SGENFO,SGENFO);
				infosho(ALL);
			}
			else    { 
				tell(disable);
				tdisable = YES;
				}
			break;

		case 'g':
			global(1);
			break;

		case 'i':
			if ( user ) {
				tell(disable);
				tdisable = YES;
				break;
				}
			setdot();
			nonzero();
			newline();
			append(gettty, addr2-1);
			break;

		case 'k':
			if ((c = egetchar()) < 'a' || c > 'z')
				error;
			newline();
			setdot();
			nonzero();
			names[c-'a'] = *addr2 | 01;
			break;

		case 'm':
			j = addr2 - addr1;
			move(0);
			if ( user ) {
				display((dot-zero)-j,TOEND);
				col = 0; row = SSIZE/2;
				infosho(LINE);
				}
			break;

		case '\n':
			if (addr2==0)
				addr2 = dot+1;
			addr1 = addr2;
			if ( searching ) {
				dot = addr2;
				return;
				}
			else goto print;
		case 'l':
			listf++;
		case 'p':
			if ( user ) {
				tell(disable);
				tdisable = YES;
				break;
				}
			newline();
		print:
			setdot();
			display(addr1-zero,TOEND);
			listf = 0;
			break;

		case 'r':
		caseread:
			filename();
			if ((io = open(file, 0)) < 0) {
				lastc = '\n';
				close(io);
				io = -1;
				ZAPTUBE;
				globp = 0;
				initscreen(1);
				break;
			}
			setall();
			ninbuf = 0;
			if( !user ) ZAPTUBE;
			tell("reading ... ");
			j = addr2 + 1;     
			append(getfile, addr2);
			exfile();
			if ( user ) {
				display(j-zero,TOEND);
				col = 0;
				row = SSIZE/2;
				infosho(LINE);
				}
			else    {
				clearmessage();
				initscreen(1);  /* new file: reinitialize */
				}
			break;

		case 's':
			rein = LOOSE;
			user = BOY;
			setdot();
			nonzero();
			substitute(globp);
			break;

		case 't':
			j = addr2 - addr1;
			move(1);
			if ( user ) {
				display((dot-zero)-j,TOEND);
				col = 0; row = SSIZE/2;
				infosho(LINE);
				}
			break;

		case 'v':
			global(0);
			break;

		case 'w':
			filename();
			writeout(file);
			break;

		case 'x':
			rein = TIGHT;
			user = BOY;
			setdot();
			nonzero();
			substitute(globp);
			break;

		case '!':
			ZAPTUBE;
			cleanup();
			unix();
			settty();
			ZAPTUBE;
			initscreen(port[SSIZE/2].line);
			poscursor(col,row);
			break;

		case EOF:
			return;
		default:
			error;  /* indirectly calls errfunc, then resets */
	} /* end of switch */
}

address()
{
	register *a1, minus, c;
	int n, relerr;

	minus = 0;
	a1 = 0;
	for (;;) {
		c = egetchar();
		if ('0'<=c && c<='9') {
			n = 0;
			do {
				n =* 10;
				n =+ c - '0';
			} while ((c = egetchar())>='0' && c<='9');
			peekc = c;
			if (a1==0)
				a1 = zero;
			if (minus<0)
				n = -n;
			a1 =+ n;
			minus = 0;
			continue;
		}
		relerr = 0;
		if (a1 || minus)
			relerr++;
		switch(c) {
		case ' ':
		case '\t':
			continue;
	
		case '+':
			minus++;
			if (a1==0)
				a1 = dot;
			continue;

		case '-':
		case '^':
			minus--;
			if (a1==0)
				a1 = dot;
			continue;
	
		case '?':
		case '/':
			compile(c);
			if( dot <= dol ) {
				a1 = dot;
				if ( !circfl && matchalgol(loc1+1,expbuf) )
					break;
				}
			else if( c=='/' ) a1 = dot = dol;
			else a1 = dot = zero;
			for (;;) {
				if (c=='/') {
					a1++;
					if (a1 > dol)
						a1 = zero;
				} else {
					a1--;
					if (a1 < zero)
						a1 = dol;
				}
				if (execute(0, a1))
					break;
				if (a1==dot)
					error;
			}
			break;
	
		case '$':
			a1 = dol;
			break;
	
		case '.':
			a1 = dot;
			break;

		case '\'':
			if ((c = egetchar()) < 'a' || c > 'z')
				error;
			for (a1=zero; a1<=dol; a1++)
				if (names[c-'a'] == (*a1|01))
					break;
			break;
	
		default:
			peekc = c;
			if (a1==0)
				return(0);
			a1 =+ minus;
			return(a1);
		}
		if (relerr)
			error;
	}
}

setdot()
{
	if (addr2 == 0)
		addr1 = addr2 = dot;
	if (addr1 > addr2)
		error;
}

setall()
{
	if (addr2==0) {
		addr1 = zero+1;
		addr2 = dol;
		if (dol==zero)
			addr1 = zero;
	}
	setdot();
}

setnoaddr()
{
	if (addr2)
		error;
}

nonzero()
{
	if (addr1<=zero || addr2>dol)
		error;
}

newline()
{
	if ( egetchar() == '\n') return;
	else error;
}

filename()
{
	register char *p1, *p2;
	register c;

	count[1] = 0;
	c = egetchar();
	if (c=='\n' || c==EOF) {
		p1 = savedfile;
		if (*p1==0)
			error;
		p2 = file;
		while (*p2++ = *p1++);
		return;
	}
	if (c!=' ')
		error;
	while ((c = egetchar()) == ' ');
	if (c=='\n')
		error;
	p1 = file;
	do {
		*p1++ = c;
	} while ((c = egetchar()) != '\n');
	*p1++ = 0;
	if (savedfile[0]==0) {
		p1 = savedfile;
		p2 = file;
		while (*p1++ = *p2++);
	}
}

exfile()
{
	extern int col, row;
	close(io);
	io = -1;
}

onintr()
{
	signal(SIGINTR, onintr);
	putchar('\n');
	lastc = '\n';
	error;
}

errfunc()
{
	register c;

	listf = 0;
	count[0] = 0;
	seek(0, 0, 2);
	pflag = 0;
	if (globp)
		lastc = '\n';
	globp = 0;
	globf = NO;
	peekc = lastc;
	while ((c = egetchar()) != '\n' && c != EOF);
	if (io > 0) {
		close(io);
		io = -1;
	}
	reset();
}

egetchar()
{
	if (lastc=peekc) {
		peekc = 0;
		return(lastc);
	}
	if (globp) {
		if ((lastc = *globp++) != 0)
			return(lastc);
		globp = 0;
		return(EOF);
	}
	return(lastc = EOF);
}

gettty()
{
	register c;
	register char *gf;
	register char *p;
	int j;

	p = linebuf;
	gf = globp;

	while ((c = egetchar()) != '\n') {
		if (c==EOF) {
			if (gf)
				peekc = c;
			return(c);
		}
		if ((c =& 0177) == 0)
			continue;
		*p++ = c;
		if (p > &linebuf[LBSIZE-1])
			error;
	}
	*p++ = 0;
	if (linebuf[0]=='.' && linebuf[1]==0)
		return(EOF);
	return(0);
}

getfile()
{
	register c;
	register char *lp, *fp;

	lp = linebuf;
	fp = nextip;
	do {
		if (--ninbuf < 0) {
			if ((ninbuf = read(io, genbuf, LBSIZE)-1) < 0)
				return(EOF);
			fp = genbuf;
		}

		if ((c = *fp++ & 0177) == 0) *lp++ = 0200;
		else if (c == TAB) lp = detab(lp,linebuf);
		else *lp++ = c;

		if (lp > &linebuf[LBSIZE-1]) {
			savedfile[0] = 0;
			error;
		}
	} while (c != '\n');
	*--lp = 0;
	for(; lp >= linebuf; lp--)
		if (++count[1] == 0) ++count[0];
	nextip = fp;
	return(0);
}

putfile()
{
	int *a1;
	register char *fp, *lp;
	register nib;

	nib = 512;
	fp = genbuf;
	a1 = addr1;
	do {
		lp = getline(*a1++);
		entab(lp,0);
		for (;;) {
			if (--nib < 0) {
				write(io, genbuf, fp-genbuf);
				nib = 511;
				fp = genbuf;
			}
			if (++count[1] == 0)
				++count[0];
			if (*lp == 0) {
				*fp++ = '\n';
				break;
			}
			else *fp++ = *lp++ & 0177;
		}
	} while (a1 <= addr2);
	write(io, genbuf, fp-genbuf);
}

append(f, a)
int (*f)();
{
	register *a1, *a2, *rdot;
	int nline, tl;
	struct { int integer; };

	nline = 0;
	dot = a;
	while ((*f)() == 0) {
		if (mockdol >= endcore) {
			if (sbrk(1024) == -1)
				error;
			endcore.integer =+ 1024;
		}
		tl = putline();
		nline++;
		++zapstart;
		++dol;
		a1 = ++mockdol;
		a2 = a1+1;
		rdot = ++dot;
		while (a1 > rdot)
			*--a2 = *--a1;
		*rdot = tl;
	}
	return(nline);
}

unix()
{
	register savint, pid, rpid;
	int retcode;

	setnoaddr();
	if ((pid = fork()) == 0) {
		signal(SIGHUP, onhup);
		signal(SIGQUIT, onquit);
		execl("/bin/sh", "sh", "-c", globp, 0);
		exit();
	}
	savint = signal(SIGINTR, 1);
	while ((rpid = wait(&retcode)) != pid && rpid != -1);
	signal(SIGINTR, savint);
	puts1("! CR to reenter ep ... ");
	getchar();
}

delete()
{
	register *a1, *a2, *a3;
	int diff;

	setdot();
	newline();
	nonzero();
	a1 = addr1;
	a2 = addr2+1;
	a3 = mockdol;
	diff = a2 - a1;
	zapstart =- diff;
	dol =- diff;
	mockdol =- diff;
	do
		*a1++ = *a2++;
	while (a2 <= a3);
	a1 = addr1;
	if (a1 > dol)
		a1 = dol;
	dot = a1;
}

getline(tl)
{
	register char *bp, *lp;
	register nl;
	char *keep;

	lp = linebuf;
	bp = getblock(tl, READ);
	keep = bp;
	nl = nleft;
	tl =& ~0377;            
	while ( *lp = *bp++ ) {
		if ( *lp == TAB ) lp = detab(lp,linebuf);
		else lp++;
		if (--nl == 0) {
			bp = getblock(tl=+0400, READ);
			nl = nleft;
			}
		}
	return(linebuf);
}

putline()
{
	register char *bp, *lp;
	register nl;
	int tl;

	entab(linebuf,0);
	lp = linebuf;
	tl = tline;
	bp = getblock(tl, WRITE);
	nl = nleft;
	tl =& ~0377;
	while (*bp = *lp++) {
		if (*bp++ == '\n') {
			*--bp = 0;
			linebp = lp;
			break;
		}
		if (--nl == 0) {
			bp = getblock(tl=+0400, WRITE);
			nl = nleft;
		}
	}
	nl = tline;
	tline =+ (((lp-linebuf)+03)>>1)&077776;
	return(nl);
}

getblock(atl, iof)
{
	extern read(), write();
	register bno, off;
	
	bno = (atl>>8)&0377;
	off = (atl<<1)&0774;
	if (bno >= 255) {
		puts1(TMPERR);
		error;
	}
	nleft = 512 - off;
	if (bno==iblock) {
		ichanged =| iof;
		return(ibuff+off);
	}
	if (bno==oblock)
		return(obuff+off);
	if (iof==READ) {
		if (ichanged)
			blkio(iblock, ibuff, write);
		ichanged = 0;
		iblock = bno;
		blkio(bno, ibuff, read);
		return(ibuff+off);
	}
	if (oblock>=0)
		blkio(oblock, obuff, write);
	oblock = bno;
	return(obuff+off);
}

blkio(b, bufit, iofcn)
int (*iofcn)();
{
	seek(tfile, b, 3);
	if ((*iofcn)(tfile, bufit, 512) != 512) {
		puts1(TMPERR);
		error;
	}
}

init()
{
	register char *p;
	register pid;

	close(tfile);
	tline = 0;
	iblock = -1;
	oblock = -1;
	tfname = "/tmp/exxxxx";
	ichanged = 0;
	pid = getpid();
	for (p = &tfname[11]; p > &tfname[6];) {
		*--p = (pid&07) + '0';
		pid =>> 3;
	}
	close(creat(tfname, 0600));
	tfile = open(tfname, 2);
	brk(fendcore);
	dot = zero = dol = fendcore;
	/* reserve a slot between dol and zapstart for picked addresses */
	mockdol = dol;
	zapstart = dol + 1;
	endcore = fendcore - 2;
}

global(k)
{
	register char *gp;
	register c;
	register int *a1;
	char globuf[GBSIZE];

	if (globf)
		error;
	globf = YES;
	setall();
	nonzero();
	if ((c=egetchar())=='\n')
		error;
	compile(c);
	gp = globuf;
	while ((c = egetchar()) != '\n') {
		if (c==EOF)
			error;
		if (c=='\\') {
			c = egetchar();
			if (c!='\n')
				*gp++ = '\\';
		}
		*gp++ = c;
		if (gp >= &globuf[GBSIZE-2])
			error;
	}
	*gp++ = '\n';
	*gp++ = 0;
	for (a1=zero; a1<=dol; a1++) {
		*a1 =& ~01;
		if (a1>=addr1 && a1<=addr2 && execute(0, a1)==k)
			*a1 =| 01;
	}
	for (a1=zero; a1<=dol; a1++) {
		if (*a1 & 01) {
			*a1 =& ~01;
			dot = a1;
			globp = globuf;
			commands();
			if( tdisable ) break;
			a1 = zero;
		}
	}
	globf = NO;
}

substitute(inglob)
{
	register gsubf, *a1, nl;
	int getsub();

	gsubf = compsub();
	for (a1 = addr1; a1 <= addr2; a1++) {
		if (execute(0, a1)==0)
			continue;
		inglob =| 01;
		if ( subcontrol(a1) == ED ) return;
		if (gsubf) {
			while (*loc2) {
				if (execute(1)==0)
					break;
				if ( subcontrol(a1) == ED ) return;
			}
		}
		*a1 = putline();
		nl = append(getsub, a1);
		a1 =+ nl;
		addr2 =+ nl;
	}
	if (inglob==0)
		error;
}

compsub()
{
	register seof, c;
	register char *p;
	int gsubf;

	if ((seof = egetchar()) == '\n')
		error;
	compile(seof);
	p = rhsbuf;
	for (;;) {
		c = egetchar();
		if (c=='\\')
			c = egetchar() | 0200;
		if (c=='\n')
			error;
		if (c==seof)
			break;
		*p++ = c;
		if (p >= &rhsbuf[LBSIZE/2])
			error;
	}
	*p++ = 0;
	if ((peekc = egetchar()) == 'g') {
		peekc = 0;
		newline();
		return(1);
	}
	newline();
	return(0);
}

getsub()
{
	register char *p1, *p2;

	p1 = linebuf;
	if ((p2 = linebp) == 0)
		return(EOF);
	while (*p1++ = *p2++);
	linebp = 0;
	return(0);
}

dosub()
{
	register char *lp, *sp, *rp;
	int c;

	lp = linebuf;
	sp = genbuf;
	rp = rhsbuf;
	while (lp < loc1)
		*sp++ = *lp++;
	while (c = *rp++) {
		if (c=='&') {
			sp = place(sp, loc1, loc2);
			continue;
		} else if (c<0 && (c =& 0177) >='1' && c < NBRA+'1') {
			sp = place(sp, braslist[c-'1'], braelist[c-'1']);
			continue;
		}
		*sp++ = c&0177;
		if (sp >= &genbuf[LBSIZE])
			error;
	}
	lp = loc2;
	loc2 = sp + linebuf - genbuf;
	while (*sp++ = *lp++)
		if (sp >= &genbuf[LBSIZE])
			error;
	lp = linebuf;
	sp = genbuf;
	while (*lp++ = *sp++);
}

place(asp, al1, al2)
{
	register char *sp, *l1, *l2;

	sp = asp;
	l1 = al1;
	l2 = al2;
	while (l1 < l2) {
		*sp++ = *l1++;
		if (sp >= &genbuf[LBSIZE])
			error;
	}
	return(sp);
}

move(cflag)
{
	register int *adt, *ad1, *ad2;
	int getcopy();

	setdot();
	nonzero();
	if ((adt = address())==0)
		error;
	newline();
	ad1 = addr1;
	ad2 = addr2;
	if (cflag) {
		ad1 = dol;
		append(getcopy, ad1++);
		ad2 = dol;
	}
	ad2++;
	if (adt<ad1) {
		dot = adt + (ad2-ad1);
		if ((++adt)==ad1)
			return;
		reverse(adt, ad1);
		reverse(ad1, ad2);
		reverse(adt, ad2);
	} else if (adt >= ad2) {
		dot = adt++;
		reverse(ad1, ad2);
		reverse(ad2, adt);
		reverse(ad1, adt);
	} else
		error;
}

reverse(aa1, aa2)
{
	register int *a1, *a2, t;

	a1 = aa1;
	a2 = aa2;
	for (;;) {
		t = *--a2;
		if (a2 <= a1)
			return;
		*a2 = *a1;
		*a1++ = t;
	}
}

getcopy()
{
	if (addr1 > addr2)
		return(EOF);
	getline(*addr1++);
	return(0);
}

compile(aeof)
{
	register eof, c;
	register char *ep;
	char *lastep;
	char bracket[NBRA], *bracketp;
	int nbra;
	int cclcnt;

	ep = expbuf;
	eof = aeof;
	bracketp = bracket;
	nbra = 0;
	if ((c = egetchar()) == eof) {
		if (*ep==0)
			error;
		return;
	}
	circfl = 0;
	if (c=='^') {
		c = egetchar();
		circfl++;
	}
	if (c=='*')
		goto cerror;
	peekc = c;
	for (;;) {
		if (ep >= &expbuf[ESIZE])
			goto cerror;
		c = egetchar();
		if (c==eof) {
			*ep++ = CEOF;
			return;
		}
		if (c!='*')
			lastep = ep;
		switch (c) {

		case '\\':
			if ((c = egetchar())=='(') {
				if (nbra >= NBRA)
					goto cerror;
				*bracketp++ = nbra;
				*ep++ = CBRA;
				*ep++ = nbra++;
				continue;
			}
			if (c == ')') {
				if (bracketp <= bracket)
					goto cerror;
				*ep++ = CKET;
				*ep++ = *--bracketp;
				continue;
			}
			*ep++ = CCHR;
			if (c=='\n')
				goto cerror;
			*ep++ = c;
			continue;

		case '.':
			*ep++ = CDOT;
			continue;

		case '\n':
			goto cerror;

		case '*':
			if (*lastep==CBRA || *lastep==CKET)
				error;
			*lastep =| STAR;
			continue;

		case '$':
			if ((peekc=egetchar()) != eof)
				goto defchar;
			*ep++ = CDOL;
			continue;

		case '[':
			*ep++ = CCL;
			*ep++ = 0;
			cclcnt = 1;
			if ((c=egetchar()) == '^') {
				c = egetchar();
				ep[-2] = NCCL;
			}
			do {
				if (c=='\n')
					goto cerror;
				*ep++ = c;
				cclcnt++;
				if (ep >= &expbuf[ESIZE])
					goto cerror;
			} while ((c = egetchar()) != ']');
			lastep[1] = cclcnt;
			continue;

		defchar:
		default:
			*ep++ = CCHR;
			*ep++ = c;
		}
	}
   cerror:
	expbuf[0] = 0;
	error;
}

execute(gf, addr)
int *addr;
{
	register char *p1, *p2, c;

	if (gf) {
		if (circfl)
			return(0);
		p1 = linebuf;
		p2 = genbuf;
		while (*p1++ = *p2++);
		locs = p1 = loc2;
	} else {
		if (addr==zero)
			return(0);
		p1 = getline(*addr);
		locs = 0;
	}
	p2 = expbuf;
	if (circfl) {
		loc1 = p1;
		return(advance(p1, p2));
	}
	return( matchalgol(p1,p2) );
}

/* matchalgol -
*/
matchalgol(s1,s2) char *s1, *s2; {
	register char *p1, *p2, c;

	p1 = s1; 
	p2 = s2;
	/* fast check for first character */
	if (*p2==CCHR) {
		c = p2[1];
		do {
			if (*p1!=c)
				continue;
			if (advance(p1, p2)) {
				loc1 = p1;
				return(1);
			}
		} while (*p1++);
		return(0);
	}
	/* regular algorithm */
	do {
		if (advance(p1, p2)) {
			loc1 = p1;
			return(1);
		}
	} while (*p1++);
	return(0);
	}

advance(alp, aep)
{
	register char *lp, *ep, *curlp;
	char *nextep;

	lp = alp;
	ep = aep;
	for (;;) switch (*ep++) {

	case CCHR:
		if (*ep++ == *lp++)
			continue;
		return(0);

	case CDOT:
		if (*lp++)
			continue;
		return(0);

	case CDOL:
		if (*lp==0)
			continue;
		return(0);

	case CEOF:
		loc2 = lp;
		return(1);

	case CCL:
		if (cclass(ep, *lp++, 1)) {
			ep =+ *ep;
			continue;
		}
		return(0);

	case NCCL:
		if (cclass(ep, *lp++, 0)) {
			ep =+ *ep;
			continue;
		}
		return(0);

	case CBRA:
		braslist[*ep++] = lp;
		continue;

	case CKET:
		braelist[*ep++] = lp;
		continue;

	case CDOT|STAR:
		curlp = lp;
		while (*lp++);
		goto star;

	case CCHR|STAR:
		curlp = lp;
		while (*lp++ == *ep);
		ep++;
		goto star;

	case CCL|STAR:
	case NCCL|STAR:
		curlp = lp;
		while (cclass(ep, *lp++, ep[-1]==(CCL|STAR)));
		ep =+ *ep;
		goto star;

	star:
		do {
			lp--;
			if (lp==locs)
				break;
			if (advance(lp, ep))
				return(1);
		} while (lp > curlp);
		return(0);

	default:
		error;
	}
}

cclass(aset, ac, af)
{
	register char *set, c;
	register n;

	set = aset;
	if ((c = ac) == 0)
		return(0);
	n = *set++;
	while (--n)
		if (*set++ == c)
			return(af);
	return(!af);
}

puts(as)
{
	register char *sp;

	sp = as;
	while ( *sp ) putchar(*sp++);
	putchar('\n'); 
	return(sp-as);  /* return number chars written */
}

puts1(as)
{
/*      Same as puts but no '\n' at EOL     */
	register int ras;

	ras = length(as);
	write(1,as,ras);
	return(ras);
}

char    line[70];
char    *linp   line;

putchar(ac)
{
	register char *lp;
	register c;


	lp = linp;
	c = ac;
	++colm;
	if (listf) {
		if (colm >= 72) {
			colm = 0;
			*lp++ = '\\';
			*lp++ = '\n';
		}
		if (c=='\t') {
			c = '>';
			goto esc;
		}
		if (c=='\b') {
			c = '<';
		esc:
			*lp++ = '-';
			*lp++ = '\b';
			*lp++ = c;
			goto outgo;
		}
		if (c<' ' && c!= '\n') {
			*lp++ = '\\';
			*lp++ = (c>>3)+'0';
			*lp++ = (c&07)+'0';
			colm =+ 2;
			goto outgo;
		}
	}

	/* Don't print control characters (unless l flag is on) */
	if ( (c > 0 && c <= 037) || c == 0177 )             
		if ( c != '\n' ) return(OK);    /* let line feeds thru */

outgo:
	if(c == '\n' || lp >= &line[64]) {
		linp = line;
		write(1, line, lp-line);
		return(OK);
	}
	linp = lp;
	return(OK);
}
