#

/*
 * directory link count check
 *
 * modified to do repair, Mike Tilson, Feb. 1977  (Toronto)
 * modified to put orphaned inodes in "/lost" instead of just clearing them.
 * --ghg 04/15/79.
 */

#define DISASTER 04
#define PROBLEM 02
#define MINORPROBLEM 01
#define ALLSWELL 0
#define NULL 0

#define PURDUE          /* Root inode always nlink=127 at Purdue */

char	*dargv[]
{
	"/dev/root",
	"/dev/sys",
	"/dev/u0",
	"/dev/u1",
	0
};

#define NINODE	16*16
#define	NI	20

#include <ino.h>
#include <filsys.h>

struct	inode	inode[NINODE];
struct  inode   dinode[16];
struct	filsys	sblock;

int	sflg;
int	rflg;
int	qflg;
int	headpr;

int	ilist[NI] { -1};
int	fi;
char	*ecount;
char	*lasts;
int	ino;
int	nerror;
int	nminor;
int	nfiles;
struct dir {
	int	ino;
	char	name[14];
};

char *dbno;     /* last directory block read in */
char dbuf[512]; /* buffer for last dir block read in */

main(argc, argv)
char **argv;
{
	register char **p;
	register int n, *lp;

	ecount = sbrk(0);
	if (argc == 1) {
		for (p = dargv; *p;)
			check(*p++);
		if(nerror)
			return(nerror==nminor?MINORPROBLEM:PROBLEM);
		else
			return(ALLSWELL);
	}
	while (--argc) {
		argv++;
		if (**argv=='-') switch ((*argv)[1]) {
		case 's':
			sflg++;
			continue;

		case 'r':
			rflg++;
			continue;

		case 'q':
			qflg++;
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
			printf("Bad flag\n");
		}
		check(*argv);
	}
	if(nerror)
		return(nerror==nminor?MINORPROBLEM:PROBLEM);
	else
		return(ALLSWELL);
}

char *devname;
int devpr;

check(file)
char *file;
{
	register i, j;
	fi = open(file, rflg?2:0);
	if(fi < 0) {
		printf("cannot open %s\n", file);
		nerror++;
		return;
	}
	devpr = 0;
	headpr = 0;
	if(!qflg) {
		printf("%s:\n", file);
		devpr++;
	}
	else
		devname = file;
	sync();
	bread(1, &sblock, 512);
	nfiles = sblock.s_isize*16 + 1; /* insure ecount gets cleared */
	if (lasts < nfiles) {
		if ((sbrk(nfiles - lasts)) == -1) {
			prnt("Not enough core\n");
			exit(DISASTER);
		}
		lasts = nfiles;
	}
	for (i=0; i<nfiles; i++)
		ecount[i] = 0;
	nfiles =- 1;
	ino= 0;
	for(i=0; ino<nfiles; i =+ NINODE/16) {
		bread(i+2, inode, sizeof inode);
		for(j=0; j<NINODE && ino<nfiles; j++) {
			ino++;
			pass1(&inode[j]);
		}
	}
	ino = 0;
	for (i=0; ino<nfiles; i =+ NINODE/16) {
		bread(i+2, inode, sizeof inode);
		for (j=0; j<NINODE && ino<nfiles; j++) {
			ino++;
			pass2(&inode[j]);
		}
	}
}

pass1(aip)
struct inode *aip;
{
	register doff;
	register struct inode *ip;
	register struct dir *dp;
	int i;

	ip = aip;
	if((ip->i_mode&IALLOC) == 0)
		return;
	if((ip->i_mode&IFMT) != IFDIR)
		return;
	doff = 0;
	while (dp = dread(ip, doff)) {
		doff =+ 16;
		if (dp->ino==0)
			continue;
		for (i=0; ilist[i] != -1; i++)
			if (ilist[i]==dp->ino)
				prnt("%5l arg; %l/%.14s\n", dp->ino, ino, dp->name);
		/* mdt Oct/77 */
		if(dp->ino<1 || dp->ino>nfiles) {
			prnt("inode %d invalid, in directory inode %d\n", dp->ino, ino);
			nerror++;
		}
		else
			ecount[dp->ino]++;
		/* end mdt Oct/77 */
	}
}

pass2(aip)
{
	register struct inode *ip;
	register i;
	int links, icnt;

	ip = aip;
	i = ino;
	if ((ip->i_mode&IALLOC)==0 && ecount[i]==0)
		return;
	/* third condition below fixes bug in distributed version */
	if (ip->i_nlink==ecount[i] && ip->i_nlink!=0 && ip->i_mode&IALLOC)
		return;
#ifdef PURDUE
	if (i == 1)
		return;
#endif
	if (headpr==0) {
		prnt("entries	#links inode_cnt\n");
		headpr++;
	}
	links = ecount[i]&0377;
	icnt = ip->i_nlink&0377;
	prnt("%7l %6d %6d", ino,
	    links, icnt);
	nerror++;
	/* decide if serious */
	if((ip->i_mode&IALLOC && links<icnt) || rflg)
		nminor++;
	/* try to correct the problem  */
	/*
	 * if the inode is allocated
	 *      if not in any file, do nothing for moment, will be handled later
	 *  	else make its link count agree with number of directory refs
	 * else inode is not allocated, but IS in some file
	 *	in this case, we clear the inode, since it likely has
	 *	random block pointers, and then we allocate it with
	 *	the proper link count.
	 *
	 * not gauranteed to be a perfect fix, but should prevent
	 * spreading contagion.
	 * after this fixing is done, icheck -s will likely be needed.
	 */
	if(rflg) {
		if(ip->i_mode&IALLOC) {
			if(links==0)
				if (icnt == 0)
					clri(i);
				else
					lost(i);        /* stuff into /lost */
			else
				counti(i, links);
		}
		else {
			clri(i);
			counti(i, links);
		}
	}
	putchar('\n');
}

clri(inum)
	register inum;
{
	register block;
	register i;
	int j;
	struct inode ibuf[16];

	if (inum == 1) {
		printf("Can't clear inode 1 !!!!\n");
		exit(DISASTER);
	}
	block=(inum+31)/16;
	bread(block, ibuf, 512);
	i=(inum+31)%16;
	ibuf[i].i_mode = 0;
	ibuf[i].i_nlink = 0;
#ifdef PURDUE
	ibuf[i].i_uid0 = 0;     /* Purdue uses 16 bit uid's no gid's */
	ibuf[i].i_uid1 = 0;
#else
	ibuf[i].i_uid = 0;
	ibuf[i].i_gid = 0;
#endif
	ibuf[i].i_size0 = 0;
	ibuf[i].i_size1 = 0;
	for(j=0; j!=8; j++)
		ibuf[i].i_addr[j] = 0;
	bwrite(block, ibuf, 512);
	prnt("     cleared");
}

counti(inum, nlink)
	register inum;
{
	register block;
	register i;
	struct inode ibuf[16];

	block=(inum+31)/16;
	bread(block, ibuf, 512);
	i=(inum+31)%16;
	ibuf[i].i_nlink = nlink;
	if((ibuf[i].i_mode&IALLOC)==0 && nlink!=0)
		ibuf[i].i_mode = IALLOC; /* allocate the inode if necessary */
	bwrite(block, ibuf, 512);
	prnt("     count adjusted");
}

/*
 * lost - attempt to place orphaned inode into "/lost" directory
 * directory "lost" must reside in the root directory of the filesystem
 * being recovered.  Also, the size of the dir inode should be %512.
 * no blocks can be allocated to this directory during a recovery, and
 * hence a "/lost full" message will be issued if more space is needed.
 * --ghg 04/16/79.
 */
lost(i)
{
	static struct inode *dipp;
	int inum, block;
	register struct inode *dip;
	register struct dir *dp;
	register doff;
	char *np;

	if (dipp == NULL) {
		/*
		 * Find "/lost" directory and set dipp pointing to its inode.
		 */
		bread(2, &dinode[0], 512);      /* get root inode */
		doff = 0;
		while (dp = dread(&dinode[0], doff)) {
			doff =+ 16;
			if (dp->ino)
				if (match(dp->name,"lost")) {
					inum = dp->ino;
					block = (inum+31)/16;
					bread(block, dinode, 512);
					dipp = &dinode[(inum+31)%16];
					goto found;
				}
		}
		prnt("     /lost not found - file orphaned");
		return(NULL);
	}
found:
	/*
	 * Insert this inode in "/lost" directory with a filename
	 * as the ascii integer representation of the inode number.
	 */
	dip = dipp;
	doff = 0;
	while (dp = dread(dip, doff)) {
		doff =+ 16;
		if (dp->ino)
			continue;
		/* empty slot found in /lost */
		dp->ino = i;
		for (np=dp->name; np < &dp->name[14]; *np++ = 0);
		strcopy(itoa(i), dp->name);
		bwrite(dbno, dbuf);
		prnt("     inserted in /lost");
		return(1);
	}
	prnt("     /lost full - file orphaned");
	return(NULL);
}

strcopy(sa, sb)
register char *sa, *sb;
{

	while (*sb++ = *sa++);
}

match(sa, sb)
register char *sa, *sb;
{

	while (*sa == *sb++)
		if (*sa++ == '\0')
			return(1);
	return(0);
}

dread(aip, aoff)
{
	register b, off;
	register struct inode *ip;
	static ibuf[256];

	off = aoff;
	ip = aip;
	if ((off&0777)==0) {
		if (off==0177000) {
			prnt("Monstrous directory %l\n", ino);
			nerror++;
			return(0);
		}
		if ((ip->i_mode&ILARG)==0) {
			if (off>=010000 || (b = ip->i_addr[off>>9])==0)
				return(0);
			bread(dbno=b, dbuf, 512);
		} else {
			if (off==0) {
				if (ip->i_addr[0]==0)
					return(0);
				bread(dbno=ip->i_addr[0], ibuf, 512);
			}
			if ((b = ibuf[(off>>9)&0177])==0)
				return(0);
			bread(dbno=b, dbuf, 512);
		}
	}
	return(&dbuf[off&0777]);
}

bread(bno, buf, cnt)
{

	seek(fi, bno, 3);
	if(read(fi, buf, cnt) != cnt) {
		prnt("read error %d\n", bno);
		exit(DISASTER);
	}
}

bwrite(bno, buf)
{

	seek(fi, bno, 3);
	if(write(fi, buf, 512) != 512) {
		prnt("write error %d\n", bno);
		exit(DISASTER);
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

prnt(a,b,c,d,e,f,g) {
	if(!devpr) {
		printf("%s:\n", devname);
		devpr++;
	}
	printf(a,b,c,d,e,f,g);
}
