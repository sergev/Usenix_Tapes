#
/*
 * cc -s -n -O pr.c
 */
/*
 *   print file with headings
 *  2+head+2+page[56 or 68]+5
 *
 * 1. from CAC has following options:
 *    -f  use form feeds instead of multiple line feeds
 *    -n  number each line
 *    -i  number each file and increment file number
 *        each time if number follows the i use it
 *        as starting number
 * 2. from dlm has following options:
 *    -p# use # as the inital page number
 * 3. from RLK at UofMd:
 *    -L  set 8 line per inch mode of printronix
 *        and do an -l88
 *    -S  reset 6 line per inch mode of printronix (default)
 *        and do an -l66
 */
#define  NEW    /* Invoke CAC capabilities */
#define DWIDTH	132
#define BUFS    10500
int	ncol	1;
char	*header;
int	col;
int	icol;
int	file;
char	*bufp;
char	buffer[BUFS];
#define	FF	014
int	line;
char    *colp[DWIDTH];
int	nofile;
char	isclosed[10];
int	peekc;
int	fpage;
int     pageno; /* default pagenumber */
int	page;
int	colw;
int	nspace;
int     width   DWIDTH;
int	length	66;
int	plength 61;
int	margin	10;
int	ntflg;
int	mflg;
int     l88flg; /* Printonix 8 lines per page RLK */
#ifdef NEW
int	numflg;
int	linenum;
int	ffflg;
int	inxflg;
int	fileinx;
#endif NEW
int	tabc;
char	*tty;
int	mode;

struct inode {
	int dev;
	int inum;
	int flags;
	char nlink;
	char uid;
	char gid;
	char siz0;
	int size;
	int ptr[8];
	int atime[2];
	int mtime[2];
};

main(argc, argv)
char **argv;
{
	int nfdone;
	int onintr();
	extern fout;

	tty = "/dev/ttyx";
	fout = dup(1);
	close(1);
	if ((signal(2, 1) & 01) == 0)
		signal(2, onintr);
	fixtty();
	for (nfdone=0; argc>1; argc--) {
		argv++;
		if (**argv == '-') {
			switch (*++*argv) {
			case 'h':
				if( *++*argv ) {
					header = *argv;
					continue;
				}
				if (argc>=2) {
					header = *++argv;
					argc--;
				}
				continue;

			case 't':
				ntflg++;
				continue;

			case 'l':
				length = getn(++*argv);
				continue;

			case 'w':
				width = getn(++*argv);
				continue;

			case 's':
				if (*++*argv)
					tabc = **argv;
				else
					tabc = '\t';
				continue;

			case 'm':
				mflg++;
				continue;

			case 'p':
				pageno = getn(++*argv);
				continue;

			case 'L':
				length = 88;
				l88flg++;
				continue;

			case 'S':
				length = 66;
				l88flg = 0;
				continue;

#ifdef NEW
			case 'n':
				numflg++;
				continue;

			case 'f':
				ffflg++;
				continue;

			case 'i':
				inxflg++;
				fileinx = getn(++*argv);
				continue;

#endif NEW
			default:
				ncol = getn(*argv);
				continue;
			}
		} else if (**argv == '+') {
			fpage = getn(++*argv);
		} else {
			print(*argv, argv);
			nfdone++;
			if (mflg)
				break;
		}
	}
	if (nfdone==0)
		print(0);
	flush();
	onintr();
}

onintr()
{

	chmod(tty, mode);
	exit(0);
}

fixtty()
{
	struct inode sbuf;
	extern fout;

	tty[8] = ttyn(fout);
	fstat(fout, &sbuf);
	mode = sbuf.flags&0777;
	chmod(tty, 0600);
}

print(fp, argp)
char *fp;
char **argp;
{
	struct inode sbuf;
	register int sncol, sheader;
	register char *cbuf;
	extern fout;

	if (ntflg)
		margin = 0;
	else
		margin = 10;
	if (length <= margin)
		length = 66;
	if (width <= 0)
		width = DWIDTH;
	if (ncol>132 || ncol>width) {
		write(2, "Too many cols.\n", 15);
		exit();
	}
	if (mflg) {
		mopen(argp);
		ncol = nofile;
	}
	colw = width/ncol;
	sncol = ncol;
	sheader = header;
	plength = length-5;
	if (ntflg)
		plength = length;
	if (mflg)
		fp = 0;
	if (fp) {
		file = open(fp, 0);
		if (file<0)
			return;
		fstat(file, &sbuf);
#ifdef NEW
		linenum = 1;
#endif NEW
	} else {
		file = 0;
		time(sbuf.mtime);
	}
	if (--ncol<0)
		ncol = 0;
	if (header == 0)
		header = fp;
	cbuf = ctime(sbuf.mtime);
	cbuf[16] = '\0';
	cbuf[24] = '\0';
	page = (pageno?pageno:1);
	icol = 0;
	colp[ncol] = bufp = buffer;
	if (mflg==0)
		nexbuf();
	while (mflg&&nofile || (!mflg)&&tpgetc(ncol)>0) {
		if (mflg==0) {
			colp[ncol]--;
			if (colp[ncol] < buffer)
				colp[ncol] = &buffer[BUFS];
		}
		line = 0;
		if (ntflg==0) {
#ifdef NEW
			savebuf ();
#endif	NEW
			puts("\n\n");
			puts(cbuf+4);
			puts(" ");
			puts(cbuf+20);
			puts("  ");
			puts(header);
			puts(" Page ");
#ifndef NEW
			putd(page);
#endif NEW
#ifdef NEW
			putd(page, 5);
#endif NEW
			puts("\n\n\n");
#ifdef	NEW
			flush ();
			restbuf ();
#endif	NEW
		}
		putpage();
		if (ntflg==0)
#ifdef NEW
			if (ffflg)
				write (fout, "\014", 1);
			else
				while (line++ <length)  {
					if(l88flg) write (fout, "\006", 1);
					write (fout, "\n", 1);
				}
#endif NEW
#ifndef	NEW
			while(line<length)
				put('\n');
#endif	NEW
		page++;
	}
	if (file)
		close (file);
#ifdef	NEW
	if (file)
		fileinx++;
#endif	NEW
	ncol = sncol;
	header = sheader;
}

mopen(ap)
char **ap;
{
	register char **p, *p1;

	p = ap;
	while ((p1 = *p++) && p1 != -1) {
		isclosed[nofile] = fopen(p1, &buffer[2*259*nofile]);
		if (++nofile>=10) {
			write(2, "Too many args.\n", 15);
			exit();
		}
	}
}

putpage()
{
	register int lastcol, i, c;
	int j;
#ifdef NEW
	int fffound;

	fffound = 0;
#endif	NEW

	if (ncol==0) {
		while (line<plength) {
#ifdef NEW
			if (numflg) {
				if (inxflg) {
					putd (fileinx, 3);
					put ('-');
				}
				putd ( linenum++, 5);
				put ('\t');
			}
#endif NEW
			while((c = tpgetc(0)) && c!='\n' && c!=FF)
				putcp(c);
#ifdef	NEW
			if (c == FF) {
				fffound++;
				while ((c = tpgetc(0)) && c != '\n')
					if (c != FF)
						putcp (c);
			}
			if (c == 0) {
				savebuf (); /* clear buf too */
				return ;
			}
#endif	NEW
			putcp('\n');
			line++;
#ifdef	NEW
			if (fffound)
				break;
			else
				flush ();
#endif	NEW
#ifndef	NEW
			if (c==FF)
				break;
#endif	NEW
		}
		return;
	}
	colp[0] = colp[ncol];
	if (mflg==0) for (i=1; i<=ncol; i++) {
		colp[i] = colp[i-1];
		for (j = margin; j<length; j++)
			while((c=tpgetc(i))!='\n')
				if (c==0)
					break;
	}
	while (line<plength) {
		lastcol = colw;
		for (i=0; i<ncol; i++) {
			while ((c=pgetc(i)) && c!='\n')
				if (col<lastcol || tabc!=0)
					put(c);
#ifdef NEW
			if (c==0)
				continue;
#endif NEW
#ifndef NEW
 /*del by lab*/ /***    if (c==0 && ntflg)                   ***/
 /*17 Nov 76 */ /***            return;                      ***/
#endif NEW
			if (tabc)
				put(tabc);
			else while (col<lastcol)
				put(' ');
			lastcol =+ colw;
		}
		while ((c = pgetc(ncol)) && c!='\n')
			put(c);
		put('\n');
	}
	flush ();
}

nexbuf()
{
	register int n;
	register char *rbufp;

	rbufp = bufp;
	n = &buffer[BUFS] - rbufp;
	if (n>512)
		n = 512;
	if ((n = read(file, rbufp, n)) <= 0)
		*rbufp = 0376;
	else {
		rbufp =+ n;
		if (rbufp >= &buffer[BUFS])
			rbufp = buffer;
		*rbufp = 0375;
	}
	bufp = rbufp;
}

tpgetc(ai)
{
	register char **p;
	register int c, i;

	i = ai;
	if (mflg) {
		if ((c = getc(&buffer[2*259*i])) < 0) {
			if (isclosed[i]==0) {
				isclosed[i] = 1;
				if (--nofile <= 0)
					return(0);
			}
			return('\n');
		}
		if (c==FF && ncol>0)
			c = '\n';
		return(c);
	}
loop:
	c = **(p = &colp[i]) & 0377;
	if (c == 0375) {
		nexbuf();
		c = **p & 0377;
	}
	if (c == 0376)
		return(0);
	(*p)++;
	if (*p >= &buffer[BUFS])
		*p = buffer;
	if (c==0)
		goto loop;
	return(c);
}

pgetc(i)
{
	register int c;

	if (peekc) {
		c = peekc;
		peekc = 0;
	} else
		c = tpgetc(i);
	if (tabc)
		return(c);
	switch (c) {

	case '\t':
		icol++;
		if ((icol&07) != 0)
			peekc = '\t';
		return(' ');

	case '\n':
		icol = 0;
		break;

	case 010:
	case 033:
		icol--;
		break;
	}
	if (c >= ' ')
		icol++;
	return(c);
}

puts(as)
char *as;
{
	register int c;
	register char *s;

	if ((s=as)==0)
		return;
	while (c = *s++)
		put(c);
}


#ifdef NEW

int sizes [] {0,0,10,100,1000,10000};

putd(an,cols)
{
	register int a, n;

	n = an;
	if (n < sizes [cols]) {
		putcp (' ');
		putd (n, cols-1);
		return;
	}
	if (a = n/10)
		putd(a, cols-1);
	put(n%10 + '0');
}
#endif NEW

#ifndef NEW
putd(an)
{
	register int a, n;

	n = an;
	if (a = n/10)
		putd(a);
	put(n%10 + '0');
}
#endif NEW
put(ac)
{
	register int ns, c;

	c = ac;
	if (tabc) {
		putcp(c);
		if (c=='\n')
			line++;
		return;
	}
	switch (c) {

	case ' ':
		nspace++;
		col++;
		return;

	case '\n':
		col = 0;
		nspace = 0;
		line++;
		break;

	case 010:
	case 033:
		if (--col<0)
			col = 0;
		if (--nspace<0)
			nspace = 0;

	}
	while(nspace) {
		if (nspace>2 && col > (ns=((col-nspace)|07))) {
			nspace = col-ns-1;
			putcp('\t');
		} else {
			nspace--;
			putcp(' ');
		}
	}
	if (c >= ' ')
		col++;
	putcp(c);
}

getn(ap)
char *ap;
{
	register int n, c;
	register char *p;

	p = ap;
	n = 0;
	while ((c = *p++) >= '0' && c <= '9')
		n = n*10 + c - '0';
	return(n);
}

putcp(c)
{
	if (page >= fpage) {
		/* Set to 8 lines per inch for this line RLK */
		if(l88flg && c=='\n') putchar(6);
		putchar(c);
	}
}

#ifdef NEW
int saveplace [259];

savebuf ()
{
	int i, *p1, *p2;
	extern int fout [];

	p1 = fout;
	p2 = saveplace;

	for (i = 0; i < 259; i++)
		*p2++ = *p1++;
	fout [1] = 512;
	fout [2] = &fout [3];
}

restbuf ()
{
	int i, *p1, *p2;
	extern fout;

	p1 = &fout;
	p2 = saveplace;

	for (i = 0; i < 259; i++)
		*p1++ = *p2++;
}

#endif	NEW
