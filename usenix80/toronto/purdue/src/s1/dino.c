#
/*
 * dino -- dump inodes 
 *
 *
 *  syntax:
 *
 *	dino [<device> [<low> [<high>]]]
 *
 *	<device>::=	name of device to dump inodes from
 *	<low>::=	first inode to dump
 *	<high>::=	last inode to dump
 *
 * semantics:
 *
 *	no args =>	dump all inodes from default device.
 *	1 arg   =>	dump all inodes from specified device.
 *	2 args  =>	dump inode # <low> from specified device
 *	3 args  =>	dump inodes <low> through <high> from specified device
 *
 *	if the bounds specified by <low> or <high> exceed those
 *	on the device, they are trunicated.
 *
 *	jjb 25 may 76
 */

#include "/usr/sys/ino.h"

struct{
	char d_minor;
	char d_major;
};
struct inode nodes[16];
struct{
	int	isize;
	int	fsize;
	int	junk[254];
} super_blk;

int	fd;
int	low	0;
int	high	0;
int	index	0;
int	blkno	-1;
char	*device	"/dev/hp0";

main(argc,argp)
char **argp;
{
	int i;

	switch(argc){
	case 4:
		high = atoi(argp[3]);
	case 3:
		low = atoi(argp[2]);
		if(low > high)
			high = low;
	case 2:
		device = argp[1];
	}
	if((fd = open(device,0)) < 0){
		printf("can't open %s\n",device);
		exit(1);
	}
	if(seek(fd,1,3) < 0 || read(fd,&super_blk,512) != 512){
		printf("can't read super block!\n");
		exit(2);
	}
	if(high > super_blk.isize*16 || high <= 0)
		high = super_blk.isize*16;
	if(high < low)
		low = high;
	if(low == 0)
		low = 1;
	printf("inodes from %s, %l thru %l (isize = %l)\n",
			device,low,high,super_blk.isize);
	for(i=low; i<high+1; i++)
		pino(getino(i),i);
	exit(0);
}

getino(n)
{
	register int blk;

	blk = (n+31)/16;
	index = (n-1)%16;
	if(blk != blkno){
/*
 *		printf(" reading blk %l for inode %l; index = %l\n",blk,n,index);
 */
		seek(fd,blk,3);
		read(fd,&nodes,512);
		blkno = blk;
	}
	return(&nodes[index]);
}


char *type[] { "file","char","dir","blk"};

pino(ino,num)
struct inode *ino;
{
	int fmt,mode,i,*p;

	printf("\n dump of inode # %l, blk %l, offset %l, pos %s\n",
		num, blkno, (index*32), locv(hmul(blkno,512), blkno*512 + (index*32)));
	mode = ino->i_mode;
	printf("%6o:\t",mode);
	if((mode&IALLOC) == 0){
		printf(" not allocated\n\n");
		return(0);
	}
	printf("type = ");
	fmt = (mode>>13)&03;
	if((mode&IFCHR) != 0)
		printf("%s device (%l,%l)",type[fmt],
						ino->i_addr[0].d_major,
						ino->i_addr[0].d_minor);
	else
		printf("%s",type[fmt]);
	if(mode&ILARG)
		if(ino->i_addr[7] != 0)
			printf(" (huge)");
		else
			printf(" (large)");
	printf(", prot = %3o",mode&0777);
	if(mode&ISUID)
		printf(", suid");
	if(mode&ISGID)
		printf(", sgid");
	if(mode&ISVTX)
		printf(", svtxt");
	printf("\n\t%l links, uid=%l, ",ino->i_nlink,
						ino->i_uid0&0377 | ino->i_uid1<<8);
	if((mode&IFCHR) == 0){
		printf("\n\tsize = %s",locv(ino->i_size0,ino->i_size1));
		printf("\n\tblocks used: ");
		for(i=0, p=ino->i_addr; i<8; i++)
			printf("%l, ",*p++);
	}
	printf("\n");
	ptime("atime",ino->i_atime);
	putchar(';');
	ptime("mtime",ino->i_mtime);
	printf("\n\n");
	return(1);
}

ptime(s,t)
char *s;
int *t;
{
	int *p;

	p = localtime(t);
	printf("\t%s: %l:%s%l, %l/%s%l/%l",s,p[2],(p[1]<10?"0":""),p[1],p[4],(p[3]<10?"0":""),p[3],p[5]);
}
