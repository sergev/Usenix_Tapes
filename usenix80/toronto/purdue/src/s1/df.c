#
int	fout;
#define NEWFS	/* use version 7 superblock format */

struct  inode {
  char  minor;         /* +0: minor device of i-node */
  char  major;         /* +1: major device */
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
} statb;
char	*dargv[]
{
	0,
	"/dev/hp0",
	"/dev/hp1",
	"/dev/hp2",
	"/dev/hp3",
	"/dev/hs0",
	"/dev/sj0",
	"/dev/sj1",
	"/dev/sj2",
	"/dev/sj3",
	"/dev/sj4",
	"/dev/sj5",
	"/dev/sj6",
	"/dev/sj7",
	"/dev/sj8",
	0
};
struct
{
	char	*s_isize;
	char	*s_fsize;
	int	s_nfree;
	int	s_free[100];
	int	s_ninode;
	int	s_inode[100];
	char	s_flock;
	char	s_ilock;
	char	s_fmod;
	int	time[2];
#ifndef NEWFS
	int	pad[50];
#endif
#ifdef NEWFS
 	int	pad[40];
 	int	s_tfree;	/* Total free, for subsystem examination */
 	int	s_tinode;	/* Free inodes, for subsystem examination */
 	char	s_fname[6];	/* File system name */
 	char	s_fpack[6];	/* File system pack name */
#endif
} sblock;

int	fi;
int	nbavail;

main(argc, argv)
char **argv;
{
	int i;

	fout = dup(1);
	if(argc <= 1) {
		for(argc = 1; dargv[argc]; argc++);
		argv = dargv;
	}

#ifdef NEWFS
	sync();	/* only needed once for new fs */
#endif
	for(i=1; i<argc; i++) {
		if(argc > 1)
			printf("%-9s ", argv[i]);
		dfree(argv[i]);
	}
	flush();
}

dfree(file)
char *file;
{
	float pcb();
	int i;
	int j;

	if (getuid()) {
		stat(file, &statb);
		if ((statb.flags&060000) != 060000) {
			printf("not block special file\n");
			return;
		}
	}
	if ((fi = open(file, 0)) < 0) {
		printf("cannot open\n");
		return;
	}
#ifndef NEWFS
	sync();
#endif
	bread(1, &sblock);
	i = 0;
	nbavail = sblock.s_fsize - sblock.s_isize -2;
#ifndef NEWFS
	while(alloc())
		i++;
	printf("%6l %6.1f %%\n", i, pcb(i,nbavail));
##endif
#ifdef NEWFS
	i = sblock.s_tfree;
	j = sblock.s_tinode;
	printf("%6l%6.1f%% blocks %6l%6.1f%% inodes\n",
	i, pcb(i,nbavail), j, pcb(j, sblock.s_isize*16));
#endif
	close(fi);
}

float flt(n)
{
	float f;
	f = n;
	if ( f < 0 )
		f =+ 65536.0;
	return(f);
}

float pcb(n,m)
{
	return((flt(n)*100.0)/flt(m));
}

#ifndef NEWFS
alloc()
{
	int b, i, buf[256];

	i = --sblock.s_nfree;
	if(i<0 || i>=100) {
		printf("bad free count\n");
		return(0);
	}
	b = sblock.s_free[i];
	if(b == 0)
		return(0);
	if(b<sblock.s_isize+2 || b>=sblock.s_fsize) {
		printf("bad free block (%l)\n", b);
		return(0);
	}
	if(sblock.s_nfree <= 0) {
		bread(b, buf);
		sblock.s_nfree = buf[0];
		for(i=0; i<100; i++)
			sblock.s_free[i] = buf[i+1];
	}
	return(b);
}

#endif
bread(bno, buf)
{
	int n;
	extern errno;

	seek(fi, bno, 3);
	if((n=read(fi, buf, 512)) != 512) {
		printf("read error %d\n", bno);
		printf("count = %d; errno = %d\n", n, errno);
		flush();
		exit();
	}
}
