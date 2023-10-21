#include "../param.h"
#include "../user.h"
#include "../filsys.h"
#include "../buf.h"
#include "../cf.h"
#include "../inode.h"

cf_start(dev)			/* initialize data base for 
				   free list on UNIX device dev */
{
	register *sbp;

	while(cf_lock++ > 0)
		sleep(&cf_lock, 1);
	cf_depth = 0;
	cf_devn = dev;
	cf_sbp = getfs(dev);
	bcopy(&(cf_sbp->s_devc), &cf_sb, SBSIZ);
	sbp = &(cf_sb.chunksiz);
	if(*sbp++ && *sbp++ && *sbp++ && (cf_ipfl = iget(dev, *sbp++)) != NULL)
		getbuf(*sbp, NULL, 0);
	else {
		if(u.u_error == 0)
			u.u_error = ENOTBLK;
	}
	return(!u.u_error);
}

cf_relse()
{
	register i;
	bcopy(&cf_sb, &(cf_sbp->s_devc), SBSIZ);
	cf_getmax();
	iput(cf_ipfl);
	if(--cf_lock > 0) {
		cf_lock = 0;
		wakeup(&cf_lock);
	}
}

getbuf(blkno, father, rflag)
{
	register *cfp, depth, n;

	cfp = &cf_sb;
	if(blkno == -1)
		blkno = cfp->flsiz++;
	if(rflag) {	/* root is being split */
		cfp->rootlbn = blkno;
		depth = 0;
	}
	else 	if((depth = ++cf_depth) > MAXDEPTH) {
			--cf_depth;
			u.u_error = ECFS;
			prdev("cfs too deep", cf_devn);
		}
	if(u.u_error)
		return;
n = bmap(cf_ipfl, blkno); 
cf_buf[depth] = bread(cf_devn, n); 
bnodes[depth] = (cf_buf[depth])->b_addr; 
(bnodes[depth])->father = father;
	return(blkno);
}

putbuf()
{
	register struct buf **bufp;

	bufp = &(cf_buf[cf_depth--]);
	if(*bufp == NULL)
		return;
	bawrite(*bufp);
	*bufp = NULL;
}

getcdev(ip, blkio)	/* inode to character device number 
			   or if "blkio", block device number */
register struct inode *ip;
{
	register add0;

	add0 = ip->i_addr[0];
	if((ip->i_mode&IFMT) == IFCHR && add0 == ICFLAG) {
		add0 = ip->i_addr->i_cdev;
		if(blkio)
			add0 = ip->i_addr->i_bdev;
	}
	return(add0);
}
