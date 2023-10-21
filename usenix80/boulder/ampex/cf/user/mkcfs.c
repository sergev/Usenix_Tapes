/*
   add a contiguous file system onto an existing normal file system 
 */

#include "/usr/sys/cf.h"
#include "/usr/sys/filsys.h"

struct inodestat {		/* inode structure for "stat" call */
	int   dev;
        int   inumber;       /* +2 */
        int   flags;         /* +4: see below */
        char  nlinks;        /* +6: number of links to file */
        char  uid;           /* +7: user ID of owner */
        char  gid;           /* +8: group ID of owner */
        char  size0;         /* +9: high byte of 24-bit size */
        int   size1;         /* +10: low word of 24-bit size */
        int   addr[8];       /* +12: block numbers or device number */
        int   actime[2];     /* +28: time of last access */
        int   modtime[2];    /* +32: time of last modification */
} ;
#define	IFMT	060000		/* type of file */
#define		IFBLK	060000	/* block special, 0 is regular */

struct bnbuf {
	struct bnode b;
	char padd[512 - sizeof(struct bnode)];
};

int flist -1, devb -1, chunksiz 1, boundsiz 1;
char *flname;

main(argc, argv)	/* args are: 1:  name of Unix normal file system
				     2:  size of c.f.s. (in chunks)
				     3:  device where cfs char i/o goes
				     devb=d (device where c.f.s. is,  default
					     = same device as normal f.s.)
				     fl=name (name of free list file to be
					      created, default = ".flist")
				     cs=nblks (chunk size in blocks, default = 1)
				     bs=nchunks (bound size, default = 1)
				     devb=d (device where c.f.s. is, default = same device as normal f.s.)
			 */
char *argv[];
{
	struct filsys fs;
	struct inodestat ip;
	struct bnode *bp;
	struct key *k;
	struct bnbuf bn;
	register i, j, *p;
	int nblk, oldfs, nbnode, devc;
	unsigned nfssiz, firstb;
	char *fsname;

	if(argc < 4 || argc > 8)
		error("arg count", "");
	fsname = argv[1];
	flname = ".flist";
	nfssiz = getnum(argv[2]);
	devc = getnum(argv[3]);
	for(i=4; i<argc; i++)
		getarg(argv[i]);
	if(stat(flname, &ip) >= 0)
		error(flname, "free list already exists");
	oldfs = open(fsname, 2);
	if(oldfs < 0)
		error("can't open", fsname);
	i = seek(oldfs, 1, 3);
	j = read(oldfs, &fs, 512);
	if(i < 0 || j < 512)
		error("can't read superblock of", fsname);
	for(p = &(fs.s_devc); p <= &(fs.s_boundsiz); p++)
		if(*p != 0)
			error(fsname, "junk on superblock");
	if(fstat(oldfs, &ip) < 0)	/* ip contains old sb */
		error(fsname, "fstat");
	if((ip.flags & IFMT) != IFBLK)
		error(fsname, "not a mountable file system");
	if(devb == -1)		/* default: on same device as normal fs */
		devb = ip.addr[0];
	firstb = 0;
	if(devb == ip.addr[0])
		firstb = fs.s_fsize;
	flist = creat(flname, 0664);
	if(flist < 0)
		error("can't create", flname);
	if(fstat(flist, &ip) < 0)	/* ip contains flist */
		error(flname, "fstat");
	fs.s_devb = devb;		/* set up new info on superblock */
	fs.s_devc = devc;
	fs.s_cfb0 = firstb;
	fs.s_cfsiz = nfssiz;
	fs.s_chunksiz = chunksiz;
	fs.s_boundsiz = boundsiz;
	fs.s_flsiz = 1;
	fs.s_infl = ip.inumber;
	fs.s_rootlbn = 0;

	p = bp = &bn;
	for(i = 0; i < 256; i++)
		*p++ = 0;
	/* write zeroes into free list, up to maximum b-tree size */
	for(i = 0, nblk = 1; i < MAXDEPTH; i++) {
		for(j = 0; j < nblk; j++)
			if(write(flist, bp, 512) < 512)
				error(flname, "free list write");
		nblk =* (NKEY + 1);
	}
	seek(flist, 0, 0);
	bp->nfarea = 1;
	k = bp->keys;
	k->base = 0;
	i = nfssiz%boundsiz;
	k->last = nfssiz - i - 1;
	if(i > 0) {
		(++k)->base = nfssiz - i;
		k->last = nfssiz - 1;
		bp->nfarea++;
	}
	write(flist, bp, 512);
	seek(oldfs, 1, 3);
	if(write(oldfs, &fs, 512) < 512)
		 error(fsname, "superblock write");
}

getnum(arg)
char *arg;
{
	long ans;
	int i, base;
	register char c, *p;
	struct{int hiword, loword;};

	ans = 0;
	base = 10;
	if(*arg == '0')
		base = 8;
	p = arg;
	for(i=0; i<=16; i++) {
		c = *p++;
		if(c == '\0')
			return(ans.loword);
		if(c < '0' || c > '9')
			error(arg, "bad input number");
		ans = ans * base + c - '0';
		if(ans.hiword)
			error(arg, "number too large");
	}
	return(ans.loword);
}

error(s1, s2)
{
	printf("error:  %s %s.\n", s1, s2);
	if(flist >= 0)
		unlink(flname);
	exit();
}

getarg(str)
char *str;
{
	register ans;
	register char *c;
	
	c = str;
	while(*c && *c++ != '=')
		;
	if(*str == 'f') {
		flname = c;
		return;
	}
	ans = getnum(c);
	switch(*str) {
		case 'c':
			if(ans <= 0)
				error(str, "unreasonable chunk size");
			chunksiz = ans;
			break;
		case 'b':
			if(ans <= 0)
				error(str, "unreasonable bound size");
			boundsiz = ans;
			break;
		case 'd':
			devb = ans;
			break;
		default:
			error(str, "funny arg");
			break;
	}
}
