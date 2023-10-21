#
/*
 *      ncheck  - modified to add new options as follows
 *              - v give full details about each inode such as
 *                  given in the ls command
 *              - u report all inodes owned by the given uid
 *              - g as was for users so be it for groups
 *
 */
char	*dargv[]
{
	/* Defaults for the UoMd C.V.L. PDP-11/45 */
	"/dev/rk0",
	"/dev/rk1",
	0
};

#define NINODE  16*16
#include "/usr/sys/h/ino.h"
#include "/usr/sys/h/filsys.h"
#define DIR     040000
#define	CHR	020000
#define BLK     060000
#define	LARGE	010000
#define	SUID	04000
#define	SGID	02000
#define STXT    01000
#define	ROWN	0400
#define	WOWN	0200
#define	XOWN	0100
#define	RGRP	040
#define	WGRP	020
#define	XGRP	010
#define	ROTH	04
#define	WOTH	02
#define	XOTH	01
#define	RSTXT	01000

struct	filsys	sblock;
struct	inode	inode[NINODE];
struct inode *iip;
int gid, uid, mode;

int     vflg;   /* verbose flag: give more details on each inode */
int     gflg;   /* we are looking for a specific group id */
int     uflg;   /* we are looking for a specific user id */
int	aflg;
#define	NI	20
#define	NDIRS	787
int	uidfil	-1;
struct {
	int	fdes;
	int	nleft;
	char	*nextc;
	char	buff[512];
} 
inf;

int	ilist[NI] { 
	-1};
int	fi;
struct	htab {
	int	hino;
	int	hpino;
	char	hname[14];
} 
htab[NDIRS];
int	nhent	10;
int	(*pass[])()	{ 
	pass1, pass2, pass3 };
char	*lasts;
int	ino;
int	nerror;
int	nffil;
int	fout;
int	nfiles;
struct dir {
	int	ino;
	char	name[14];
};
int lmtime[2];
int year;

main(argc, argv)
char **argv;
{
	register char **p;
	register int n, *lp;

	time(lmtime);
	year = lmtime[0] - 245; /* 6 months ago */
	nffil = dup(1);
	if (argc == 1) {
		for (p = dargv; *p;)
			check(*p++);
		return(nerror);
	}
	while (--argc) {
		argv++;
		if (**argv=='-') switch ((*argv)[1]) {
		case 'v':       /* verbose flag means tell all */
			vflg++;
			uidfil = open("/etc/passwd", 0);
			continue;

		case 'u':               /* report on all files with uid */
			aflg++;
			vflg++;
			uflg++;
			uidfil = open("/etc/passwd", 0);
			p = *argv  ; 
			p++;
			uid = atoi(p);
			continue;

		case 'g':               /* report on all files with gid */
			aflg++;
			vflg++;
			gflg++;
			uidfil = open("/etc/passwd", 0);
			p = *argv  ; 
			p++;
			gid = atoi(p);
			continue;

		case 'a':
			aflg++;
			continue;

		case 'i':
			lp = ilist;
			while (lp < &ilist[NI-1] && (n = number(argv[1]))) {
				*lp++ = n;
				argv++;
				argc--;
			}
			*lp++ = -1;
			continue;

		default:
			printf2("Bad flag\n");
		}
		if (argc == 0) {
			for (p = dargv; *p;)
				check(*p++);
			return(nerror);
		}
		check(*argv);
	}
	return(nerror);
}

check(file)
char *file;
{
	register i, j, pno;

	fi = open(file, 0);
	if (fi < 0) {
		printf2("cannot open %s\n", file);
		return;
	}
	printf2("%s:\n", file);
	sync();
	bread(1, &sblock, 512);
	nfiles = sblock.s_isize*16;
	for (i=0; i<NDIRS; i++)
		htab[i].hino = 0;
	fout = nffil;
	flush();
	for (pno=0; pno<3; pno++) {
		ino = 0;
		for (i=0; ino<nfiles; i =+ NINODE/16) {
			bread(i+2, inode, sizeof inode);
			for (j=0; j<NINODE && ino<nfiles; j++) {
				ino++;
				(*pass[pno])(&inode[j]);
			}
		}
	}
	flush();
	fout = 1;
}

pass1(ip)
{
	if ((ip->i_mode&IALLOC)==0 || (ip->i_mode&IFMT)!=IFDIR)
		return;
	lookup(ino, 1);
}

pass2(ip)
struct inode *ip;
{
	register doff;
	register struct htab *hp;
	register struct dir *dp;
	int i;

	if ((ip->i_mode&IALLOC)==0 || (ip->i_mode&IFMT)!=IFDIR)
		return;
	doff = 0;
	while (dp = dread(ip, doff)) {
		doff =+ 16;
		if (dp->ino==0)
			continue;
		if ((hp = lookup(dp->ino, 0)) == 0)
			continue;
		if (dotname(dp))
			continue;
		hp->hpino = ino;
		for (i=0; i<14; i++)
			hp->hname[i] = dp->name[i];
	}
}

pass3(ip)
struct inode *ip;
{
	register doff;
	register struct dir *dp;
	register int *ilp;
	static char buf[512];

	if ((ip->i_mode&IALLOC)==0 || (ip->i_mode&IFMT)!=IFDIR)
		return;
	doff = 0;
	while (dp = dread(ip, doff)) {
		doff =+ 16;
		if (dp->ino==0)
			continue;
		if (aflg==0 && dotname(dp))
			continue;
		for (ilp=ilist; *ilp >= 0; ilp++)
			if (*ilp == dp->ino)
				break;
		if (ilp > ilist && *ilp!=dp->ino)
			continue;
		bread( (dp->ino+31)/16,&buf,512);
		iip = &buf[32*((dp->ino+31)%16)];
		/* iip now points to the inode we are talking about */
		if ( uflg &&  uid != iip->i_uid) continue;
		if ( gflg &&  gid != iip->i_gid) continue;
		printf("%4d\t", dp->ino);
		if (vflg )   pentry() ;
		pname(ino, 0);
		printf("/%.14s\n", dp->name);
	}
}

dotname(adp)
{
	register struct dir *dp;

	dp = adp;
	if (dp->name[0]=='.')
		if (dp->name[1]==0 || dp->name[1]=='.' && dp->name[2]==0)
			return(1);
	return(0);
}

pname(i, lev)
{
	register struct htab *hp;

	if (i==1)
		return;
	if ((hp = lookup(i, 0)) == 0) {
		printf("???");
		return;
	}
	if (lev > 10) {
		printf("...");
		return;
	}
	pname(hp->hpino, ++lev);
	printf("/%.14s", hp->hname);
}

lookup(i, ef)
{
	register struct htab *hp;

	for (hp = &htab[i%NDIRS]; hp->hino;) {
		if (hp->hino==i)
			return(hp);
		if (++hp >= &htab[NDIRS])
			hp = htab;
	}
	if (ef==0)
		return(0);
	if (++nhent >= NDIRS) {
		printf2("Out of core-- increase NDIRS\n");
		flush();
		exit(1);
	}
	hp->hino = i;
	return(hp);
}

dread(aip, aoff)
{
	register b, off;
	register struct inode *ip;
	static ibuf[256];
	static char buf[512];

	off = aoff;
	ip = aip;
	if ((off&0777)==0) {
		if (off==0177000) {
			printf2("Monstrous directory %l\n", ino);
			return(0);
		}
		if ((ip->i_mode&ILARG)==0) {
			if (off>=010000 || (b = ip->i_addr[off>>9])==0)
				return(0);
			bread(b, buf, 512);
		} 
		else {
			if (off==0) {
				if (ip->i_addr[0]==0)
					return(0);
				bread(ip->i_addr[0], ibuf, 512);
			}
			if ((b = ibuf[(off>>9)&0177])==0)
				return(0);
			bread(b, buf, 512);
		}
	}
	return(&buf[off&0777]);
}

bread(bno, buf, cnt)
{

	seek(fi, bno, 3);
	if (read(fi, buf, cnt) != cnt) {
		printf2("read error %d\n", bno);
		exit();
	}
}

bwrite(bno, buf)
{

	seek(fi, bno, 3);
	if (write(fi, buf, 512) != 512) {
		printf2("write error %d\n", bno);
		exit();
	}
}

number(as)
char *as;
{
	register n, c;
	register char *s;

	s = as;
	n = 0;
	while ((c = *s++) >= '0' && c <= '9') {
		n = n*10+c-'0';
	}
	return(n);
}

printf2(s, a1, a2)
{
	extern fout;

	flush();
	fout = 2;
	printf(s, a1, a2);
	fout = nffil;
	flush();
}

pentry()
{
	char tbuf[16];
	char *cp;
	pmode(iip->i_mode);
	printf("%2d ",iip->i_nlink); /* number of links */
	if (getname(iip->i_uid&0377, tbuf)==0)
		printf("%-6.6s", tbuf);
	else
	    printf("%-6d",iip->i_uid&0377);
	if ((((iip->i_mode) &  IFMT ) == IFBLK )
	    || (((iip->i_mode) &  IFMT ) == IFCHR) )
		printf("%3d,%3d",((iip->i_addr[0])>>8)&0377,
		(iip->i_addr[0])&0377);
	else
		printf("%7s", locv((iip->i_size0),(iip->i_size1)));
	cp = ctime(iip->i_mtime);
	if(iip->i_mtime[0] < year)
		printf(" %-7.7s %-4.4s ", cp+4, cp+20); 
	else
	    printf(" %-12.12s ", cp+4);
}

getname(usrid, buf)
int usrid;
char buf[];
{
	int j, c, n, i;

	inf.fdes = uidfil;
	seek(inf.fdes, 0, 0);
	inf.nleft = 0;
	do {
		i = 0;
		j = 0;
		n = 0;
		while((c=getc(&inf)) != '\n') {
			if (c<0)
				return(-1);
			if (c==':') {
				j++;
				c = '0';
			}
			if (j==0)
				buf[i++] = c;
			if (j==2)
				n = n*10 + c - '0';
		}
	} 
	while (n != usrid);
	buf[i++] = '\0';
	return(0);
}

int     m0[] { 
	1, ROWN, 'r', '-' };
int     m1[] { 
	1, WOWN, 'w', '-' };
int     m2[] { 
	2, SUID, 's', XOWN, 'x', '-' };
int     m3[] { 
	1, RGRP, 'r', '-' };
int     m4[] { 
	1, WGRP, 'w', '-' };
int     m5[] { 
	2, SGID, 's', XGRP, 'x', '-' };
int     m6[] { 
	1, ROTH, 'r', '-' };
int     m7[] { 
	1, WOTH, 'w', '-' };
int     m8[] { 
	1, XOTH, 'x', '-' };
int     m9[] { 
	1, STXT, 't', ' ' };

int     *m[] { 
	m0, m1, m2, m3, m4, m5, m6, m7, m8, m9};
int     flags;
pmode(aflag)
{
	register int **mp;

	flags = aflag;
	switch( flags&IFMT){
	case BLK: 
		putchar('b'); 
		break;
	case CHR: 
		putchar('c'); 
		break;
	case DIR: 
		putchar('d'); 
		break;
	default:  
		putchar('-'); 
	};

	for (mp = &m[0]; mp < &m[10];)
		select(*mp++);
}

select(pairp)
int *pairp;
{
	register int n, *ap;
	int c;

	ap = pairp;
	n = *ap++;
	while (--n>=0 && ((flags&(c = *ap++))!= c) )  ap++;
	putchar(*ap);
}
