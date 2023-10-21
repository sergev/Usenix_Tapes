#
#define	CLEAR	021
#define	DSTATE	020
#define	FSTATE	001
#define	ALLINUM	inum=1;inum<=imax;inum++
#define	ALLAP	ap= &inode->addr[0];ap<&inode->addr[8];ap++) if (*ap
#define	ALLOC	(inode->flags&0100000)
#define	LARGE	(inode->flags&010000)
#define	DIR	((inode->flags&060000)==040000)
#define	NOTSPL	((inode->flags&020000)==0)
#define	MAXDUP	100
struct SB {
	int	isize;
	int	fsize;
	int	nfree;
	int	free[100];
	int	ninode;
	int	finode[100];
	char	flock;
	char	ilock;
	char	fmod;
	int	time[2];
	int	fill[40];
	int	nbfree;
	int	nifree;
	char	fname[6];
	char	fpack[6];
} sb;
struct INODE {
	int	flags;
	char	nlinks;
	char	uid;
	char	gid;
	char	size0;
	int	size1;
	int	addr[8];
	int	actime[2];
	int	modtime[2];
} *inode;
struct DE {
	int	dnum;
	char	dname[14];
};
struct {int INT;};
char
	*bbit,
	*fbit,
	*state,
	*lc,
	pathname[200],
	*pp,
	*name,
	*fmin,
	*fmax,
	sflag,
	nflag,
	yflag,
	*dev,
;
int
	n_free,
	n_blks,
	n_files,
	inum,
	diskr,
	diskw,
	cc,
	dups[MAXDUP],
	*dc,
	*el,
	imax,
	imod,
	mod,
	dmod,
	buf[256],
	ttyargs[3],
;
main(argn,argv)
int	argn;
char	*argv[];
{
	char c,lsflag,lnflag,lyflag;
	int i;
	gtty(0,ttyargs);
	lyflag = lnflag = lsflag = 0;
	chdir("/dev");
	sync();
	if(argn==1) {
		check(dev="rootdev");
		goto fin;
	}
	for(i=1;i<argn;i++)
	if(*argv[i] == '-') {
		while(c = *++argv[i])
		switch(c) {
		case 's':
		case 'S':
			lsflag = 1;
			continue;
		case 'n':
		case 'N':
			lnflag = 1;
			continue;
		case 'y':
		case 'Y':
			lyflag = 1;
			continue;
		default:
			printf("%c option?\n",c);
			goto fin;
		}
	} else {
		sflag = lsflag;
		nflag = lnflag;
		yflag = lyflag;
		dev = argv[i];
		check();
	}
fin:
	if(mod) {
		printf("\n*****BOOT UNIX(NO SYNC!)*****\n");
		for(;;) i = 0;
	}
}
check()
{
	register int *ap,*bp;
	register char *blk;
	int a,b,*cp;
/*	Initialization		*/
	if((diskr = open(dev,0)) == -1) {
		printf("CAN NOT OPEN %s\n",dev);
		return;
	}
	printf("\n%25s",dev);
	if((diskw = open(dev,1)) == -1) {
		nflag = 1;
		printf("(NO WRITE)");
	}
	printf("\n");
	if(seek(diskr,1,3) < 0) rwerr("SEEK",1);
	if(read(diskr,&sb,512)<0) rwerr("READ",1);
	imax = 16 * sb.isize;
	fmin = 2+sb.isize;
	fmax = sb.fsize;
	bbit = sbrk(((fmax>>3)&017777)+1);
	fbit = state = sbrk(imax/4+1);
	lc = sbrk(imax+1);
	el = dc = dups;
	n_files = n_blks = n_free = *dc = 0;
	pp = pathname;
	pathname[0] = '/';
	pathname[1] = 0;
/*	Phase 1		*/
printf("Phase 1 - Check Blocks\n");
	for(ALLINUM) {
		stat();
		if(ALLOC) {
			n_files++;
			if(DIR) set(DSTATE); else set(FSTATE);
			if((lc[inum]=inode->nlinks)==0)
				set(CLEAR);
			if(NOTSPL)for(ALLAP)
			if(pass1(*ap) && LARGE)
				iblock(*ap,&pass1);
		}
	}
/*	Phase 2		*/
	setexit();
	if(dc==dups) goto phase3;
	printf("Phase 2 - Rescan for more DUPS\n");
	for(ALLINUM)
	if(get()) {
		stat();
		if(NOTSPL)for(ALLAP)
		if(pass2(*ap) && LARGE)
			 iblock(*ap,&pass2);
	}
/*	Phase 3		*/
phase3:
	printf("Phase 3 - Check Pathnames\n");
	inum = 1;
	lc[1]++;
	descend();
/*	Phase 4		*/
printf("Phase 4 - Check Reference Counts\n");
	for(ALLINUM)
	switch(get()) {
	case FSTATE:
		if(lc[inum]) adj();
		continue;
	case DSTATE:
	case CLEAR:
		clri();
	}
	stat(imax+1);
/*	Phase 5		*/
	if (sflag) goto salvage;
	printf("Phase 5 - Check Free List\n");
	brk(cp=fbit+(((fmax>>3)&017777)+1));
	bp = bbit;
	for(ap=fbit;ap<cp;) *ap++ = *bp++;
	if(sb.nfree<=0 || sb.nfree>100) goto bad;
	while(blk = sb.free[--sb.nfree]) {
		if(sb.nfree==0) {
			if(seek(diskr,blk,3) < 0) rwerr("SEEK",blk);
			if(read(diskr,&sb.nfree,202)<0) rwerr("READ",blk);
		}
		if(sb.nfree<=0 || sb.nfree>100) goto bad;
		if(blk<fmin || blk>=fmax
		|| (fbit[a=((blk>>3)&017777)]&(b=1<<(blk&07)))) {
bad:
			printf("%15s %-15s\tSALVAGE?","","BAD FREE LIST");
			sflag = reply();
			goto salvage;
		}
		fbit[a] =| b;
		n_free++;
	}
	if((n_blks+n_free)!=(fmax-fmin)) {
		printf("%15l %-15s\tSALVAGE?",fmax-fmin-n_blks-n_free,
		"MISSING");
		sflag = reply();
	}
/*	Phase 6		*/
salvage:
	if(sflag==0) goto statistic;
	printf("Phase 6 - Salvage Free List\n");
	n_free = sb.ninode = sb.nfree = 0;
	sb.free[sb.nfree++] = 0;
	for(blk=fmax-1;blk>=fmin;--blk)
	if((bbit[(blk.INT>>3)&017777]&(1<<(blk&07)))==0) {
		if(sb.nfree==100) {
			if(nflag==0) {
				if(seek(diskw,blk,3)<0) rwerr("SEEK",blk);
				if(write(diskw,&sb.nfree,512)<0)
					rwerr("WRITE",blk);
			}
			sb.nfree = 0;
		}
		sb.free[sb.nfree++] = blk;
		n_free++;
	}
	sb.nbfree = n_free;	/* put free block count in super block */
	sb.nifree = sb.isize*16 - n_files;	/* free inodes */
	if(nflag==0) {
		if(seek(diskw,1,3)<0) rwerr("SEEK",1);
		if(write(diskw,&sb,512)<0) rwerr("WRITE",1);
		mod = 1;
	}
statistic:
	printf("%5l files    %5l blocks     %5l free\n",
	n_files,n_blks,n_free);
	close(diskr);
	close(diskw);
}
pass1(blk)
char	*blk;
{
	register int a,b,*ip;
	if(blk<fmin || blk>=fmax) {
		blkerr("BAD",blk);
		return(0);
	}
	if(bbit[a=((blk>>3)&017777)]&(b=(1<<(blk&07)))) {
		blkerr("DUP",blk);
		if(el > &dups[MAXDUP]) {
			printf("\tEXCESSIVE DUPS   EXIT?");
			if(reply()) exit(); else goto ret;
		}
		ip = dups;
		while(ip<dc)
		if(*ip++ == blk) {
			*el++ = blk;
			goto ret;
		}
		*el++ = *dc;
		*dc++ = blk;
	} else {
		bbit[a] =| b;
		n_blks++;
	}
ret:
	return(1);
}
pass2(blk)
char	*blk;
{
	register int *ip;
	if(blk<fmin || blk>=fmax) return(0);
	ip = dups;
	while(ip<dc)
	if(*ip++ == blk) {
		blkerr("DUP",blk);
		*--ip = *--dc;
		*dc = blk;
		if(dc==dups) reset(); else break;
	}
	return(1);
}
stat()
{
	static struct INODE ibuf[16];
	static int cib;
	register int ib;
	if((ib=2+(inum-1)/16)!=cib) {
		if(imod && (nflag==0)) {
			if(seek(diskw,cib,3)<0) rwerr("SEEK",cib);
			if(write(diskw,ibuf,512)<0) rwerr("WRITE",cib);
			imod = 0;
			mod = 1;
		}
		if(seek(diskr,cib=ib,3) < 0) rwerr("SEEK",cib);
		if(read(diskr,ibuf,512)<0) rwerr("READ",cib);
	}
	return(inode = &ibuf[(inum-1)%16]);
}
iblock(blk,func)
char	*blk,(*func)();
{
	register int *ap;
	ap = buf;
	do {
		getblk(blk);
		if(*ap)
			(*func)(*ap);
	} while(++ap<&buf[256]);
}
blkerr(s,blk)
char	*blk;
char	*s;
{
 	printf("%15l %-15s  I = %l\n",blk,s,inum);
	set(CLEAR);
}
clri()
{
	register int *ap;
	stat();
	printf("%15s %-15sI = %5l\tCLEAR?",
	((inode->nlinks==0)||(get()!=CLEAR))?"UNREFERENCED":"BAD/DUP",
	DIR?"DIRECTORY":"FILE",inum);
	if(reply()) {
		n_files--;
		if(NOTSPL)for(ALLAP)
		if(pass4(*ap) && LARGE)
			iblock(*ap,&pass4);
		for(ap=inode;ap<&inode[1];*ap++ = 0);
		imod = 1;
	}
}
pass4(blk)
char	*blk;
{
	register int a,b,*ip;
	if(blk<fmin || blk>=fmax) return(0);
	if(bbit[a=((blk>>3)&017777)]&(b=1<<(blk&07))) {
		ip = dups;
		while(ip<el)
		if(*ip++ == blk) {
			*--ip = *--el;
			goto ret;
		}
		bbit[a] =& ~ b;
		n_blks--;
	}
ret:
	return(1);
}
adj()
{
	stat();
	if(inode->nlinks==lc[inum])
		clri();
	else {
		printf("%15s %-15sI = %5l\tADJUST?",
		"LINK COUNT",DIR?"DIRECTORY":"FILE",inum);
		if(reply()) {
			inode->nlinks =- lc[inum];
			imod = 1;
		}
	}
}
descend()
{
	register int *ip,*ap;
	int a[8];
	extern int pass3();
	char *lname;
	if(inum>imax)
		return(direrr("I OUT OF RANGE"));
again:
	switch(get()) {
	case DSTATE:
		set(FSTATE);
		lc[inum]--;
		ip = &stat()->addr[0];
		for(ap=a;ap<&a[8];) *ap++ = *ip++;
		*pp++ = '/';
		lname = name;
		name = pp;
		if(LARGE) {
			for(ap=a;ap<&a[8];ap++)
			if(*ap) iblock(*ap,&pass3);
		} else {
			for(ap=a;ap<&a[8];ap++)
			if(*ap) pass3(*ap);
		}
		name = lname;
		*--pp = 0;
		return(0);
	case FSTATE:
		lc[inum]--;
		return(0);
	case 0:
		return(direrr("UNALLOCATED"));
	case CLEAR:
		if(direrr("DUP/BAD")) return(1);
		stat();
		if(DIR) set(DSTATE); else set(FSTATE);
		goto again;
	}
}
pass3(blk)
char	*blk;
{
	register struct DE *dp;
	register char *c;
	dp = buf;
	do {
		getblk(blk);
		if(inum = dp->dnum) {
			c = &dp->dname[0];
			while((*pp = *c++) && pp++ && c<&dp->dname[14]);
			*pp = 0;
			if(descend()) {
				getblk(blk);
				dmod = 1;
				dp->dnum = 0;
			}
			pp = name;
		}
	} while(++dp<&buf[256]);
}
direrr(s)
char	*s;
{
	printf("%15s   I = %-5l%s\tREMOVE?",s,inum,pathname);
	return(reply());
}
reply()
{
	register char	c;
	if(nflag) {
		printf(" no\n");
		return(0);
	}
	if(yflag) {
		printf(" yes\n");
		return(1);
	}
	do
		c = getchar();
	while(c == ' ' || c == '\t');
	stty(0,ttyargs);
	printf("\n");
	if(c == 'y')
		return(1);
	else
		return(0);
}
putchar(c)
char	c;
{
	if(c) write(1,&c,1);
}
getchar()
{
	char c;
	read(0,&c,1);
	return(c);
}
rwerr(s,b)
char	*s,*b;
{
	printf("\nCAN NOT %s: %s\tBLOCK %5l\tEXIT?",s,dev,b);
	if(nflag || reply()) {
		printf("\n\n");
		exit();
	}
}
getblk(blk)
char	*blk;
{
	static char *cb;
	if(blk==cb) return;
	if(dmod && (nflag==0)) {
		if(seek(diskw,cb,3)<0) rwerr("SEEK",cb);
		if(write(diskw,buf,512)<0) rwerr("WRITE",cb);
		dmod = 0;
	}
	if(seek(diskr,cb=blk,3)<0) rwerr("SEEK",blk);
	if(read(diskr,buf,512)<0) rwerr("READ",blk);
}
