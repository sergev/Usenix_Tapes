#

/*
 * incremental dump
 * dump fisbuodhav filesystem
 * f take output tape from arglist
 * i from date in /etc/dtab
 * s specify tape size in feet (feet = blocks/9)
 * b specify tape size in blocks
 * u update /etc/dtab to current date
 * 0 dump from the epoch
 * d dump specified number of days
 * h dump specified number of hours
 * a on incremental dump, dump files even >= MAXSIZE
 * l write 50 block records on dump tape - must use raw device.
 * t update /etc/dtab to current date now - without taking a dump
 */


#include "/usr/sys/ino.h"
#include "/usr/sys/filsys.h"
#define	MAXSIZE	1000
/* LBUFZ must be a multiple of 256 */
#define LBUFZ 12800
struct filsys sblock;
struct
{
	char	name[16];
	int	date[2];
} dtab[15];
#define	DTABSIZE 15
char	*dfile	"/etc/dtab";
char	*ofile;
int	*talist;
int	fi;
int	buf[256];
int	dbuf[256];
int	ibuf[256];
int	vbuf[256];
int	lbuf[LBUFZ];	/* long tape buffer */
int	*lbpaddr;
int	*lbpsv;
char	*date[2];
char	*ddate[2];
int	fo	-1;
int	pher;
int	dflg;
int	tflg;	/* update date now */
int	iflg;
int	cflg;
int	aflg;
int	vflg;	/* verify mode option flag */
int	oflg;	/* write dump on standard output */
char	*tsize	19000;
char	*taddr;
char	*dumpdev;
int	filenum;
int	fout;	/* file des for printf output */

main(argc, argv)
char **argv;
{
	char  *shead;
	char *key;
	int s, i, nfil, nblk, f;
	register *tap;
	register struct inode *ip;
	int ino;

	lbpaddr = lbuf;
	ofile = "/dev/mt0";
	time(date);
	if(argc == 1) {
		f = open(dfile, 0);
		if (f >= 0) {
			read (f, dtab, sizeof dtab);
			close(f);
			for (i=0; i<DTABSIZE&dtab[i].name[0]; i++)
				printf ("%s\t%s", dtab[i].name,
					ctime(dtab[i].date));
			}
		exitt(0);
	}

	argc--;
	argv++;
	key = *argv;
	while(*key)
	switch(*key++) {

	default:
		printf("bad character in key\n");
		exitt(1);

	case 'a': /* dump all (even large) */
		aflg++;
		continue;

	case '-':
		continue;

	case 'c': /* increment file name */
		cflg++;
		continue;

	case 'f': /* file name from arg list */
		argc--;
		argv++;
		ofile = *argv;
		continue;

	case 'i': /* date from date file */
		iflg++;
		continue;

	case 's': /* tape size */
		tsize = number(argv[1]) * 9;
		argv++;
		argc--;
		continue;

	case 'b': /* tape size */
		tsize = number(argv[1]);
		argv++;
		argc--;
		continue;

	case 'u': /* rewrite date */
		dflg++;
		continue;

	case 't': /* rewrite date without doing dump */
		tflg++;
		continue;

	case '0': /* dump all */
		ddate[0] = ddate[1] = 0;
		continue;

	case 'd': /* dump some number of days */
		i = 21600;
		goto sd;

	case 'h': /* dump some number of hours */
		i = 900;
		goto sd;

	case 'v':  /* verify mode */
		vflg = 1;
		continue;

	case 'o':	/* write on standard output */
		fout = 2;
		oflg++;
		continue;

	case 'l': /* long tape buffer */
		lbpsv = lbuf;
		continue;

	sd:
		ddate[0] = date[0];
		ddate[1] = date[1];
		s = number(argv[1])*4;
		argv++;
		argc--;
		while(s) {
			if(i > ddate[1])
				ddate[0]--;
			ddate[1] =- i;
			s--;
		}
		continue;
	}
	if(argc <= 1) {
		printf("no file system specified\n");
		exitt(1);
	}
	dumpdev = argv[1];
	if(tflg)
		goto dte;
	if(iflg) {
		f = open(dfile, 0);
		if(f >= 0) {
			read(f, dtab, sizeof dtab);
			close(f);
			for(i=0; i<DTABSIZE; i++)
				if(equal(dtab[i].name, dumpdev)) {
					ddate[0] = dtab[i].date[0];
					ddate[1] = dtab[i].date[1];
				}
		}
	}
	printf("%s:\n", dumpdev);
	fi = open(dumpdev, 0);
	if(fi < 0) {
		printf("cannot open %s\n", dumpdev);
		exitt(1);
	}
	printf("incremental dump ");
	if (vflg)
		printf ("with verification ");
	printf ("from\n");
	pdate(ddate);
	sync();
	bread(1, &sblock);
	talist = sbrk(size(0, sblock.s_isize*32)*512);
	tap = talist;
	if(tap == -1) {
		printf("No memory\n");
		exitt(1);
	}
	nfil = 0;
	nblk = size(0, sblock.s_isize*32);
	ino = 0;
	for(i=0; i<sblock.s_isize; i++) {
		bread(i+2, buf);
		for(ip = &buf[0]; ip < &buf[256]; ip++) {
			ino++;
			if((ip->i_mode&IALLOC) == 0 || ip->i_nlink == 0) {
				*tap++ = -1;
				continue;
			}
			if(ip->i_mtime[0] < ddate[0])
				goto no;
			if(ip->i_mtime[0] == ddate[0] &&
			   ip->i_mtime[1] <  ddate[1])
				goto no;
			s = size(ip->i_size0&0377, ip->i_size1) + 1;
			if (s>MAXSIZE && aflg==0 && iflg!=0) {
				printf("%l big; not dumped.\n", ino);
				goto no;
			}
			nfil++;
			nblk =+ s;
			*tap++ = s;
			continue;
		no:
			*tap++ = 0;
		}
	}
	printf("%l files\n%l blocks\n", nfil, nblk);
	i = ldiv(0, nblk, ldiv(0, tsize, 10));
	printf("%l.%l tapes\n", i/10, i%10);
	tap = buf;
	clrbuf(tap);
	*tap++ = sblock.s_isize;
	*tap++ = sblock.s_fsize;
	*tap++ = date[0];
	*tap++ = date[1];
	*tap++ = ddate[0];
	*tap++ = ddate[1];
	*tap++ = tsize;
	key = tap;
	shead = &"s-block for ";
	while ( (*key++)=(*shead++) );
	shead = dumpdev;
	while ( (*key++)=(*shead++) );
	swrite(buf);
	i = size(0, sblock.s_isize*32);
	tap = talist;
	while(i--) {
		bwrite(tap);
		tap =+ 256;
	}
	tap = talist;
	filenum = 0;
	for(i=0; i<sblock.s_isize; i++) {
		bread(i+2, buf);
		for(ip = &buf[0]; ip < &buf[256]; ip++) {
			filenum++;
			if(*tap && *tap != -1)
				dump(ip, *tap-1);
			tap++;
		}
	}
	printf("%l phase errors\n", pher);
	if(!dflg)
		exitt(0);
dte:
	for(i=0; i<DTABSIZE; i++)
		dtab[i].name[0] = 0;
	f = open(dfile, 2);
	if(f < 0) {
		f = creat(dfile, 0666);
		if(f < 0) {
			printf("cannot create %s\n", dfile);
			exitt(1);
		}
	} else
		read(f, dtab, sizeof dtab);
	for(i=0; i<DTABSIZE; i++)
		if(dtab[i].name[0] == 0 || equal(dtab[i].name, dumpdev))
			goto found;
	printf("%s full\n", dfile);
	exitt(1);

found:
	for(s=0; s<15; s++) {
		dtab[i].name[s] = dumpdev[s];
		if(dumpdev[s] == 0)
			break;
	}
	dtab[i].date[0] = date[0];
	dtab[i].date[1] = date[1];
	seek(f, 0, 0);
	write(f, dtab, sizeof dtab);
	printf("date updated\n");
	pdate(date);
exitt(0);
}

pdate(d)
int *d;
{

	if(d[0] == 0 && d[1] == 0)
		printf("the epoch\n"); else
		printf(ctime(d));
}

dump(ip, sz)
struct inode *ip;
{
	register *p, *q, *r;
	char  *fp, *fhead;

	p = dbuf;
	q = ip;
	clrbuf(p);
	while(q < &ip->i_mtime[2])
		*p++ = *q++;
	fp = p;
	fhead = &("file header");
	while ( (*fp++)=(*fhead++) );
	p = p+6;
	*p++ = filenum;
	q = ip;
	while (q < &ip->i_mtime[2])
		*p++ = *q++;
	q = ip;
	while (q < &ip->i_mtime[2])
		*p++ = *q++;
	swrite(dbuf);
	if(ip->i_mode & (IFBLK&IFCHR)) {
		if(sz != 0)
			printf("file %d is a special file\n", filenum);
		return;
	}
	for(p = &ip->i_addr[0]; p < &ip->i_addr[8]; p++)
	if(*p) {
		if(ip->i_mode&ILARG) {
			bread(*p, ibuf);
			for(q = &ibuf[0]; q < &ibuf[256]; q++)
			if(*q) {
				if(p == &ip->i_addr[7]) {
					bread(*q, vbuf);
					for(r = &vbuf[0]; r < &vbuf[256]; r++)
					if(*r) {
						if(--sz < 0)
							goto pe;
						bread(*r, dbuf);
						bwrite(dbuf);
					}
					continue;
				}
				if(--sz < 0)
					goto pe;
				bread(*q, dbuf);
				bwrite(dbuf);
			}
		} else {
			if(--sz < 0)
				goto pe;
			bread(*p, dbuf);
			bwrite(dbuf);
		}
	}
	if(sz)
		goto pe;
	return;

pe:
	clrbuf(dbuf);
	while(--sz >= 0)
		bwrite(dbuf);
	pher++;
	printf("phase error on file %d\n", filenum);
}

bread(bno, b)
{

	seek(fi, bno, 3);
	if(read(fi, b, 512) != 512) {
		printf("read error %l\n", bno);
	}
}

clrbuf(b)
int *b;
{
	register i, *p;

	p = b;
	i = 256;
	while(i--)
		*p++ = 0;
}

swrite(b)
int *b;
{
	register i, s, *p;

	i = 254;
	s = taddr;
	p = b;
	while(i--)
		s =+ *p++;
	*p++ = taddr;
	*p = 031415 - s;
	bwrite(b);
}

bwrite(b)
{
	register int *lbp, *bp;
	register int i;

	if(taddr == 0) {
		if(fo != -1) {
			printf("change tapes\n");
			close(fo);
			rdline();
		}
		otape();
	}
	if(lbpsv) {
		lbp = lbpsv;	/* copy to large tape buffer */
		bp = b;
		for(i=0; i<256; i++)
			*lbp++ = *bp++;
		if(lbp == &lbuf[LBUFZ]) {
			lwrite();
			lbp = &lbuf[0];
		}
		lbpsv = lbp;
	}
	else {
		wloop:
			if(write(fo, b, 512) != 512) {
				printf("write error\n");
				rdline();
				seek(fo, taddr, 3);
				goto wloop;
			}
	}
	if (vflg) {
		seek (fo, -1, 4);
		verify(b);
	}
	taddr++;
	if(taddr >= tsize)
		taddr = 0;
}

lwrite()
{
	if(write(fo, lbpaddr, LBUFZ*2) != LBUFZ*2) {
		printf("write error\n");
		lbpsv = 0; /* so exit doesnt try to flush */
		exitt(1);
	}
}
rdline()
{
	int c;

	flush();
loop:
	c = 0;
	read(0, &c, 1);
	if(c == 0)
		exitt(1);
	if(c != '\n')
		goto loop;
}

number(s)
char *s;
{
	register n, c;

	n = 0;
	while(c = *s++) {
		if(c<'0' || c>'9')
			continue;
		n = n*10+c-'0';
	}
	return(n);
}

size(s0, s1)
{
	register s;
	extern ldivr;

	s = ldiv(s0&0377, s1, 512);
	if(ldivr)
		s++;
	return(s);
}

otape()
{
	register char *p;

	if(oflg) 
		fo = 1; /* standard output */
	else {
		fo = creat(ofile, 066);
		close (fo);
		fo = open(ofile, 2);
	}
	if(fo < 0) {
		printf("can not open %s\n", ofile);
		exitt(1);
	}
	if(!cflg)
		return;
	p = ofile;
	while(*p++)
		;
	p[-2]++;
}

equal(a, b)
char *a, *b;
{

	while(*a++ == *b)
		if(*b++ == 0)
			return(1);
	return(0);
}

verify(a)
int	*a;
{
	int register  *apntr, *bpntr, sum;
	int static  datrec, sblk, b[256];
	int i;
	struct inode  *ipntr1, *ipntr2;
	if (read(fo, b, 512) != 512) {
		printf ("verify read error\n");
		exitt(1);
	}
	if (sblk == 0) {
		sblk = 1;
		for (sum=0, bpntr=(&b[0]); bpntr<(&b[256]);)
			sum =+ (*bpntr++);
		if (!equal(&b[7], "s-block for ") || sum != 031415) {
				printf ("dump tape s-block does not verify\n");
				exitt(1);
		}
	} else {
		if (equal(a+16, "file header")) {
			datrec = 0;
			for (sum=0, bpntr=(&b[0]); bpntr<(&b[256]); )
				sum =+ *bpntr++;
			for (ipntr1=a, ipntr2=(&b[0]), i=0; i<=7; i++)
				ipntr2->i_addr[i] = ipntr1->i_addr[i];
				ipntr2->i_atime[0] = ipntr1->i_atime[0];
				ipntr2->i_atime[1] = ipntr1->i_atime[1];
				ipntr2->i_mtime[0] = ipntr1->i_mtime[0];
				ipntr2->i_mtime[1] = ipntr1->i_mtime[1];
			for (ipntr1=a+23, ipntr2=(&b[23]), i=0; i<=7; i++)
				ipntr2->i_addr[i] = ipntr1->i_addr[i];
				ipntr2->i_atime[0] = ipntr1->i_atime[0];
				ipntr2->i_atime[1] = ipntr1->i_atime[1];
				ipntr2->i_mtime[0] = ipntr1->i_mtime[0];
				ipntr2->i_mtime[1] = ipntr1->i_mtime[1];
			for (ipntr1=a+39, ipntr2=(&b[39]), i=0; i<=7; i++)
				ipntr2->i_addr[i] = ipntr1->i_addr[i];
				ipntr2->i_atime[0] = ipntr1->i_atime[0];
				ipntr2->i_atime[1] = ipntr1->i_atime[1];
				ipntr2->i_mtime[0] = ipntr1->i_mtime[0];
				ipntr2->i_mtime[1] = ipntr1->i_mtime[1];
			for (apntr=a, bpntr=(&b[0]); bpntr<(&b[255]); )
				if ( (*apntr++) != (*bpntr++) ) {
					printf ("file header record for file %d does not verify\n", filenum);
					exitt (1);
				}
				if (sum != 031415) {
					printf ("file header record for file %d's checksum does not verify\n", filenum);
					exitt (1);
				}
		} else {
			datrec++;
			for (apntr=a, bpntr=(&b[0]); bpntr<(&b[256]);)
				if ( (*apntr++) != (*bpntr++) ) {
					printf ("data record %d in file %d does not verify\n",
						datrec, filenum);
					exitt (1);
				}
		}
	}
}
exitt(arg)
{
	flush();
	if(lbpsv) 
		lwrite();
	exit(arg);
}
