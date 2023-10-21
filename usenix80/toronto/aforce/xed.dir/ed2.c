#
#include	"/doc/source/prog/s1.afdsc/xed.dir/externals.h"
address()
{
	register *a1, minus, c;
	int n, relerr;

	minus = 0;
	a1 = 0;
	for (;;) {
		c = getchar();
		if ('0'<=c && c<='9') {
			n = 0;
			do {
				n =* 10;
				n =+ c - '0';
			} while ((c = getchar())>='0' && c<='9');
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
			a1 = dot;
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
					ERROR;
			}
			break;
	
		case '$':
			a1 = dol;
			break;
	
		case '.':
			a1 = dot;
			break;

		case '\'':
			if ((c = getchar()) < 'a' || c > 'z')
				ERROR;
			for (a1=zero; a1<=dol; a1++)
				if (names[c-'a'] == (*a1|01))
					break;
			break;
	
		default:
			peekc = c;
			if (a1==0)
				return(0);
			a1 =+ minus;
			if (a1<zero) a1=zero;
			else if (a1>dol) a1=dol;
			return(a1);
		}
		if (relerr)
			ERROR;
	}
}

setdot()
{
	if (addr2 == 0) {
		if (dot>dol) ERROR;
		addr1 = addr2 = dot;
		}
	if (addr1 > addr2)
		ERROR;
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
		ERROR;
}

nonzero()
{
	if (addr1 > dol) ERROR;
	if (addr1 <= zero) addr1 = zero+1;
	if (addr2 > dol) addr2 = dol;
	if (addr1>addr2) ERROR;
}

newline()
{
	register c;

	if ((c = getchar()) == '\n')
		return;
	pflag++;
	if((c == 'l') || (c == 'L'))
		listf = 1;
	else if((c == 'p') || (c == 'P'));
	else if((c == 'n') || (c == 'N'))
		listf = 2;
	else ERROR;
	if (getchar() != '\n')
		ERROR;
	return;
}

filename()
{
	register char *p1, *p2;
	register c;

	count[1] = 0;
	c = getchar();
	if (c=='\n' || c==EOF) {
		p1 = whichfile;
		if (*p1==0)
			ERROR;
		p2 = file;
		while (*p2++ = *p1++);
		return;
	}
	if (c!=' ')
		ERROR;
	while ((c = getchar()) == ' ');
	if (c=='\n')
		ERROR;
	p1 = file;
	do {
		*p1++ = c;
	} while ((c = getchar()) != '\n');
	*p1++ = 0;
	if (whichfile[0]==0) {
		p1 = whichfile;
		p2 = file;
		while (*p1++ = *p2++);
	}
}

exfile()
{
	close(io);
	io = -1;
	if (vflag) {
		putd(&count);
		putchar('\n');
	}
}

getzlen()
{
	register char c,zmode;
	register int i;
	while ((c=getchar()) == ' ');
	if (c == '.' || c == '-' || c== '+') {
		zmode = c;
		c = getchar();
		}
	else zmode = '+';
	if (c>='0' && c<= '9') {
		i = c-'0';
		while ((c=getchar()) >= '0' && c <= '9')
			i = i*10 + (c-'0');
		}
	else i = zlength;
	peekc = c;
	newline();
	zlength = i;
	if (addr2 == 0) addr1 = dot;
	switch (zmode) {
		case '.': addr1 =- zlength/2;
			break;
		case '-': addr1 =- zlength-1;
			break;
		}
	addr2 = addr1 + zlength-1;
	}

onintr()
{
	signal(SIGINTR, onintr);
	putchar('\n');
	ERROR;
}

onhup()
{
	char   hupp[20];
	char   *pt;
	
	pt = &hupp;
	*pt++ = 'w';
	*pt++ = ' ';
	while(*hup)	*pt++ = *hup++;
	*pt++ = '\n';
	*pt = '\0';
	globp = &hupp;
	push(&curdepth);
	commands(1);
	pop(&curdepth);
	rmfiles();
	exit(-1);
}

dontgo()
{
	register char c;
	fchanged = 0;
	errfunc("Modified file not written out");
}

errfunc(s)
char *s;
{

	listf = 0;
	puts(s);
	count[0] = 0;
	seek(0, 0, 2);
	pflag = 0;
	globp = 0;
	if (io > 0) {
		close(io);
		io = -1;
	}
	peekc = 0;
	if(curdepth == 2 || curdepth == 3) {
		write(1,"?Error in x file?\n",18);
		close(0);
		dup(savin);
		close(savin);
		vflag = savvflag;
	}
	ttmode[2] = ttnorm;
	stty(0,ttmode);		/* reset tty if necessary */
				/* and flush all pending input */
	stackptr = &deepstk[0];		/* reset stack pointer */
	reset();
				/* returns to top command level */
}

getchar()
{
	char c;
	if(colread)
	{
		if((c = *colbufp++) == '\0')	colread = 0;
		else	return(c&0177);
	}
	if (c=peekc) {
		peekc = 0;
		return(c);
	}
	if (globp) {
		if ((c = *globp++) != 0)
			return(c);
		globp = 0;
		return(EOF);
	}
	if (read(0, &c, 1) <= 0)
		return(c = EOF);
	c =& 0177;
	return(c);
}

gettty()
{
	register c, gf;
	char c2;
	register char *p;
	char *threshold;

	threshold = linebuf + margin;
	p = linebuf;
	gf = globp;
	if(fakend) {
		fakend = 0;
		*p++ = '.';
	}
	else {
		while ((c = getchar()) != '\n') {
			if(yflag) {
				c2 = c & 0177;
				if(c2 == (ttmode[1] & 0177)) {
					if(p != &linebuf[0]) {
						--p;
						write(1,&c2,1);
					}
					continue;
				}
				if(c2 == ((ttmode[1] >> 8) & 0177)) {
					p = linebuf;
					write(1,&c2,1);
					write(1,"\n",1);
					continue;
				}
				if(c2 == '\\') {
					write(1,"\\",1);
					c = getchar();
					if(c != (ttmode[1] & 0177) && c != ((ttmode[1] >> 8) & 0177))		*p++ = '\\';
				}
				c2 = c;
				write(1, &c2, 1);
			}
			if (c==EOF) {
				if (gf)
					peekc = c;
				return(c);
			}
			if ((c =& 0177) == 0)
				continue;
			if((c == ' ') && (p > threshold)) {
				if(!yflag)	putchar('\n');
				break;
			}
			*p++ = c;
			if (p >= &linebuf[LBSIZE-2])
				ERROR;
		}
	}
	if(yflag)	write(1,"\n",1);
	*p++ = 0;
	if(imedmode) {
		imedmode = 0;
		fakend++;
	}
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
		if (lp >= &linebuf[LBSIZE])
			errfunc(BUFERR);
		if ((*lp++ = c = *fp++ & 0177) == 0) {
			lp--;
			continue;
		}
		if (++count[1] == 0)
			++count[0];
	} while (c != '\n');
	*--lp = 0;
	nextip = fp;
	return(0);
}

putfile()
{
	int *a1;
	register char *fp, *lp;
	register int i;

	i = 0;
	fp = genbuf;
	a1 = addr1;
	lp = getline(*a1);
	for (;;) {
		/* i = fp-genbuf;
		   fp points to next place to put char to output
		   genbuf[0..i-1] contains chars created to output
		   a1 = line number of current line being moved
		   lp points to next character to move
		   count[0,1] contains total # of bytes moved
		*/
		if (++count[1] == 0)
			++count[0];
		i++;
		if ((*fp++ = *lp++) == 0) {
			fp[-1] = '\n';
			if (a1++ >= addr2) break;
			lp = getline(*a1);
		}
		if ( i == 512) {
			if (write(io,genbuf,i) != i) errfunc(IOERR);
			fp = genbuf;
			i = 0;
		}
	}
	if (write(io,genbuf,i) != i) errfunc(IOERR);
}

append(f, a)
int (*f)();
{
	register *a1, *a2, *rdot;
	int nline, tl;
	struct { int integer; };

	nline = 0;
	dot = a;
	if(yflag) {
		if(!nostty) {
			ttmode[2] =& ~ ECHO;		/* set no echo */
			ttmode[2] =| 040;
			stty(0,ttmode);
		}
	}
	while ((*f)() == 0) {
		if (dol >= endcore) {
			if (sbrk(1024) == -1)
				errfunc("No more core");
			endcore.integer =+ 1024;
		}
		tl = putline();
		nline++;
		a1 = ++dol;
		a2 = a1+1;
		rdot = ++dot;
		while (a1 > rdot)
			*--a2 = *--a1;
		*rdot = tl;
	}
	if(yflag) {
		if(!nostty) {
			ttmode[2] = ttnorm;
			stty(0,ttmode);
		}
	}
	nostty = 0;
	return(nline);
}

unix()
{
	register savint, pid, rpid;
	char	*p;
	int retcode;

	setnoaddr();
	p = linebuf;
	while ( (*p = getchar()) != '\n' && *p != EOF ) 
		if (p++ >= linebuf + LBSIZE) errfunc(BUFERR);
	*p = 0;
	if ((pid = fork()) == 0) {
		signal(SIGHUP, onhup);
		signal(SIGQUIT, onquit);
		if (*linebuf) execl("/bin/sh","-shell(xed)","-c",linebuf);
		else execl("/bin/sh","-sh");
		ERROR;
	}
	savint = signal(SIGINTR, 1);
	while ((rpid = wait(&retcode)) != pid && rpid != -1);
	signal(SIGINTR, savint);
	if(gtty(0,ttmode) != -1)	ttnorm = ttmode[2];
	puts("!");
}

delete()
{
	register *a1, *a2, *a3;

	a1 = addr1;
	a2 = addr2+1;
	a3 = dol;
	dol =- a2 - a1;
	do
		*a1++ = *a2++;
	while (a2 <= a3);
	a1 = addr1;
	if (a1 > dol)		/* reinserted by jsk */
		a1 = dol;
	dot = a1;
	fchanged =+ a2 - a1;
	if (fchanged>FLUSHLIM) flushio();
}

join()
{
	register char *a1,*a2;
	int  *one, *two;
	int   genlen, len;

	if(addr1 == addr2)	ERROR;
	one = addr1;
	two = addr2;
	a1 = genbuf;
	a2 = getline(*one);
	while (*a1++ = *a2++);
	a1--;
	genlen = a1 - &genbuf;
	one++;
	for(; one <= two; one++)
	{
		a2 = getline(*one);
		while(*a2++);
		a2--;
		len = a2 - &linebuf;
		if((genlen =+ len) >= LBSIZE-1)
		{
			a1 = &genbuf;
			a2 = &linebuf;
			while(*a2++ = *a1++);
			one--;
			*one = putline();
			addr2 = one - 1;
			delete();
			errfunc(JOINERR);
		}
		a2 = &linebuf;
		while (*a1++ = *a2++);
		a1--;
	}
	a1 = &genbuf;
	a2 = &linebuf;
	while(*a2++ = *a1++);
	*addr2 = putline();
	addr2 = one - 2;
	delete();
}

getline(tl)
{
	register char *bp, *lp;
	register nl;

	lp = linebuf;
	bp = getblock(tl, READ);
	nl = nleft;
	tl =& ~0377;
	while (*lp++ = *bp++)
		if (--nl == 0) {
			bp = getblock(tl=+0400, READ);
			nl = nleft;
		}
	return(linebuf);
	}

putline()
{
	register char *bp, *lp;
	register nl;
	int tl;

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
	fchanged++;
	if (fchanged>FLUSHLIM) flushio();
	return(nl);
}

getblock(atl, iof)
{
	extern read(), write();
	register bno, off;
	
	bno = (atl>>8)&0377;
	off = (atl<<1)&0774;
	if (bno >= 255) errfunc(TMPERR);
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
