#
/* check structure of contiguous file system.
   should be run after icheck.
   about half of the code is the same as icheck.
   author:
	Mitchell Gart
	Ampex Corporation
	June, 1979
 */

char	*dargv[]
{
	"/dev/rhp0",
	"/dev/rhp1",
	"/dev/rhp2",
	"/dev/rhp6",
	0
};

#define NULL 0
#define	NINODE	16*16
#define	NB	10
#define min(a,b) (a<b? a:b)
#include "/usr/sys/ino.h"
#include "/usr/sys/filsys.h"
#include "/usr/sys/cf.h"

struct	inode	inode[NINODE];
struct	filsys	sblock;
struct	inode	inodefl;

int	sflg;

int	fi;
int	nifiles;
int	nfree;
int	nmaxfile;
int	nmaxfree;
int	ncontig;
int	ino;
int	ndup;
int	clist[10] {-1};
int	nerror;
int	cmap[4096];
int	flsiz;

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

		case 'c':
			lp = clist;
			while (lp < &clist[NB-1] && (n = number(argv[1]))) {
				*lp++ = n;
				argv++;
				argc--;
			}
			*lp++ = -1;
			continue;

		default:
			printf("Bad flag\n");
		}
		check(*argv);
	}
	return(nerror);
}

check(file)
char *file;
{
	register *ip, i, j;
	int oldsiz, newsiz;

	fi = open(file, sflg?2:0);
	if (fi < 0) {
		printf("cannot open %s\n", file);
		nerror =| 04;
		return;
	}
	printf("%s:\n", file);
	nfree = 0;
	ndup = 0;
	nmaxfile = 0;
	nmaxfree = 0;
	ncontig = 0;
	for (ip = cmap; ip < &cmap[4096];)
		*ip++ = 0;
	sync();
	bread(1, &sblock, 512);
	if(!getfl())
		return;
	nifiles = sblock.s_isize*16;
	for(i=0; ino < nifiles; i =+ NINODE/16) {
		bread(i+2, inode, sizeof inode);
		for(j=0; j<NINODE && ino<nifiles; j++) {
			ino++;
			if(ino == sblock.s_infl) {
				register *ip2;
				ip2 = &inodefl;
				for(ip = &inode[j]; ip < &inode[j+1]; )
					*ip2++ = *ip++;
			}
			pass1(&inode[j]);
		}
	}
	sync();
	bread(1, &sblock, 512);
	ino = sblock.s_infl;
		/* number of blocks in current free list */
	flsiz = (inodefl.i_size0 << 7) | ((inodefl.i_size1 >> 9) & 0177);
	if (sflg) {
		nfree = countfree();
		oldsiz = sblock.s_flsiz;
		sblock.s_flsiz = 0;
		sblock.s_rootlbn = 0;
		makefree(nfree, 0);
		bwrite(1, &sblock);
		newsiz = sblock.s_flsiz;
		for(ip = &sblock, i = 0; i < 256; i++)
			*ip++ = 0;
		for(i = newsiz; i < oldsiz; i++)
			bwrite(bmap(i), &sblock);
		return;
	}
	checkfl(sblock.s_rootlbn, 0);
	if (ndup) {
		printf("%l dups in free\n", ndup);
		nerror =| 02;
	}
	j = 0;
	for (ip = cmap; ip < &cmap[4096];) {
		i = *ip++;
		while (i) {
			if (i<0)
				j--;
			i =<< 1;
		}
	}
	j =+ sblock.s_cfsiz;
	if (j)
		printf("missing%5l\n", j);
	printf("free  %6l\n", nfree);
	printf("contiguous files %d\n", ncontig);
	printf("largest file %d\n", nmaxfile);
	printf("largest free area %d\n", nmaxfree);
	close(fi);
}

brk(){}

pass1(aip)
struct inode *aip;
{
	int firstc, lastc;
	long firstb, lastb;
	register i, j, *ip;
	extern long dpmul();

	ip = aip;
	if((ip->i_mode & IALLOC) == 0 || (ip->i_mode & IFMT) != IFCHR
	    || ip->i_addr->i_cflag != ICFLAG)
		return;
	firstb = ip->i_addr->i_cbase;
	lastb = ip->i_addr->i_clast;
	ncontig++;
	if(firstb < sblock.s_cfb0 || firstb > lastb 
	   || lastb > (dpmul(sblock.s_chunksiz, sblock.s_cfsiz) + sblock.s_cfb0)) {
		printf("%ld - %ld bad contiguous blocks, inode %d\n", firstb,
			lastb, ino);
		return;
	}
	if(sblock.s_devc != ip->i_addr->i_cdev) {
		printf("bad contiguous device %d %d, inode %d\n", sblock.s_devc,
			ip->i_addr->i_cdev, ino);
		return;
	}
	if(firstb % sblock.s_chunksiz != 0 ||
	   (lastb+1) % sblock.s_chunksiz != 0 ||
	   (((lastb - firstb)/sblock.s_chunksiz) % sblock.s_boundsiz) != 0) {
		printf("%ld - %ld bad chunk boundaries, inode %d\n", firstb,
			lastb, ino);
		return;
	}
	firstc = (firstb - sblock.s_cfb0)/sblock.s_chunksiz;
	lastc  = (lastb  - sblock.s_cfb0)/sblock.s_chunksiz;
	if(lastc - firstc >= nmaxfile)
		nmaxfile = lastc - firstc + 1;
	putchunk(firstc, lastc);
}

bread(bno, buf, cnt)
int *buf;
{
	register *ip;

	seek(fi, bno, 3);
	if (read(fi, buf, cnt) != cnt) {
		printf("read error %d\n", bno);
		if (sflg) {
			printf("No update\n");
			sflg = 0;
		}
		for (ip = buf; ip < &buf[256];)
			*ip++ = 0;
	}
}

bwrite(bno, buf)
{
	if(!sflg)
		return;
	seek(fi, bno, 3);
	if (write(fi, buf, 512) != 512)
		printf("write error %d\n", bno);
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

checkfl(lbn, depth)	/* add chunks on free list file to bit map */
{
	int buf[256];
	struct bnode *bp;
	struct key *k;
	int i, pbn, m, nf;

	if((pbn = bmap(lbn)) == NULL || depth > MAXDEPTH)
		return;
	bread(pbn,(bp = &buf), 512);
	nf = bp->nfarea;
	if(nf < 0 || nf > NKEY) {
		printf("bad bnode %d\n", lbn);
		return;
	}
	k = bp->keys;
	for(i = 0; i < nf; i++, k++) {		/* inorder recursive tree traversal */
		if(k->max > 0)
			checkfl(k->lson, depth + 1);
		putchunk(k->base, k->last);
		if((m = k->last - k->base + 1) > nmaxfree)
			nmaxfree = m;
		nfree =+ m;
	}
	if(k->max > 0)
		checkfl(k->lson, depth + 1);
}

/* Make new free list.
   Again, inorder ...
   with "nfree" areas put at the current depth, if possible.
   Otherwise, the current level is filled to capacity, and the
   others are divided equally, left to right, among lower level bnodes.
 */
makefree(nfree, depth)
{
	struct key key;
	register struct key *retk, *k;
	struct bnode bnode;
	int i, nson, siz;

	if(depth > MAXDEPTH) {
		printf("%d too deep in new free list\n", depth);
		return;
	}
	key.lson = sblock.s_flsiz++;
	key.max = 0;
	bnode.nfarea = min(NKEY, nfree);
	bnode.father = NULL;
	k = bnode.keys;
	for(i = 0; i <= bnode.nfarea; i++, k++) {
		nson = nfree/(NKEY - i + 1);
		if(nson) {
			retk = makefree(nson, depth + 1);
			k->max = retk->max;
			k->lson = retk->lson;
			if(retk->max > key.max)
				key.max = retk->max;
			nfree =- nson;
		} else {
			k->max = 0;
			k->lson = 0;
		}
		if(nfree == 0)
			break;
		if(!(retk = getfree()))
			return;
		k->base = retk->base;
		k->last = retk->last;
		if((siz = k->last - k->base + 1) > key.max)
			key.max = siz;
		nfree--;
	}
	bwrite(bmap(key.lson), &bnode);
	return(&key);
}

#define map(n) ((cmap[((n)>>4) & 07777]) & (1 << ((n)&017)))

getfree()
{
	static start 0;
	static struct key key;
	register unsigned s;

	s = start;
	while(map(s))
		s++;
	if(s >= sblock.s_cfsiz) {
		printf("%d out of range free area.\n", s);
		return(0);
	}
	key.base = s++;
	while(!map(s) && s)
		s++;
	key.last = min(s, sblock.s_cfsiz) - 1;
	if(!s)
		key.last = sblock.s_cfsiz - 1;
	start = s;
	return(&key);
}

countfree()
{
	register last, pres, j;
	int nfree, b, ch;

	last = map(0);
	nfree = 0;
	if(last == 0)
		nfree++;
	j = 17;
	for(b = 0; b < 4096; b++) {
		pres = cmap[b];
		if(pres == 0 || pres == -1)
			goto skip;
		ch = b<<4;
		for(j = 0; j < 16; j++) {
			pres = map(ch);
			ch++;
	    skip:
			if(last && !pres)
				nfree++;
			last = pres;
		}
	}
	if(map(sblock.s_cfsiz))
		--nfree;
	return(nfree);
}

#undef map(n)

bmap(lbn)	/* similar to internal "bmap" routine,
		   maps a logical block # in free list file
		   to a pyhsical block #, through inode and indirect blocks
		 */
{
	register i, *ip;
	static int iblock[256];

	if(lbn > flsiz || lbn < 0) {
		printf("bad block # in free list %d\n", lbn);
		sflg = 0;
		return(NULL);
	}
	ip = &inodefl;
	if((ip->i_mode & ILARG) == 0)
		return(ip->i_addr[lbn]);
	i = lbn>>8;
	if(i > 7)
		i = 7;
	bread(ip->i_addr[i], iblock, 512);
	if(i == 7)	/* huge algorithm */
		bread(iblock[(lbn>>8) - 7], iblock, 512);
	return(iblock[lbn&0377]);
}

putchunk(first, last)	/* put chunks onto bit map */
{
	register m, n, b;

	for(b = first; b <= last; b++) {
		for(n = 0; clist[n] != -1; n++)
			if(b == clist[n])
				printf("%d chunk found; inode = %d\n", b, ino);
		m = 1<<(b&017);
		n = (b>>4) & 0777;
		if(cmap[n] & m) {
			printf("%d dup, inode = %d\n", b, ino);
			ndup++;
		}
		cmap[n] =| m;
	}
}

getfl()		/* see if file system looks contiguous, and
		   free list inode looks OK 
		 */
{
	register *ip;

	for(ip = &(sblock.s_cfsiz); ip <= &(sblock.s_infl); )
		if(*ip++ == 0) {
			printf("missing superblock parameter\n");
			return(0);
		}
	return(1);
}
