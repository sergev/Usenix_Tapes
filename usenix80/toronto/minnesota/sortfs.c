#
/*
 *	Sortfs - a file system sorter.  Sortfs reorders all
 *	the blocks in all the files and the free list of a file
 *	system.  Sortfs also can change the sizes of the file
 *	system and the inode space.  Inode space can only be
 *	increased.
 *
 *	Brad Blasing - University of Minnesota - 77/02/27.
 *	Last modification - 78/07/09.
 */

#include "/usr/sys/ino.h"
#include "/usr/sys/filsys.h"
	/* pad[50] in filsys.h should be changed to pad[48] */

#define MAXBLK	4872
#define INDB	0100000	/* internal indirection flag */

int	buffers[2*256];
int	*buf[2]		{&buffers[0], &buffers[256]};

struct	filsys	sblk;
struct	inode	*ino;
struct	inode	inobuf[16];
int	lastinoblk	-1;
int	inoblkcng	0;

int	table[MAXBLK];	/* indirection table */

int	nleft;		/* number of blocks left to be allocated */
int	lastblk;
int	fd;
char	topt;
char	copt;
int	newfsize 0;	/* new file system size */
int	newisize 0;	/* new inode space size */


main(argc, argv)
char	**argv;
{
	extern int onintr();

	while(**++argv == '-') {
		--argc;
		switch(*++*argv) {
			case 'c': copt++; break;
			case 't': topt++; break;
			case 'i':
				newisize = atoi(*++argv);
				--argc;
				break;
			case 'f':
				newfsize = atoi(*++argv);
				--argc;
		}
	}
	if(argc != 2) {
		printf("Arg. count.\n");
		printf("Usage: sortfs [-c] [-t] [-i isize] [-f fsize] device\n");
		exit(1);
	}
	if((fd = open(*argv, topt? 0:2)) < 0) {
		printf("Can't open %s for %sing.\n", *argv,
			topt? "read":"writ");
		exit(1);
	}
	signal(1, 1);		/* hang-up */
	if(ttyn(0) != 'x')
		signal(2, onintr);	/* interrupt */
	else
		signal(2, 1);

	sync();
	bread(&sblk, 1);	/* read super block */
	if(newisize && newisize < sblk.s_isize) {
		printf("New isize < current isize.\n");
		exit(1);
	}
	if(newisize == 0)
		newisize = sblk.s_isize;
	if(newfsize == 0)
		newfsize = sblk.s_fsize;
	nleft = newfsize - newisize - 2;
	lastblk = newisize - 14;
	/* do the work: */
	phase1();
	if(topt) exit(0);
	phase2();
	phase3();
	phase4();
	sblk.s_isize = newisize;	/* update to the new sizes */
	sblk.s_fsize = newfsize;
	bwrite(&sblk, 1);	/* write super block */
	close(fd);
	exit(0);
}



onintr()
{
	register c;

	signal(2, 1);
	printf("\n\n\007Sure you want to quit? ");
	c = getchar();
	if(c == 'y') exit(0);
	if(c != '\n')
		while((c = getchar()) != '\n' && c != '\0') ;
	signal(2, onintr);
}



phase1()
{
	register int i,j,b;
	int	*p;

	if(sblk.s_fsize > MAXBLK || newfsize > MAXBLK) {
		printf("Too many blocks on device.\n");
		exit(1);
	}
	printf("\007Begin phase 1:  Build table.\n");
	for(i = 0; i < MAXBLK; ) table[i++] = 0;
	for(j = 1; j <= (sblk.s_isize * 16); ++j) {
		iread(j);
		if(inoalloc())
			if(ino->i_mode & ILARG) {
				/* large file */
				for(i = 0; i < 7; ++i)
					if(b = ino->i_addr[i]) {
						mark(&ino->i_addr[i], 1);
						table[b] =| INDB;
						bread(buf[0], b);
						mark(buf[0], 256);
					}
				/* huge file */
				if(b = ino->i_addr[7]) {
					mark(&ino->i_addr[7], 1);
					table[b] =| INDB;
					bread(buf[1], b);
					for(p = buf[1]; p < buf[1] + 256; ++p)
						if(b = *p) {
							mark(p, 1);
							table[b] =| INDB;
							bread(buf[0], b);
							mark(buf[0], 256);
						}
				}
			} else {
				/* small file */
				mark(&ino->i_addr[0], 8);
			}
	}
	if(topt) {
		i = "/tmp/table";
		unlink(i);
		i = creat(i, 0644);
		write(i, table, sblk.s_fsize * 2);
		close(i);
	}
	/* check for file system overflow */
	if(nleft < 0) {
		printf("Sortfs aborted - file system unchanged.\n");
		printf("New i or f size leaves too few blocks for files.\n");
		exit(1);
	}
}



mark(p, num)
int	*p;
{
	register int *rp,*lp;

	lp = p + num;
	for(rp = p; rp < lp; ++rp)
		if(badblock(*rp))
			table[*rp] = getblk();
}



getblk()
{
	register int bno,sec,trk;

	if(nleft-- <= 0) return(0);
loop:
	bno = lastblk++;
	if(!copt) {	/* interleave */
		sec = bno % 12;
		trk = bno / 12;
		bno =- sec;
		sec = ((sec * 3) + (sec / 4) + (trk * 3)) % 12;
		bno =+ sec;
	}
	if(bno < newisize + 2 || bno >= newfsize) goto loop;
	return(bno);
}



phase2()
{
	extern write();
	register int i,*p;

	printf("\007Begin phase 2:  Update block pointers.\n");
	/* relocate inode pointers */
	for(i = 1; i <= (sblk.s_isize * 16); ++i) {
		iread(i);
		if(inoalloc()) {
			for(p = &ino->i_addr[0]; p < &ino->i_addr[8]; ++p)
				*p = table[*p] & ~INDB;
			iwrite(i);
		}
	}
	/* relocate indirection blocks */
	for(i = sblk.s_isize + 2; i < sblk.s_fsize; ++i)
		if(table[i] & INDB) {
			table[i] =& ~INDB;
			bread(buf[0], i);
			for(p = buf[0]; p < buf[0] + 256; ++p)
				*p = table[*p] & ~INDB;
			bwrite(buf[0], i);
		}
	if(inoblkcng)
		bio(inobuf, lastinoblk, write);
}



phase3()
{
	register int to,from,k;
	int	i;

	printf("\007Begin phase 3:  Move blocks.\n");
	k = 0;
	for(i = sblk.s_isize + 2; i < sblk.s_fsize; ++i)
		if(table[i] && table[i] != i) {
			from = i;
			to = table[i];
			bread(buf[k], from);
			while(table[to]) {
				bread(buf[1-k], to);
				bwrite(buf[k], to);
				table[from] = 0;
				from = to;
				to = table[to];
				k = 1 - k;
			}
			bwrite(buf[k], to);
			table[from] = 0;
		}
}



phase4()
{
	register int bno,i,*p;
	int	cur;

	printf("\007Begin phase 4:  Build free-space list.\n");
	i = (nleft + 1) % 100;
	if(i == 0) i = 100;
	for(p = &sblk.s_free[i]; --p >= &sblk.s_free[0]; )
		*p = getblk();
	sblk.s_nfree = i;
	cur = sblk.s_free[0];
	bno = getblk();
	p = buf[0];
	while(bno) {
		for(i = 100; i > 0; --i) {
			p[i] = bno;
			bno = getblk();
		}
		p[0] = 100;
		bwrite(p, cur);
		cur = p[1];
	}
	sblk.s_flock = sblk.s_ilock = sblk.s_fmod = 0;
}



inoalloc()
{
	register int m;

	m = ino->i_mode & (IALLOC | IFMT);
	return(m == IALLOC || m == (IALLOC | IFDIR));
}



badblock(bno)
{
	if(bno == 0) return(0);
	if(bno < sblk.s_isize + 2 || bno >= sblk.s_fsize) {
		printf("Bad block encountered - %d\n", bno);
		bno = 0;
	}
	return(bno);
}



bread(buff, bno)
{
	extern read();
	bio(buff, bno, read);
}



bwrite(buff, bno)
{
	extern write();
	bio(buff, bno, write);
}



iread(inum)
{
	inodeio(inum, 0);
}



iwrite(inum)
{
	inodeio(inum, 1);
}



inodeio(inum, rw)
{
	extern read(), write();
	register int bno;

	bno = (inum + 31) / 16;
	if(bno != lastinoblk) {
		if(inoblkcng)
			bio(inobuf, lastinoblk, write);
		bio(inobuf, bno, read);
		lastinoblk = bno;
		inoblkcng = 0;
	}
	ino = &inobuf[(inum - 1) % 16];
	inoblkcng =| rw;
}



bio(buff, bno, iofcn)
int (*iofcn)();
{
	extern write();
	register int i;

	if(seek(fd, bno, 3) < 0) {
		printf("Seek error.\n");
		exit(1);
	}
	i = (*iofcn)(fd, buff, 512);
	if(i != 512) {
		printf("%s error; bn=%d, cnt=%d\n", (iofcn==write)? "Write":"Read",
			bno, i);
		exit(1);
	}
}
