#

/*
 *	teus hagen
 *	mathematisch centrum
 *	amsterdam
 *	1976
 */

/* system variables */
#include "/usr/sys/param.h"


struct {
	char	n_name[8];
	int	n_type;
	char	*n_value;
}	nl[3];

int	aflg;
int	pflg;
int	lflg;
int	kflg;
int	xflg;
int	mem;
char	*coref;

struct s_file {
	char	f_flag;
	char	f_count;
	int	f_inode;
	char	*f_offset[2];
}	file[NFILE];

struct  d_inode {	/* as it appears on disk */
	int	di_mode;
	char	di_nlink;
	char	di_uid;
	char	di_gid;
	char	di_szeo;
	char	*di_size1;
	int	di_addr[8];
	int	di_acct[2];
	int	di_modt[2];
};
struct  s_inode {	/* as it appears in core */
	char	i_flag;
	char	i_count;
	int	i_dev;
	int	i_number;
	int	i_mode;
	char	i_nlink;
	char	i_uid;
	char	i_gid;
	char	i_sizeo;
	char	*i_size1;
	int	i_addr[8];
	int	i_last;
	unsigned i_lrt;
}	inode[NINODE];
char	i_cntcntrl[NINODE];

/* flags */
#define	FREAD	01
#define	FWRITE	02
#define	FPIPE	04

setup(p,s)	char	*p,*s;{
	while(*p++ = *s++);
}

done(){
	flush();
	exit();
}

prflags(flag,flag1,char1,flag2,char2,flag3,char3)  
	char char1, char2, char3;
{
		if(flag1 & flag)
			putchar(char1);
		else putchar('_');
		if(flag2 & flag)
			putchar(char2);
		else putchar('_');
		if(flag3 & flag)
			putchar(char3);
		else putchar('_');
}

struct {
	int devnr;
};

getinode(inr) char *inr;{
	return((inr-nl[1].n_value)/sizeof inode[0]);
}
struct 	devices {
	char	d_major;
	char	d_minor;
	char	*d_name;
} devices[] {
	{ 0, 0, "/dev/rk0" },
	{ 0, 1, "/dev/rk1" },
	{ 0, 2, "/dev/rk2" },
	{ -1, -1, "?" }
};


int	getdevice(devicenr){
	register	int i;
	for(i = devicenr & 07; i < sizeof devices / sizeof devices[0]; i++){
		if( devices[i].devnr != devicenr) continue;
		return(i);
	}
	return(i-2);
}


char	tbuf[16];
char	tbufg[16];
int	uidfil	-1;
int	gidfil	-1;
int	olduid	-1;
int	oldgid	-1;

struct {
	int	fdes;
	int	nleft;
	char	*nextp;
	char	buff[512];
} inf;

#define	UID	0
#define	GID	1
getname(id, buf,fie)
char buf[];
{
	register  int c, n, i;
	int j;
	if(fie == UID){
		if(olduid == id) return(0);
	} else
	if(oldgid == id) return(0);
	inf.fdes = (fie == UID)?uidfil:gidfil;
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
	} while (n != id);
	buf[i++] = '\0';
	if(n == id) 
		if(fie == UID) olduid = n;
		else oldgid = n;
	return(n-id);
}

print(aiptr,space) struct s_inode *aiptr; {
	register struct s_inode *iptr;
	int devindex;
	int parent;
	iptr = aiptr;
	if(space) printf("          ");
	prflags(iptr->i_flag,
		040,'T',020,'W',010,'M'
	);
	prflags(iptr->i_flag,
		04,'A',02,'U',01,'L'
	);
	getname(iptr->i_uid,tbuf,UID);
	getname(iptr->i_gid,tbufg,GID);
	devindex = getdevice(iptr->i_dev);
	printf(" %4d %-2d  %-6.6s %6.6s%7s %5.5s ",
		iptr->i_number,	/* inumber */
		iptr->i_count,	/* inumber refcnt */
		tbuf,tbufg,	/* name of uid,gid */
		locv(iptr->i_sizeo,iptr->i_size1), /* number of bytes */
		&devices[devindex].d_name[5] /* device name */
	);
	if(lflg || pflg) 
		parent = findino(iptr->i_number,devindex);
	putchar('\n');
	if(pflg)
		if(parent != iptr->i_number)
			printp(parent,iptr->i_dev);
}

printp(inonr,devnr) {
	register struct s_inode *iptr;
	register i;
	if(inonr == 0) return;
	for(i=0;i<NINODE;i++){
		iptr = &inode[i];
		if(iptr->i_dev != devnr) continue;
		if(iptr->i_number != inonr) continue;
		i_cntcntrl[i]++;
		print(iptr,1);
	}
}

main(argc,argv) char **argv;{
	register	int	i;
	register	struct	s_inode	*inoder;
	register	struct	s_file	*filer;
	char		*ap;
	int		j;
	struct	devices	*devptr;

	if( argc > 1){
		ap = argv[1];
		while( *ap ) switch( *ap++){
		case 'a':	/* for more inodes not related to a file */
			aflg++;
			break;
		case 'l':	/* for a name of an inode */
			lflg++;
			break;
		case 'p':	/* print a parent inode if in core */
			pflg++;
			break;
		case 'k':	/* take a corefile */
			kflg++;
			break;
		case 'x':	/* print unused inodes */
			xflg++;
			break;
		}
	}

	setup(&nl[0],"_file");
	setup(&nl[1],"_inode");
	nlist(argc>2?argv[2]:"/unix",nl);
	if(nl[0].n_type == 0){
		printf("No namelist\n");
		done();
	}
	coref = "/dev/mem";
	if(kflg) coref = "/usr/sys/core";
	if((mem = open(coref,0))<0){
		printf("No mem\n");
		done();
	}
	seek(mem,nl[0].n_value,0);
	read(mem,file,sizeof file);
	seek(mem,nl[1].n_value,0);
	read(mem,inode,sizeof inode);
	uidfil = open("/etc/passwd",0);
	gidfil = open("/etc/group",0);

	printf("\nFLGS FCNT IFLAGS INUM CNT  UID     GID  BYTES   DEVICE   %s\n\n",
		lflg?"NAME":"");
	for(devptr= &devices[0];devptr < &devices[sizeof devices / sizeof devices[0] ];
		devptr++){
		for(i=0; i < NFILE; i++){
			filer = &file[i];
			if( filer->f_count ==0) continue;
			j = getinode(filer->f_inode);
			inoder = &inode[j];
			if(inoder->i_dev != devptr->devnr) continue;
			prflags(filer->f_flag,FREAD,'R',FWRITE,'W',FPIPE,'P');
			i_cntcntrl[j]++;
			printf("   %-4d",filer->f_count);
			print(inoder,0);
		}
	}
	if(!aflg) exit();
	pflg = 0;
	printf("\nMORE CORE INODES:\n");
	for(j = (sizeof devices / sizeof devices[0])-2;
		j >= 0;
		j--){
		devptr = &devices[j];
		for(i=0; i < NINODE; i++){
			inoder = &inode[i];
			if(inoder->i_dev != devptr->devnr) continue;
			if((!xflg || inoder->i_count) && 
				i_cntcntrl[i] == inoder->i_count)
				continue;
			print(inoder,1);
		}
	}
	done();
}

#define NRINODE	16
/*
 * Definition of the unix super block.
 */
struct	filsys
{
	int	s_isize;	/* size in blocks of I list */
	int	s_fsize;	/* size in blocks of entire volume */
	int	s_nfree;	/* number of in core free blocks (0-100) */
	int	s_free[100];	/* in core free blocks */
	int	s_ninode;	/* number of in core I nodes (0-100) */
	int	s_inod[100];	/* in core free I nodes */
	char	s_flock;	/* lock during free list manipulation */
	char	s_ilock;	/* lock during I list manipulation */
	char	s_fmod;		/* super block modified flag */
	char	s_ronly;	/* mounted read-only flag */
	int	s_time[2];	/* current date of last update */
	int	pad[50];
};

struct	d_inode	d_inode[NRINODE];

#define	NI	20
#define	NDIRS	787

int	fi;
int	oldindex	-2;
struct	htab {
	int	hino;
	int	hpino;
	char	hname[14];
} htab[NDIRS];
int	nhent	;
int	pass1(), pass2(), pass3();
int	(*pass[])()	{ pass1, pass2, pass3 };
int	ino;
int	nfiles;
struct dir {
	int	ino;
	char	name[14];
};

/* flags for inode */
#define	ILOCK	01		/* inode is locked */
#define	IUPD	02		/* inode has been modified */
#define	IACC	04		/* inode access time to be updated */
#define	IMOUNT	010		/* inode is mounted on */
#define	IWANT	020		/* some process waiting on lock */
#define	ITEXT	040		/* inode is pure text prototype */

/* modes */
#define	IALLOC	0100000		/* file is used */
#define	IFMT	060000		/* type of file */
#define		IFDIR	040000	/* directory */
#define		IFCHR	020000	/* character special */
#define		IFBLK	060000	/* block special, 0 is regular */
#define	ILARG	010000		/* large addressing algorithm */
#define	ISUID	04000		/* set user id on execution */
#define	ISGID	02000		/* set group id on execution */
#define ISVTX	01000		/* save swapped text even after use */
#define	IREAD	0400		/* read, write, execute permissions */
#define	IWRITE	0200
#define	IEXEC	0100

findino(inonr,sysindex) {	/* find a name and return a parent */
	register i, j, pno;
	struct	filsys	sblock;
	pno = 2;
	if( sysindex != oldindex ){
		if(fi)close(fi);
		pno = 0;
		if((fi = open(devices[sysindex].d_name, 0)) <0){
			printf("Can't open %s\n",devices[sysindex].d_name);
			done();
		}
		oldindex = sysindex; 
		sync();
		bread(1, &sblock, 512);
		nfiles = sblock.s_isize*16;
		for (i=0; i<NDIRS; i++)
			htab[i].hino = 0;
	}
	for ( ; pno<3; pno++) {
		ino = 0;
		for (i=0; ino<nfiles; i++) {
			bread(i+2, &d_inode[0], sizeof d_inode);
			for (j=0; j<NRINODE && ino<nfiles; j++) {
				ino++;
				if((*pass[pno])(&d_inode[j],inonr)) return(ino);
			}
		}
	}
	return(0);
}

pass1(ip,nil)    /* set allocated  directory-inodenumbers in table */
struct	d_inode	*ip;
{
	if ((ip->di_mode&IALLOC)==0 || (ip->di_mode&IFMT)!=IFDIR)
		return(0);
	lookup(ino, 1);
	return(0);
}

pass2(ip,nil)	/* set parents and the name of the inode in the table */
struct d_inode *ip;
{
	register doff;
	register struct htab *hp;
	register struct dir *dp;
	int i;
	if ((ip->di_mode&IALLOC)==0 || (ip->di_mode&IFMT)!=IFDIR)
		return(0);
	doff = 0;
	while (dp = dread(ip, doff)) {
		doff =+ 16;
		if (dp->ino==0)
			continue;
		if (dotname(dp))
			continue;
		if ((hp = lookup(dp->ino, 0)) == 0)
			continue;	/* may not happen in a splendid filesystem */
		hp->hpino = ino;
		for (i=0; i<14; i++)
			hp->hname[i] = dp->name[i];
	}
	return(0);
}

pass3(ip,inr)  struct  d_inode *ip; {
	register doff;
	register struct htab *hp;
	register struct dir *dp;
	int i;
	if( (ip->di_mode&IALLOC)==0 || (ip->di_mode&IFMT)!=IFDIR)
		return(0);
	if( inr == 1){
		printf("/");
		return(1);
	}
	doff = 0;
	while( dp = dread(ip,doff)){
		doff =+ 16;
		if(dp->ino==0) continue;
		if(dotname(dp))continue;
		if(dp->ino==inr) break;
	}
	if( !dp ) return(0);
	if(!lflg) return(1);
	pname(ino,0);
	printf("/%.14s",dp->name);
	return(1);
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

lookup(i, ef)	/* hash inode in table, ef!=0 write it in table */
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
		printf("Out of core-- increase NDIRS\n");
		done();
	}
	hp->hino = i;
	return(hp);
}

dread(aip, aoff)
{
	register b, off;
	register struct d_inode *ip;
	static ibuf[256];	/* indirect buffer */
	static char buf[512];

	off = aoff;
	ip = aip;
	if ((off&0777)==0) {	/* read only once a block */
		if (off==0177000) {
			printf("Monstrous directory %l\n", ino);
			return(0);
		}
		if ((ip->di_mode&ILARG)==0) {
			if (off>=010000 || (b = ip->di_addr[off>>9])==0)
				return(0);
			bread(b, buf, 512);
		} else {
			if (off==0) {
				if (ip->di_addr[0]==0)
					return(0);
				bread(ip->di_addr[0], ibuf, 512);
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
		printf("read error %d\n", bno);
		done();
	}
}
