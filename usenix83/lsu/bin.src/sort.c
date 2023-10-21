/*
 * PWB sort
 */
/*
		REVISION  HISTORY

	SDK  09/23/81  	Added ctime to the temporary sort file name
			so that now simultaneous sorts can be run by
			a single user.  This should take care of the
			scrambled file problems.

	SDK  07/18/82	Increased L to 1024 to handle very long lines.

*/
#define	L	1024
#define	N	7
#define	C	20
#define	MEM	(16*2048)
#define NF	10

int	ibuf[259];
int	obuf[259];
char    *file;
char	*filep;
int	nfiles;
int	nlines;
int	ntext;
int	*lspace;
char	*tspace;
int	cmp(), cmpa();
int	(*compare)() cmpa;
int	term();
int 	mflg;
int	uflg;
char	*outfil;
char	tabchar;
int 	eargc;
char	**eargv;
char *cpid;

char zero[256];

char	fold[256] {
	0200,0201,0202,0203,0204,0205,0206,0207,
	0210,0211,0212,0213,0214,0215,0216,0217,
	0220,0221,0222,0223,0224,0225,0226,0227,
	0230,0231,0232,0233,0234,0235,0236,0237,
	0240,0241,0242,0243,0244,0245,0246,0247,
	0250,0251,0252,0253,0254,0255,0256,0257,
	0260,0261,0262,0263,0264,0265,0266,0267,
	0270,0271,0272,0273,0274,0275,0276,0277,
	0300,0301,0302,0303,0304,0305,0306,0307,
	0310,0311,0312,0313,0314,0315,0316,0317,
	0320,0321,0322,0323,0324,0325,0326,0327,
	0330,0331,0332,0333,0334,0335,0336,0337,
	0340,0341,0342,0343,0344,0345,0346,0347,
	0350,0351,0352,0353,0354,0355,0356,0357,
	0360,0361,0362,0363,0364,0365,0366,0367,
	0370,0371,0372,0373,0374,0375,0376,0377,
	0000,0001,0002,0003,0004,0005,0006,0007,
	0010,0011,0012,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0024,0025,0026,0027,
	0030,0031,0032,0033,0034,0035,0036,0037,
	0040,0041,0042,0043,0044,0045,0046,0047,
	0050,0051,0052,0053,0054,0055,0056,0057,
	0060,0061,0062,0063,0064,0065,0066,0067,
	0070,0071,0072,0073,0074,0075,0076,0077,
	0100,0101,0102,0103,0104,0105,0106,0107,
	0110,0111,0112,0113,0114,0115,0116,0117,
	0120,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0132,0133,0134,0134,0136,0137,
	0140,0101,0102,0103,0104,0105,0106,0107,
	0110,0111,0112,0113,0114,0115,0116,0117,
	0120,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0132,0173,0174,0175,0176,0177
};
char nofold[256] {
	0200,0201,0202,0203,0204,0205,0206,0207,
	0210,0211,0212,0213,0214,0215,0216,0217,
	0220,0221,0222,0223,0224,0225,0226,0227,
	0230,0231,0232,0233,0234,0235,0236,0237,
	0240,0241,0242,0243,0244,0245,0246,0247,
	0250,0251,0252,0253,0254,0255,0256,0257,
	0260,0261,0262,0263,0264,0265,0266,0267,
	0270,0271,0272,0273,0274,0275,0276,0277,
	0300,0301,0302,0303,0304,0305,0306,0307,
	0310,0311,0312,0313,0314,0315,0316,0317,
	0320,0321,0322,0323,0324,0325,0326,0327,
	0330,0331,0332,0333,0334,0335,0336,0337,
	0340,0341,0342,0343,0344,0345,0346,0347,
	0350,0351,0352,0353,0354,0355,0356,0357,
	0360,0361,0362,0363,0364,0365,0366,0367,
	0370,0371,0372,0373,0374,0375,0376,0377,
	0000,0001,0002,0003,0004,0005,0006,0007,
	0010,0011,0012,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0024,0025,0026,0027,
	0030,0031,0032,0033,0034,0035,0036,0037,
	0040,0041,0042,0043,0044,0045,0046,0047,
	0050,0051,0052,0053,0054,0055,0056,0057,
	0060,0061,0062,0063,0064,0065,0066,0067,
	0070,0071,0072,0073,0074,0075,0076,0077,
	0100,0101,0102,0103,0104,0105,0106,0107,
	0110,0111,0112,0113,0114,0115,0116,0117,
	0120,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0132,0133,0134,0135,0136,0137,
	0140,0141,0142,0143,0144,0145,0146,0147,
	0150,0151,0152,0153,0154,0155,0156,0157,
	0160,0161,0162,0163,0164,0165,0166,0167,
	0170,0171,0172,0173,0174,0175,0176,0177
};

char	nonprint[256] {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1
};

char	dict[256] {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1
};

struct	field {
	char *code;
	char *ignore;
	int nflg;
	int rflg;
	int bflg;
	char *m[2];
	char *n[2];
}	fields[NF];
int proto[9] {
	nofold+128,
	zero+128,
	0,
	1,
	0,
	0,-1,
	0,0
};
int	nfields;
int 	error 1;
char *setfil();

main(argc, argv)
char **argv;
{
	register a, i;
	char *arg;
	int index;
	int tbuf[2];char *c;
	register int *p;
	int *q;
	int n;

	copyproto();
	eargv = argv;
	while (--argc > 0) {
		if(**++argv == '-') for(arg = *argv;;) {
			switch(*++arg) {
			case '\0':
				if(arg[-1] == '-')
					eargv[eargc++] = "-";
				break;

			case 'o':
				if(--argc > 0)
					outfil = *++argv;
				continue;

			default:
				field(++*argv,1);
				break;
			}
			break;
		} else if (**argv == '+') {
			if(++nfields>=NF) {
				mess("Too many keys\n");
				exit(1);
			}
			copyproto();
			field(++*argv,0);
		} else
			eargv[eargc++] = *argv;
	}
	q = &fields[0];
	for(a=1; a<=nfields; a++) {
		p = &fields[a];
		for(i=0; i<5; i++)	/*sensitive to sizeof(proto)*/
			if(p[i] != proto[i])
				goto next;
		for(i=0; i<5; i++)
			p[i] = q[i];
next:	;
	}
	if(eargc == 0)
		eargv[eargc++] = "-";

	a = MEM;
	i = lspace = sbrk(0);
	while(brk(a) == -1)
		a =- 512;
	brk(a =- 512);	/* for recursion */
	a =- i;
	nlines = ((a-L)>>1) & 077777;
	nlines =/ 5;
	ntext = nlines*8;
	tspace = lspace+nlines;
	n=getpid();
	file = "/tmp/stmXaa       ";
	time(tbuf);
	c = ctime(tbuf);
	for (index=11;index<19;index++) file[index] = *(c+index);
	filep = file;
	while(*filep != 'X')
		filep++;
	for(*filep = 'a';;(*filep)++) {
		if(stat(file, lspace) < 0) {
			a = creat(file, 0600);
			if(a >= 0)
				break;
		}
		if(*filep == 'z') {
/*
			if(file[1] != 't') {
				file = "/tmp/stmXaa";
				goto loop;
			}
*/
			mess("Cannot locate temp\n");
			exit(1);
		}
	}
	close(a);
	filep++;
	if ((signal(2, 1) & 01) == 0)
		signal(2, term);
	if ((signal(13, 1) & 01) == 0)
		signal(13, term);
	nfiles = eargc;
	if(!mflg) {
		ibuf[0] = -1;
		sort();
		close(0);
	}
	for(a = mflg?0:eargc; a+N < nfiles; a=+N) {
		newfile();
		merge(a, a+N);
	}
	if(a != nfiles) {
		oldfile();
		merge(a, nfiles);
	}
	error = 0;
	term();
}

sort()
{
	register char *cp;
	register *lp, c;
	int done;
	int i;
	int f;

	done = 0;
	i = 0;
	do {
		cp = tspace;
		lp = lspace;
		while(lp < lspace+nlines && cp < tspace+ntext) {
			*lp++ = cp;
			while((*cp++ = c = getc(ibuf)) != '\n')  {
				/*
				 * Strip all NULLS in the input
				 * Yes, you're right, this is a bit kludgy
				 */
				if(!c) cp--;	/* back up over null */
				if(c >= 0) continue;
				cp--;
				close(ibuf[0]);
				if(i < eargc) {
					if((f = setfil(i++)) == 0)
						ibuf[0] = 0;
					else if(fopen(f, ibuf) < 0)
						cant(f);
				} else
					break;
			}
			if(c < 0) {
				done++;
				lp--;
				break;
			}
		}
		qsort(lspace, lp-lspace, 2, compare);
		if(done == 0 || nfiles != eargc)
			newfile(); else
			oldfile();
		while(lp > lspace) {
			cp = *--lp;
			if(*cp)
				do
				putc(*cp, obuf);
				while(*cp++ != '\n');
		}
		fflush(obuf);
		close(obuf[0]);
	} while(done == 0);
}

struct merg
{
	char	l[L];
	int	b[259];
};

merge(a,b)
{
	struct	merg	*p;
	register char	*cp, *dp;
	register	i;
	struct	{int	*ip;};
	int	f;
	int	j;
	int	k, l, c;
	int	muflg;

	p = lspace;
	j = 0;
	for(i=a; i < b; i++) {
		f = setfil(i);
		if(f == 0)
			p->b[0] = dup(0);
		else if(fopen(f, p->b) < 0)
			cant(f);
		ibuf[j] = p;
		if(!rline(p))	j++;
		p++;
	}

	do {
		i = j;
		qsort(ibuf, i, 2, compare);
		l = 0;
		while(i--) {
			cp = ibuf[i];
			if(*cp == '\0') {
				l = 1;
				if(rline(ibuf[i])) {
					k = i;
					while(++k < j)
						ibuf[k-1] = ibuf[k];
					j--;
				}
			}
		}
	} while(l);

	muflg = mflg & uflg;
	i = j;
	while(i > 0) {
		cp = ibuf[i-1];
		if(uflg == 0 || muflg || (*compare)(&ibuf[i-1],&ibuf[i-2]))
			do
				putc(*cp, obuf);
			while(*cp++ != '\n');
		if(muflg){
			cp = ibuf[i-1];
			dp = p;
			do {
			} while((*dp++ = *cp++) != '\n');
		}
		do {
			if(rline(ibuf[i-1])) {
				i--;
				if(i == 0)
					break;
				if(i == 1)
					muflg = uflg;
			}
			cp = &ibuf[i];
			while(--cp.ip>ibuf&&(*compare)(cp.ip,cp.ip-1)<0){
				dp = *cp.ip;
				*cp.ip = *(cp.ip-1);
				*(cp.ip-1) = dp;
			}
		} while(muflg && (*compare)(&ibuf[i-1],&p) == 0);
	}
	p = lspace;
	for(i=a; i<b; i++) {
		close(p->b[0]);
		p++;
		if(i >= eargc)
			close(creat(setfil(i)));
	}
	fflush(obuf);
	close(obuf[0]);
}

rline(mp)
struct merg *mp;
{
	register char *cp;
	register *bp, c;

	bp = mp->b;
	cp = mp->l;
	do {
		c = getc(bp);
		/*
		 * this strips off nulls in input, which are often left
		 * in by BASIC and other programs frequently run by WCG.
		 */
		if(c == 0) continue;
		if(c < 0)
			return(1);
		*cp++ = c;
	} while(c != '\n');
	return(0);
}

newfile()
{

	if(fcreat(setfil(nfiles), obuf) < 0) {
		mess("Can't create temp\n");
		term();
	}
	nfiles++;
}

char *
setfil(i)
{

	if(i < eargc)
		if(eargv[i][0] == '-' && eargv[i][1] == '\0')
			return(0);
		else
			return(eargv[i]);
	i =- eargc;
	filep[0] = i/26 + 'a';
	filep[1] = i%26 + 'a';
	return(file);
}

oldfile()
{

	if(outfil) {
		if(fcreat(outfil, obuf) < 0) {
			mess("Can't create output\n");
			term();
		}
	} else
		obuf[0] = 1;
}

cant(f)
{
	mess("Can't open ");
	mess(f);
	mess("\n");
	term();
}

term()
{
	register i;

	if(nfiles == eargc)
		nfiles++;
	for(i=eargc; i<nfiles; i++) {
		unlink(setfil(i));
	}
	exit(error);
}

cmp(i, j)
int *i, *j;
{
	register char *pa, *pb;
	char *code, *ignore;
	int a, b;
	int k;
	char *la, *lb;
	register int sa;
	int sb;
	char *ipa, *ipb, *jpa, *jpb;
	struct field *fp;

	for(k = nfields>0; k<=nfields; k++) {
		fp = &fields[k];
		pa = *i;
		pb = *j;
		if(k) {
			la = skip(pa, fp, 1);
			pa = skip(pa, fp, 0);
			lb = skip(pb, fp, 1);
			pb = skip(pb, fp, 0);
		} else {
			la = -1;
			lb = -1;
		}
		if(fp->nflg) {
			while(blank(*pa))
				pa++;
			while(blank(*pb))
				pb++;
			sa = sb = fp->rflg;
			if(*pa == '-') {
				pa++;
				sa = -sa;
			}
			if(*pb == '-') {
				pb++;
				sb = -sb;
			}
			if(sa != sb)
				sa = 0;
			for(ipa = pa; ipa<la&&digit(*ipa); ipa++);
			for(ipb = pb; ipb<lb&&digit(*ipb); ipb++);
			jpa = ipa;
			jpb = ipb;
			a = 0;
			if(sa) while(ipa > pa && ipb > pb)
				if(b = *--ipb - *--ipa)
					a = b;
			while(ipa > pa)
				if(*--ipa != '0')
					return(sa ? -sa : sb);
			while(ipb > pb)
				if(*--ipb != '0')
					return(sa ? sa : sb);
			if(a) return(a*sa);
			if(*(pa=jpa) == '.')
				pa++;
			if(*(pb=jpb) == '.')
				pb++;
			while(pa<la && digit(*pa))
				if(pb<lb &&digit(*pb)) {
					if(a = *pb++ - *pa++)
						return(sa ? a*sa : sb);
				} else if(*pa++ != '0')
					return(sa ? -sa : sb);
			while(pb<lb && digit(*pb))
				if(*pb++ != '0')
					return(sa ? sa : sb);
			continue;
		}
		code = fp->code;
		ignore = fp->ignore;
loop: 
		while(ignore[*pa])
			pa++;
		while(ignore[*pb])
			pb++;
		if(pa>=la || *pa=='\n')
			if(pb<lb && *pb!='\n')
				return(fp->rflg);
			else continue;
		if(pb>=lb || *pb=='\n')
			return(-fp->rflg);
		if((sa = code[*pb++]-code[*pa++]) == 0)
			goto loop;
		return(sa*fp->rflg);
	}
	if(uflg)
		return(0);
	return(cmpa(i, j));
}

cmpa(i, j)
int *i, *j;
{
	register char *pa, *pb;
	pa = *i;
	pb = *j;
	while(*pa == *pb) {
		if(*pa++ == '\n')
			return(0);
		pb++;
	}
	return(
		*pa == '\n' ? fields[0].rflg:
		*pb == '\n' ?-fields[0].rflg:
		*pb > *pa   ? fields[0].rflg:
		-fields[0].rflg
	);
}

skip(pp, fp, j)
struct field *fp;
char *pp;
{
	register i;
	register char *p;

	p = pp;
	if( (i=fp->m[j]) < 0)
		return(-1);
	while(i-- > 0) {
		if(tabchar != 0) {
			while(*p != tabchar)
				if(*p != '\n')
					p++;
				else goto ret;
			p++;
		} else {
			while(blank(*p))
				p++;
			while(!blank(*p))
				if(*p != '\n')
					p++;
				else goto ret;
		}
	}
	if(fp->bflg)
		while(blank(*p))
			p++;
	i = fp->n[j];
	while(i-- > 0) {
		if(*p != '\n')
			p++;
		else goto ret;
	} 
ret:
	return(p);
}

digit(c)
{

	return(c <= '9' && c >= '0');
}

mess(s)
char *s;
{
	while(*s)
		write(2, s++, 1);
}

copyproto()
{
	register int i, *p, *q;

	p = proto;
	q = &fields[nfields];
	for(i=0; i<sizeof(proto)/2; i++)
		*q++ = *p++;
}

field(s,k)
char *s;
{
	register struct field *p;
	register d;
	p = &fields[nfields];
	d = 0;
	for(; *s!=0; s++) {
		switch(*s) {
		case '\0':
			return;

		case 'b':
			p->bflg++;
			break;

		case 'd':
			p->ignore = dict+128;
			break;

		case 'f':
			p->code = fold+128;
			break;
		case 'i':
			p->ignore = nonprint+128;
			break;

		case 'm':
			mflg++;
			continue;

		case 'n':
			p->nflg++;
			break;
		case 't':
			tabchar = *++s;
			if(tabchar == 0) s--;
			continue;

		case 'r':
			p->rflg = -1;
			continue;
		case 'u':
			uflg++;
			break;

		case '.':
			if(p->m[k] == -1)	/* -m.n with m missing */
				p->m[k] = 0;
			d = &fields[0].n[0]-&fields[0].m[0];

		default:
			p->m[k+d] = number(&s);
		}
		compare = cmp;
	}
}

number(ppa)
char **ppa;
{
	int n;
	register char *pa;
	pa = *ppa;
	n = 0;
	while(digit(*pa)) {
		n = n*10 + *pa - '0';
		*ppa = pa++;
	}
	return(n);
}

blank(c)
{
	if(c==' ' || c=='\t')
		return(1);
	return(0);
}

int	(*qscmp)();
int	qses;

qsort(a, n, es, fc)
char *a;
int n, es;
int (*fc)();
{
	qscmp = fc;
	qses = es;
	qs1(a, a+n*es);
}

qs1(a, l)
char *a, *l;
{
	register char *i, *j, *es;
	char **k;
	char *lp, *hp;
	int n, c;


	es = qses;

start:
	if((n=l-a) <= es)
		return;


	n = ((n/(2*es))*es) & 077777;
	hp = lp = a+n;
	i = a;
	j = l-es;


	for(;;) {
		if(i < lp) {
			if((c = (*qscmp)(i, lp)) == 0) {
				qsexc(i, lp =- es);
				continue;
			}
			if(c < 0) {
				i =+ es;
				continue;
			}
		}

loop:
		if(j > hp) {
			if((c = (*qscmp)(hp, j)) == 0) {
				qsexc(hp =+ es, j);
				goto loop;
			}
			if(c > 0) {
				if(i == lp) {
					qstexc(i, hp =+ es, j);
					i = lp =+ es;
					goto loop;
				}
				qsexc(i, j);
				j =- es;
				i =+ es;
				continue;
			}
			j =- es;
			goto loop;
		}


		if(i == lp) {
			if(uflg)
				for(k=lp+2; k<=hp;) *(*k++)='\0';
			if(lp-a >= l-hp) {
				qs1(hp+es, l);
				l = lp;
			} else {
				qs1(a, lp);
				a = hp+es;
			}
			goto start;
		}


		qstexc(j, lp =- es, i);
		j = hp =- es;
	}
}

qsexc(i, j)
char *i, *j;
{
	register char *ri, *rj, c;
	int n;

	n = qses;
	ri = i;
	rj = j;
	do {
		c = *ri;
		*ri++ = *rj;
		*rj++ = c;
	} while(--n);
}

qstexc(i, j, k)
char *i, *j, *k;
{
	register char *ri, *rj, *rk;
	char	c;
	int	n;

	n = qses;
	ri = i;
	rj = j;
	rk = k;
	do {
		c = *ri;
		*ri++ = *rk;
		*rk++ = *rj;
		*rj++ = c;
	} while(--n);
}
