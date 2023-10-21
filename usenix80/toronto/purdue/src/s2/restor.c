#

/*
 * restore from incremental dumps
 */

char	*dargv[]
{
	0,
	"t",
	0
};
char	*ifile;
char	*ofile;
#include "/usr/sys/ino.h"
#include "/usr/sys/filsys.h"

struct	filsys	sblock;
int	isize;
int	*talist;
int	fi;
int	buf[256];
int	dbuf[256];
int	cbuf[256];
/* LBUFZ must be a multiple of 256 */
#define	LBUFZ	12800
int	lbuf	[LBUFZ];	/* buffer for long tape records */
int	*lbpsv;			/* nz if doing long tape i/o */
char	*date[2];
char	*ddate[2];
int	fo;
int	pher;
char	*tsize	15000;
int	iflg;
int	wflg;
int	cflg;
int	oflg;	/* use standard input for dump tape */
char	file[10];
int	ilist[100];
int	filenum;
int	fin;	/* input fd for getchar() */

main(argc, argv)
char **argv;
{
	char *key;
	register *tap, *p;
	register struct inode *ip;
	int i, com, sz, *q, l;
	int  ierr1, ierr2;
	char  *dumpdev;

	ifile = "/dev/mt0";
	if(argc == 1) {
		argv = dargv;
		for(argc = 1; dargv[argc]; argc++);
	}

	argc--;
	argv++;
	key = *argv;
	while(*key)
	switch(*key++) {
	default:
		printf("bad character in key\n");
		exit();

	case 't':
	case 'r':
	case 'x':
		com = key[-1];
		continue;

	case 'i':
		iflg++;
		continue;

	case '-':
		continue;

	case 'c':
		cflg++;
		continue;

	case 'f':
		argv++;
		argc--;
		ifile = *argv;
		continue;

	case 'w':
		wflg++;
		continue;

	case 'o':	/* read dump file from standard input */
		if((fin=open("/dev/tty",0)) < 0) {
			printf(" Can't open /dev/tty for input\n");
			exit();
		}
		oflg++;
		continue;

	case 'l':	/* long tape i/o */
		lbpsv = &lbuf[LBUFZ];  /* set buffer empty so it gets filled */
		iflg++;	/* i opt forced, no seeks on raw magtape */
		continue;

	}
	otape();
	if (sread(buf, 0)) {
		printf("unable to read s-block record\n");
		exit();
	}
	dumpdev = &buf[13];
	dumpdev =+ 1;
	tap = buf;
	isize = *tap++;
	*tap++;		/* fsize */
	date[0] = *tap++;
	date[1] = *tap++;
	ddate[0] = *tap++;
	ddate[1] = *tap++;
	tsize = *tap++;
	i = size(0, isize*32);
	talist = sbrk(i*512);
	tap = talist;
	while(i--) {
		if (tread(tap, 0)) {
			printf ("unable to read file size records\n");
			exit();
		}
		tap =+ 256;
	}
	switch(com) {

case 't':
	l = 0;
	com = 0;
	printf ("incremental dump tape for %s from ", dumpdev);
	pdate(ddate);
	printf ("dumped on ");
	pdate(date);
	printf ("the following files(by number) are on this tape\n");
	tap = talist;
	for(i=0; i<isize*16; i++) {
		sz = *tap++;
		if(sz == 0 || sz == -1) {
			if(com == 0)
				continue;
			if(i == com) {
				printf("%l", i);
				l =+ 5;
			} else {
				printf("%l-%l", com, i);
				l =+ 10;
			}
			if(l > 60) {
				printf("\n");
				l = 0;
			} else
				printf(",");
			com = 0;
		} else
		if(com == 0)
			com = i+1;
	}
	if(com)
		printf("%l-\n", com);
	printf("\n");
	exit();

case 'r':
	if(argc <= 1) {
		printf("no filesystem name\n");
		exit();
	}
	ofile = argv[1];
	fo = open(ofile, 2);
	if(fo < 0) {
		printf("can not open %s\n", ofile);
		exit();
	}
	printf("last chance before scribbling on %s\n", ofile);
	getchar();
	dread(1, &sblock);
	tap = talist;
	filenum = 0;
	for(i=0; i<sblock.s_isize; i++) {
		if(i >= isize)
			break;
		dread(i+2, buf);
		for(ip = &buf[0]; ip < &buf[256]; ip++) {
			filenum++;
			sz = *tap++;
			if(sz == 0)
				continue;
			dealoc(ip);
			if(sz == -1) {
				for(p = ip; p < &ip->i_mtime[2]; )
					*p++ = 0;
				continue;
			}
			while (sread(dbuf, 0) || (!equal(&dbuf[16], "file header")));
			if (filenum != dbuf[22]) {
				printf ("file %d lost\n", filenum);
				for (p=ip; p<(&ip->i_mtime[2]); )
					*p++ = 0;
				seek (fi, -1, 4);
				continue;
			}
			q = dbuf;
			for(p = ip; p < &ip->i_mtime[2]; )
				*p++ = *q++;
			if (restor(ip, sz-1)) {
				printf ("parts of file %d were lost\n", filenum);
				printf ("those parts were replaced by zeroes\n");
			}
		}
		dwrite(i+2, buf);
	}
	dwrite(1, &sblock);
	com = 0;
	for(; i < isize; i++)
		for(l = 0; l < 16; l++) {
			sz = *tap++;
			if(sz != 0 && sz != -1)
				com++;
		}
	if(com)
		printf("%l files not restored - small ilist\n", com);
	exit();

case 'x':
	i = 0;
	tap = ilist;
	while(argc > 1) {
		i++;
		sz = number(argv[1]);
		argv++;
		argc--;
		if(sz <= 0 || sz >=isize*16) {
			printf("%l not in range\n", sz);
			continue;
		}
		if(talist[sz-1] == 0) {
			printf("%l not dumped\n", sz);
			continue;
		}
		if(talist[sz-1] == -1) {
			printf("%l does not exist\n", sz);
			continue;
		}
		*tap++ = sz;
	}
	if(i != 0 && ilist[0] == 0)
		exit();
	tap = talist;
	for(i=1; i<=isize*16; i++) {
		if(ilist[0] != 0) {
			for(sz=0; ilist[sz]; sz++)
				if(ilist[sz] == i)
					goto yes;
			sz = *tap++;
		no:
			if(sz == -1)
				sz = 0;
			while(sz--)
				tread(dbuf, 1);
			continue;
		}
	yes:
		sz = *tap++;
		if(sz == 0 || sz == -1)
			continue;
		fo = dwait(i);
		if(fo < 0)
			goto no;
		sz--;
		filenum = i;
		if (sread(buf, 0)) {
			printf ("bad file header block for file %d -- job terminates\n",
				filenum);
			unlink(fo);
			exit();
		}
		ip = buf;
		ierr1 = 0;
		while(sz--) {
			ierr2 = tread(dbuf, 0);
			com = 512;
			if(ip->i_size0 == 0 && ip->i_size1 < 512)
				com = ip->i_size1;
			if (!ierr2)
				write(fo, dbuf, com);
			if(com > ip->i_size1)
				ip->i_size0--;
			ip->i_size1 =- com;
			ierr1 = ierr1 || ierr2;
		}
		close(fo);
		chmod(file, ip->i_mode);
		chown(file, ((ip->i_uid0)&0377 | ip->i_uid1<<8));
		if (ierr1) {
			printf ("parts of file %d were lost\n", filenum);
			printf ("those parts were replaced by zeroes\n");
		}
	}
	exit();

	}
}

dealoc(p)
struct inode *p;
{
	register struct inode *ip;
	register i, j;
	int k;
	int xbuf[256], ybuf[256];

	ip = p;
	if(ip->i_mode & (IFCHR&IFBLK))
		return;
	for(i=7; i>=0; i--)
	if(ip->i_addr[i]) {
		if(ip->i_mode&ILARG) {
			dread(ip->i_addr[i], xbuf);
			for(j=255; j>=0; j--)
			if(xbuf[j]) {
				if(i == 7) {
					dread(xbuf[j], ybuf);
					for(k=255; k>=0; k--)
					if(ybuf[k])
						free(ybuf[k]);
				}
				free(xbuf[j]);
			}
		}
		free(ip->i_addr[i]);
	}
}

restor(p, sz)
struct inode *p;
{
	register struct inode *ip;
	register i, j;
	int xbuf[256];
	int errflg;

	errflg = 0;
	ip = p;
	if(ip->i_mode & (IFCHR&IFBLK))
		return(errflg);
	for(i=0; i<8; i++)
		ip->i_addr[i] = 0;
	if(sz <= 8) {
		for(i=0; i<sz; i++)
		if (rcop(&ip->i_addr[i]))
			errflg = 1;
		ip->i_mode =& ~ILARG;
		return(errflg);
	}
	for(i=0; i<256; i++)
		xbuf[i] = 0;
	for(j=0; sz >= 256; j++) {
		if(j <= 7)
			ip->i_addr[j] = alloc();
		if(j >= 7)
			xbuf[j-7] = alloc();
		for(i=0; i<256; i++)
			if (rcop(&dbuf[i]))
				errflg = 1;
		if(j < 7)
			dwrite(ip->i_addr[j], dbuf); else
			dwrite(xbuf[j-7], dbuf);
		sz =- 256;
	}
	if(sz) {
		if(j <= 7)
			ip->i_addr[j] = alloc();
		if(j >= 7)
			xbuf[j-7] = alloc();
		for(i=0; i<sz; i++)
			if (rcop(&dbuf[i]))
				errflg = 1;
		for(; i<256; i++)
			dbuf[i] = 0;
		if(j < 7)
			dwrite(ip->i_addr[j], dbuf); else
			dwrite(xbuf[j-7], dbuf);
	}
	if(j >= 7)
		dwrite(ip->i_addr[7], xbuf);
	ip->i_mode =| ILARG;
	return(errflg);
}

rcop(blocknum)
int *blocknum;
{
	register b;
	int  err;

	b = alloc();
	err = tread(cbuf, 0);
	dwrite(b, cbuf);
	*blocknum = b;
	return (err);
}

pdate(d)
int *d;
{

	if(d[0] == 0 && d[1] == 0)
		printf("the epoch\n"); else
		printf(ctime(d));
}

dread(bno, b)
{

	seek(fo, bno, 3);
	if(read(fo, b, 512) != 512) {
		printf("disk read error %l\n", bno);
		exit();
	}
}

dwrite(bno, b)
{

	seek(fo, bno, 3);
	if(write(fo, b, 512) != 512) {
		printf("disk write error %l\n", bno);
		exit();
	}
}

sread(b, flag)
int *b;
{
	register i, s, *p;
	static int  errflg;

	if (tread(b, flag)) {
		errflg = 1;
		return(1);
	}
	if(flag) {
		errflg = 0;
		return(0);
	}
	i = 256;
	s = 0;
	p = b;
	while(i--)
		s =+ *p++;
	if(s != 031415) {
		if (!errflg)
			printf("checksum error for file %d\n", filenum);
		errflg = 1;
		if(!iflg)
			return(1);
	}
	errflg = 0;
	return(0);
}

tread(b, flag)
int *b;
{
	register c;
	static char *pta, *ata, ctflg;
	static int  errcnt;
	int  rdcnt;

	if(pta++ >= tsize) {
		pta = 1;
		ata = 0;
		close(fi);
		otape();
		ctflg++;
	}
	if(flag)
		return(0);
	if(ctflg) {
		printf("change tapes\n");
		if(ctflg > 1)
			printf("skip %d tapes\n", ctflg-1);
		while((c = getchar()) != '\n')
			if(c == 0)
				exit();
		ctflg = 0;
	}
	ata++;
	if(iflg)
	for(; pta != ata; ata++)
		lread(fi, b, 512);
	if(pta != ata) {
		seek(fi, pta-ata, 4);
		ata = pta;
	}
	if( (rdcnt=lread(fi, b, 512)) != 512) {
		if (rdcnt == 0) {
			printf("end of tape while looking for file %d -- job terminates\n",
				filenum);
			exit();
		}
		printf("tape read error at tape address %l for file %d\n",
			ata-1, filenum);
		for(c=0; c<256; c++)
			b[c] = 0;
		if (!iflg) {
			if (++errcnt > 10) {
				printf ("too many tape read errors -- job terminates\n");
				exit();
			}
		return(1);
		}
	}
	return(0);
}

lread(fd, b, count)
int	*b;
{
	register int *bp, *lbp, i;
	if(count != 512) {
		printf("lread - count != 512 bytes\n");
		exit();
	}
	bp = b;
	if(lbp=lbpsv) {
		if(lbp == &lbuf[LBUFZ]) {  /* refill buffer */
			if(read(fd, &lbuf, LBUFZ*2) != LBUFZ*2) {
				printf("long tape read error\n");
				return(0);
			}
			lbp = &lbuf[0];
		}
		count = count >>1;
		for(i=0; i<count; i++)
			*bp++ = *lbp++;
		lbpsv = lbp;
		return(512);
	}
	return(read(fd, b, count));
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
		fi = 0;	/* use standard input for tape file */
	else {
		fi = open(ifile, 0);
		if(fi < 0) {
			printf("can not open %s\n", ifile);
			exit();
		}
	}
	if(!cflg)
		return;
	p = ifile;
	while(*p++)
		;
	p[-2]++;
}

dwait(ino)
{
	register i;

	dconv(ino, file);
loop:
	if(wflg) {
		printf("%s ", file);
		i = getchar();
		if(i == 'x')
			exit();
		if(i == '\n')
			return(-1);
		if(i != 'y')
			goto flush;
		i = getchar();
		if(i != '\n') {
		flush:
			while((i=getchar()) != '\n')
				if(i == '\0')
					exit();
			goto loop;
		}
	}
	i = creat(file, 0666);
	return(i);
}

dconv(n, p)
char *p;
{
	register i;

	if(i = ldiv(0, n, 10))
		p = dconv(i, p);
	*p++ = lrem(0, n, 10) + '0';
	*p = '\0';
	return(p);
}

alloc()
{
	register b, i;

	i = --sblock.s_nfree;
	if(i<0 || i>=100) {
		printf("bad freeblock\n");
		exit();
	}
	b = sblock.s_free[i];
	if(b == 0) {
		printf("out of freelist\n");
		exit();
	}
	if(sblock.s_nfree <= 0) {
		dread(b, cbuf);
		sblock.s_nfree = cbuf[0];
		for(i=0; i<100; i++)
			sblock.s_free[i] = cbuf[i+1];
	}
	return(b);
}

free(in)
{
	register i;

	if(sblock.s_nfree >= 100) {
		cbuf[0] = sblock.s_nfree;
		for(i=0; i<100; i++)
			cbuf[i+1] = sblock.s_free[i];
		sblock.s_nfree = 0;
		dwrite(in, cbuf);
	}
	sblock.s_free[sblock.s_nfree++] = in;
}
equal(a, b)
char *a, *b;
{
	char register  *ap, *bp;
	ap = a;
	bp = b;
	while (*ap++ == *bp)
		if (*bp++ == 0)
			return(1);
	return(0);
}
