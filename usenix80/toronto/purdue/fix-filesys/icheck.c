#
/*
 * icheck - file system consistency check
 *
 * modified to do repairs, etc. Mike Tilson, Feb. 1977
 * modified to look for mis-matches between inode size and actual size,
 *      Mike Tilson, Oct. 1977  (Toronto Distrib)
 *
 * Incorporated Purdue's icheck  modifications, G. Goble 04/13/79.
 * print out %'s, sort freelist, etc.
 * Don't do update if major problems found in filesys.
 *
 */

#define NEWFS   /* use inode and block free counts in superblock */

#define DISASTER 04
#define PROBLEM 02
#define MINORPROBLEM 01
#define ALLSWELL 0
char	*dargv[]
{
	"/dev/root",
	"/dev/sys",
	"/dev/u0",
	"/dev/u1",
	0
};
#define	NINODE	16*16
#define	NB	10
#include <ino.h>
#include <filsys.h>

struct	inode	inode[NINODE];
struct	filsys	sblock;

int	sflg;
int     hflg;   /* print "hole in file message" */
int     mflg;   /* print out bitmap of filesystem */
int	qflg;
int	rflg;
int	errors;
int	notserious;
char	*devname;
int	devpr;

int	fi;
#ifdef NEWFS
int     ninodes;
#endif
int	nifiles;
int	nfile;
int	nspcl;
int	nlarg;
int	nvlarg;
int	nindir;
int     nablks;
int	nvindir;
int	ndir;
int	nused;
int	nfree;
int	ino;
int	ndup;
int     blist[20] { -1};
int	nerror;
int	bmap[4096];

main(argc, argv)
char **argv;
{
	register char **p;
	register int n, *lp;

	if (argc == 1) {
		for (p = dargv; *p;)
			check(*p++);
		return(nerror);
	}
	while (--argc) {
		argv++;
		if (**argv=='-') switch ((*argv)[1]) {
		case 's':
			sflg++;
			continue;

		case 'q':
			qflg++;
			continue;

		case 'h':
			hflg++;
			continue;

		case 'm':
			mflg++;
			continue;

		case 'r':
			rflg++;
			continue;

		case 'b':
			lp = blist;
			while (lp < &blist[NB-1] && (n = number(argv[1]))) {
				*lp++ = n;
				argv++;
				argc--;
			}
			*lp++ = -1;
			continue;

		default:
			printf("Bad flag\n");
			nerror =| DISASTER;
		}
		check(*argv);
	}
	return(nerror);
}

check(file)
char *file;
{
	register *ip, i, j;
	float pci(), pcb();
	int k;

	fi = open(file, (sflg||rflg)?2:0);
	if (fi < 0) {
		printf("cannot open %s\n", file);
		nerror =| DISASTER;
		return;
	}
	if(!qflg) {
		devpr++;
		printf("%s:\n", file);
	}
	else {
		devname = file;
		devpr = 0;
	}
	nfile = 0;
#ifdef  NEWFS
	ninodes = 0;
#endif
	nspcl = 0;
	nlarg = 0;
	nvlarg = 0;
	nindir = 0;
	nvindir = 0;
	ndir = 0;
	nused = 0;
	nfree = 0;
	ndup = 0;
	errors = 0;
	notserious = 0;
	for (ip = bmap; ip < &bmap[4096];)
		*ip++ = 0;
	sync();
	bread(1, &sblock, 512);
	nifiles = sblock.s_isize*16;
	nablks = sblock.s_fsize - 2 - sblock.s_isize;
	for(i=0; ino < nifiles; i =+ NINODE/16) {
		bread(i+2, inode, sizeof inode);
		for(j=0; j<NINODE && ino<nifiles; j++) {
			ino++;
			pass1(&inode[j]);
		}
	}
	if(mflg) {
		printf("	BITMAP FOR %s\n\n", file);
		for(k=0; k< 4096; k =+ 16) {
			printf(" %6l  ", k);
			for(i=k; i<k+16; i++)
				printf(" %6o", bmap[i]);
			printf("\n");
		}
	}
	if(ndup) {
		errors++;
		nerror =| PROBLEM;
	}
	if (errors != notserious  && (rflg || sflg)) {
		nerror =| PROBLEM;
		error("No update.\n");
		exit(nerror);
	}
	ndup = 0;
	ino = 0;
	sync();
	bread(1, &sblock, 512);
	if (sflg) {
		makefree(file);
		return;
	}
	while(i = alloc()) {
		if (chk(i, "free"))
			break;
		nfree++;
	}
	if (ndup) {
		error("%l dups in free\n", ndup);
		nerror =| rflg?MINORPROBLEM:PROBLEM;
		if(rflg)
			notserious++;
	}
	j = 0;
	for (ip = bmap; ip < &bmap[4096];) {
		i = *ip++;
		while (i) {
			if (i<0)
				j--;
			i =<< 1;
		}
	}
	j =+ sblock.s_fsize - sblock.s_isize - 2;
	if (j) {
		error("missing%5l\n", j);
		notserious++;
	}
	if(!qflg || errors!=0) {
	if(nspcl)
		printf("spcl  %6l%8.1f%%\n", nspcl,pci(nspcl));
	printf("files %6l%8.1f%%\n", nfile,pci(nfile));
	printf("large %6l%8.1f%%\n", nlarg,pci(nlarg));
	if (nvlarg)
		printf("huge  %6l%8.1f%%\n", nvlarg,pci(nvlarg));
	printf("direc %6l%8.1f%%\n", ndir,pci(ndir));
	printf("fsize %6l\n",nablks);
	printf("indir %6l%8.1f%%\n", nindir,pcb(nindir));
	if (nvindir)
		printf("indir2%6l%8.1f%%\n", nvindir,pcb(nvindir));
	printf("used  %6l%8.1f%%\n", nused,pcb(nused));
	printf("free  %6l%8.1f%%\n", nfree,pcb(nfree));
	}
	if(errors) {
		nerror =| (errors==notserious?MINORPROBLEM:PROBLEM);
		if(rflg && errors==notserious) {
			sflg++;
			printf("rebuilding free list: ");
			check(file);
			if(sflg)
				sflg--;
			printf("free list rebuilt\n");
		}
	}
	close(fi);
}

pass1(aip)
struct inode *aip;
{
	int buf[256], vbuf[256];
	register i, j, *ip;
	long int maxblk;	/* mdt oct/77 */
	long int totblk;	/* mdt oct/77 */
	long int filesize;	/* mdt oct/77 */
	struct {		/* mdt oct/77 */
		char s0;
		char xx;
		int s1;
	};

	ip = aip;
	if ((ip->i_mode&IALLOC) == 0) {
		if (ip->i_mode != 0)
			error("Inode %u trashed up: 0%o\n", ino, ip->i_mode);
		return;
	}
#ifdef NEWFS
	ninodes++;
#endif
	if ((ip->i_mode&IFCHR&IFBLK) != 0) {
		nspcl++;
		if(ip->i_size0 || ip->i_size1)
			error("inode=%l: dev size = %s\n", ino,
				locv(ip->i_size0&0377, ip->i_size1));
		for(i=1; i<7; i++)
			if(ip->i_addr[i] != 0)
				error("inode=%l: i_addr[%d] = %l\n",
					ino, i, ip->i_addr[i]);
		return;
	}
	if ((ip->i_mode&IFMT) == IFDIR)
		ndir++;
	else
		nfile++;
	/* start mdt Oct/77 */
	maxblk = 0;
	totblk = 0;
	filesize.s0 = ip->i_size0;
	filesize.xx = 0;
	filesize.s1 = ip->i_size1;
	filesize =+ 511;
	filesize =>> 9;
	/* end mdt Oct/77 */
	if ((ip->i_mode&ILARG) != 0) {
		nlarg++;
		for(i=0; i<7; i++)
		if (ip->i_addr[i] != 0) {
			nindir++;
			if (chk(ip->i_addr[i], "indirect"))
				continue;
			bread(ip->i_addr[i], buf, 512);
			for(j=0; j<256; j++)
			if (buf[j] != 0) {
				chk(buf[j], "data (large)");
				totblk++;		/* mdt oct/77 */
				maxblk = i*256 + j + 1;	/* mdt oct/77 */
			}
		}
		if (ip->i_addr[7]) {
			nvlarg++;
			if (chk(ip->i_addr[7], "indirect"))
				return;
			bread(ip->i_addr[7], buf, 512);
			for(i=0; i<256; i++)
			if (buf[i] != 0) {
				nvindir++;
				if (chk(buf[i], "2nd indirect"))
					continue;
				bread(buf[i], vbuf, 512);
				for(j=0; j<256; j++)
				if (vbuf[j]) {
					chk(vbuf[j], "data (very large)");
					maxblk = 7*256 + i*256 + j + 1;	/* mdt oct/77 */
					totblk++;			/* mdt oct/77 */
				}
			}
		}
	}
	else {
		for(i=0; i<8; i++) {
			if (ip->i_addr[i] != 0) {
				chk(ip->i_addr[i], "data (small)");
				maxblk = i+1;	/* mdt oct/77 */
				totblk++;	/* mdt oct/77 */
			}
		}
	}
	/* mdt oct/77 */
	if(maxblk!=filesize) {
		error("size mismatch; inode=%u (%ld %ld %ld)\n", ino, maxblk, totblk, filesize);
		errors--; /* changed from notserious++ 5/25/79 --ghg */
	}
	else if(totblk!=filesize && hflg) {
		error("skipped blocks (hole in file); inode=%u (%ld %ld %ld)\n", ino, maxblk, totblk, filesize);
		errors--;
	}
}

chk(ab, s)
char *ab;
{
	register char *b;
	register n, m;

	b = ab;
	if (ino)
		nused++;
	if (b<sblock.s_isize+2 || b>=sblock.s_fsize) {
		error("%l bad; inode=%l, class=%s\n", b, ino, s);
		if (ino == 0) {  /* bad block in freelist can be fixed */
			nerror =| rflg?MINORPROBLEM:PROBLEM;
			if(rflg)
				notserious++;
		}
		return(1);
	}
	m = 1 << (b&017);
	n = (b>>4) & 07777;
	if (bmap[n]&m) {
		error("%l dup; inode=%l, class=%s\n", b, ino, s);
		ndup++;
		errors--;	/* will use ndup as flag for this error */
	}
	bmap[n] =| m;
	for (n=0; blist[n] != -1; n++)
		if (b == blist[n]) {
			error("%l arg; inode=%l, class=%s\n", b, ino, s);
			errors--; /* not really an error */
		}
	return(0);
}

alloc()
{
	register b, i;
	int buf[256];

	i = --sblock.s_nfree;
	if (i<0 || i>=100) {
		error("bad freeblock\n");
		if(rflg)
			notserious++;
		return(0);
	}
	b = sblock.s_free[i];
	if (b == 0)
		return(0);
	if (sblock.s_nfree <= 0) {
		bread(b, buf, 512);
		sblock.s_nfree = buf[0];
		for(i=0; i<100; i++)
			sblock.s_free[i] = buf[i+1];
	}
	return(b);
}

bread(bno, buf, cnt)
int *buf;
{
	register *ip;

	seek(fi, bno, 3);
	if (read(fi, buf, cnt) != cnt) {
		error("read error %d\n", bno);
		nerror =| DISASTER;
		if (sflg || rflg) {
			error("No update\n");
			exit(DISASTER);
		}
		for (ip = buf; ip < &buf[256];)
			*ip++ = 0;
	}
}

free(in)
{
	register i;
	int buf[256];

#ifdef NEWFS
	nfree++;
#endif
	if (sblock.s_nfree >= 100) {
		buf[0] = sblock.s_nfree;
		for(i=0; i<100; i++)
			buf[i+1] = sblock.s_free[i];
		sblock.s_nfree = 0;
		bwrite(in, buf);
	}
	sblock.s_free[sblock.s_nfree++] = in;
}

bwrite(bno, buf)
{

	seek(fi, bno, 3);
	if (write(fi, buf, 512) != 512) {
		error("write error %d\n", bno);
		nerror =| DISASTER;
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

freebl(i)
int i;
{
	if((bmap[(i>>4)&07777] & (1<<(i&017)))==0)
		free(i);
}

makefree(file)
char *file;
{
	register char *i, *j;
	char *n, *m;
	char *high, *low;
	static char adr[100], flag[100];

	for( j = file; j[0]; j++) {
		if( j[0] == 'r')
			switch(j[1]) {
			case 'k':	/* rk05 */
				n = 24; /* sectors/track */
				m = 3;  /* interleave */
				break;
			case 'p':	/* rp03 */
				n = 10;
				m = 4;
				break;
			case 'f':	/* rf */
				n = 8;
				m = 3;
				break;
			default: ;
			}
		if (j[0] == 's')
			switch(j[1]) {
			case 'j':
			case 'i':       /* si 9400 */
			n = 33;         /* change to 32 if you have si 9500 */
			m = 15;
			break;
			}
		if( j[0] == 'h')
			switch(j[1]) {
			case 'p':	/* rp04 (/dev/hp?) */
				n = 22;
				m = 3;
				break;
			case 's':	/* rs04 (/dev/hs?) */
				n = 32;
				m = 3;
				break;
			default: ;
			}
	}
	printf("Free list = %l/%l\n",n,m);
	if(n > 100) n = 100;
	for( i = 0; i<n; i++)
		flag[i] = 0;
	j = 0;
	for( i = 0; i < n; i++) {
		while( flag[j] )
			j = (j+1)%n;
		adr[i] = j;
		flag[j]++;
		j = (j+m)%n;
	}
	sblock.s_nfree = 0;
	sblock.s_ninode = 0;
	sblock.s_flock = 0;
	sblock.s_ilock = 0;
	sblock.s_fmod = 0;

	high = sblock.s_fsize - 1;
	low = sblock.s_isize + 2;
	free(0);
	for( i = high; lrem(0,i+1,n); i--) {
		if( i < low )
			break;
		freebl(i);
	}
	for(; i >= low + n-1; i =- n)
		for( j = 0; j < n; j++)
			freebl(i-adr[j]);
	for(; i >= low; i--)
		freebl(i);
#ifdef NEWFS
	nfree--;
	sblock.s_tinode = nifiles - ninodes;
	sblock.s_tfree = nfree;
#endif
	bwrite(1, &sblock);
	close(fi);
	sync();
	return;
}

size(aip)
struct inode *aip;
{
	register struct inode *ip;
	register s;
	extern ldivr;

	ip = aip;
	s = ldiv(ip->i_size0 & 0377, ip->i_size1, 512);
	if(ldivr)
		s++;
	return(s);
}

error(a,b,c,d,e,f,g,h) {
	if(!devpr) {
		devpr++;
		printf("%s:\n", devname);
	}
	printf(a,b,c,d,e,f,g,h);
	errors++;
}

float flt(n)
{
	float f;

	f = n;
	if(f < 0)
		f =+ 65536.0;
	return(f);
}

float pci(n)
{
	return((flt(n)*100.0)/flt(nifiles));
}

float pcb(n)
{
	return((flt(n)*100.0)/flt(nablks));
}

