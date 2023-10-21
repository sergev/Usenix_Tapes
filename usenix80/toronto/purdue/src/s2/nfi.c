#
#include "/usr2/sys/param.h"

#define UNIX "/deadstart/new-unixs"

int	kf	-1;
int	fd	-1;
int	fout;

struct {
	char name[8];
	int type;
	int value;
} nl[4] { { "_text" }, { "_buf" }, { "_ccount" } };

main()
{
	fout = dup(1);
	if((kf=open("/dev/kmem",0)) < 0) {
		printf("can't open /dev/kmem\n");
		endrun();
	}
	nlist(UNIX, &nl);
	prinod();
	prfile();
	prproc();
	prtext();
	prbuf();
	prclst();
	prswap();
	endrun();
}

prinod()
{
#include "/usr/sys/inode.h"
	register struct inode *ip;
	int count;

	count = 0;
	if((fd=open("/dev/inode",0)) < 0) {
		printf("Can't open /dev/inode\n");
		endrun();
	}
	read(fd, &inode, sizeof inode);
	for(ip = &inode; ip < &inode[NINODE]; ip++)
		if(ip->i_count)
			count++;
	printf("INODES = %d/%d  ", count, NINODE);
	close(fd);
}

prfile()
{
#include "/usr/sys/file.h"
	int count;
	register struct file *fp;

	count = 0;
	if((fd=open("/dev/file",0)) < 0) {
		printf("Can't open /dev/file\n");
		endrun();
	}
	read(fd, &file, sizeof file);
	for(fp = &file[0]; fp < &file[NFILE]; fp++)
		if(fp->f_count)
			count++;
	printf("FILES = %d/%d  ", count, NFILE);
	close(fd);
}

prclst()
{
	int count[2];

	if(nl[2].value != -1) {
		seek(kf, nl[2].value, 0);
		read(kf, &count, 4);
		printf("CCOUNT = %d/%d  CBLK = %d/%d  ",
		count[0], NCLIST*6, count[1], NCLIST);
	}
}

prswap()
{
struct map {
	char	*m_size;
	char	*m_addr;
};
	register struct map *bp;
	int swapmap[200];
	register int j;

	if((fd=open("/dev/swapmap",0)) < 0) {
		printf("Can't open /dev/swapmap\n");
		endrun();
	}
	read(fd, &swapmap, 400);
	j = 0;
	for(bp=swapmap; bp->m_size; bp++)
		j =+ bp->m_size;
	printf(" SWAP LEFT = %d\n", j);
	close(fd);
}

prtext()
{
#include "/u/sys/text.h"
	register struct text *tx;
	register cnt;

	cnt = 0;
	if(nl[0].value != -1) {
		seek(kf, nl[0].value, 0);
		read(kf, &text, sizeof text);
		for(tx = &text[0]; tx < &text[NTEXT]; tx++)
			if(tx -> x_count)
				cnt++;
		printf("TEXT = %d/%d  \n", cnt, NTEXT);
	}
}

prproc()
{
#include "/usr/sys/proc.h"
	register cnt;
	register struct proc *p;

	cnt = 0;
	if((fd=open("/dev/proc",0)) < 0) {
		printf("Can't open /dev/proc\n");
		endrun();
	}
	read(fd, &proc, sizeof proc);
	for(p = &proc[0]; p < &proc[NPROC]; p++)
		if(p->p_stat)
			cnt++;
	printf("PROCS = %d/%d  ", cnt, NPROC);
	close(fd);
}

prbuf()
{
#include "/u/sys/buf.h"
	register cnt;
	register struct buf *p;

	cnt = 0;
	if(nl[1].value != -1) {
		seek(kf, nl[1].value, 0);
		read(kf, &buf, sizeof buf);
		for(p = &buf[0]; p < &buf[NBUF]; p++)
			if((p->b_flags & (B_BUSY | B_XBUF)) == B_BUSY)
				cnt++;
		printf("BUFFERS = %d/%d  ", cnt, NBUF1);
	}
}

endrun()
{
	flush();
	exit();
}
