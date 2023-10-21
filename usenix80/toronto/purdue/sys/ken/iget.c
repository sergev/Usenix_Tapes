#
#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../inode.h"
#include "../filsys.h"
#include "../conf.h"
#include "../buf.h"
#include "../proc.h"
#include "../file.h"

/*
 * Look up an inode by device,inumber.
 * If it is in core (in the inode structure),
 * honor the locking protocol.
 * If it is not in core, read it in from the
 * specified device.
 * If the inode is mounted on, perform
 * the indicated indirection.
 * In all cases, a pointer to a locked
 * inode structure is returned.
 *
 * printf warning: no inodes -- if the inode
 *	structure is full
 * panic: no imt -- if the mounted file
 *	system is not in the mount table.
 *	"cannot happen"
 */
iget(dev, ino)
{
	register struct inode *p;
	register *ip2;
	int *ip1;
	register struct mount *ip;
	int noinodes;
	noinodes = 0;

loop:
	ip = NULL;
	for(p = &inode[0]; p < &inode[NINODE]; p++) {
		if(dev==p->i_dev && ino==p->i_number) {
			if((p->i_flag&ILOCK) != 0) {
				p->i_flag =| IWANT;
				sleep(p, PINOD);
				goto loop;
			}
			if((p->i_flag&IMOUNT) != 0) {
				for(ip = &mount[0]; ip < &mount[NMOUNT]; ip++)
				if(ip->m_inodp == p) {
					dev = ip->m_dev;
					ino = ROOTINO;
					goto loop;
				}
				panic("no imt");
			}
			p->i_count++;
			p->i_flag =| ILOCK;
			return(p);
		}
		if(p->i_count == 0)
			if(ip == NULL || ip->i_lrt > p->i_lrt)
				ip = p;
	}
	if((p=ip) == NULL) {
		if (noinodes++ == 0)
			printf("WAIT incore inode table space\n");
		sleep(&lbolt, PFNT);
# ifdef XBUF
		*ka5 = praddr;
# endif
		if (u.u_procp->p_sig == SIGKIL) {
			u.u_error = ENFILE;
			return(NULL);
		}
		goto loop;
	}
	p->i_dev = dev;
	p->i_number = ino;
	p->i_flag = ILOCK;
	p->i_count++;
	p->i_lastr = -1;
	/*
	 * check for inode inrange
	 */
	icheck(p);
	ip = bread(dev, ldiv(ino+31,16));
	/*
	 * Check I/O errors
	 */
	if (ip->b_flags&B_ERROR) {
		brelse(ip);
		iput(p);
		return(NULL);
	}
# ifndef XBUF
	ip1 = ip->b_addr + 32*lrem(ino+31, 16);
# endif
# ifdef XBUF
	*ka5 = ip->b_par;
	ip1 = &b + 32*lrem(ino+31, 16);
# endif
	ip2 = &p->i_mode;
	while(ip2 < &p->i_addr[8])
		*ip2++ = *ip1++;
	brelse(ip);
	return(p);
}

/*
 * Decrement reference count of
 * an inode structure.
 * On the last reference,
 * write the inode out and if necessary,
 * truncate and deallocate the file.
 */
iput(p)
struct inode *p;
{
	register *rp;

	register struct file *fp;	/* DEBUG */

	rp = p;
	icheck(rp);	/* check for valid inode number */
	if(rp->i_count == 1) {
		rp->i_flag =| ILOCK;
		if(rp->i_nlink <= 0) {
			itrunc(rp);
			rp->i_mode = 0;
			ifree(rp->i_dev, rp->i_number);
		}
		/*
		 * DEBUG - check to see if going to drop inode with
		 * file struct's pointing to it. --ghg 12/16/78.
		 */
		for (fp = &file[0]; fp < &file[NFILE]; fp++)
			if (rp == fp->f_inode && fp->f_count) {
				printf("ip: 0%o, fp: 0%o\n", rp, fp);
				panic("Dead inode");
			}
		/*  END DEBUG */

		iupdat(rp, time);
		prele(rp);
		rp->i_flag = 0;
		rp->i_lrt = time[1];
	}
	rp->i_count--;
	prele(rp);
}

/*
 * Check accessed and update flags on
 * an inode structure.
 * If either is on, update the inode
 * with the corresponding dates
 * set to the argument tm.
 */
iupdat(p, tm)
int *p;
int *tm;
{
	register *ip1, *ip2, *rp;
	int *bp, i;

	rp = p;
	if((rp->i_flag&(IUPD|IACC)) != 0) {
		if(getfs(rp->i_dev)->s_ronly)
			return;
		i = rp->i_number+31;
		bp = bread(rp->i_dev, ldiv(i,16));
# ifndef XBUF
		ip1 = bp->b_addr + 32*lrem(i, 16);
# endif
# ifdef XBUF
		*ka5 = bp->b_par;
		ip1 = &b + 32*lrem(i, 16);
# endif
		ip2 = &rp->i_mode;
		while(ip2 < &rp->i_addr[8])
			*ip1++ = *ip2++;
		if(rp->i_flag&IACC) {
			*ip1++ = time[0];
			*ip1++ = time[1];
		} else
			ip1 =+ 2;
		if(rp->i_flag&IUPD) {
			*ip1++ = *tm++;
			*ip1++ = *tm;
		}
		rp->i_flag =& ~(IACC|IUPD);
		if (rp->i_size1 || rp->i_size0 || (rp->i_mode == 0))
			bdwrite(bp);
		else
			bwrite(bp);
	}
}

/*
 * Free all the disk blocks associated
 * with the specified inode structure.
 * The blocks of the file are removed
 * in reverse order. This FILO
 * algorithm will tend to maintain
 * a contiguous free list much longer
 * than FIFO.
 *
 * If there are any disk blocks assigned to this inode,
 * then make a temp copy of it with blocks zeroed out,
 * write this temp copy to disk and then proceed to release
 * storage assigned to real inode.  This is crash insurance
 * which greatly reduces the problem of itrunc() putting
 * indirect blocks back onto the free list, having somebody
 * else snarf them and write them to disk before the copy of
 * the inode which is being truncated makes it to the disk.
 * This change and the one in bmap() handling indirect files
 * should (hopefully) reduce most crash recoveries to just
 * "dups in free" and dangling directory entries.
 *
 * Itrunc() gets called from iput(), creat(), and core()
 * and maybe a few others.  This crash insurance costs a
 * "hard" disk write (bwrite) in itrunc() and the inode must
 * still get written out again when iput() finishes him off.
 * Iupdat() uses hard write (bwrite) when i_mode is non zero
 * which is the case he was called from itrunc(), but uses
 * a delayed write (bdwrite) if the mode is 0 as it is when
 * iput() calles him.  If the system crashes before bdwrite,
 * you have allocated inode, but zero link count, and no
 * storage assigned, so much easier to track down during
 * recovery than a trashed up indirect block.  I really hate
 * to slow the system down with extra disk writes, but the
 * flakey hardware demands it.
 * --ghg 06/04/78.
 */
itrunc(ip)
int *ip;
{
	register *rp, *bp, *cp;
	int *dp, *ep;
	struct inode iinode;

	rp = ip;
	if((rp->i_mode&(IFCHR&IFBLK)) != 0)
		return;
	rp->i_flag =| IUPD;
	if (rp->i_size1 || rp->i_size0 || rp->i_addr[0]) {
		rp->i_size0 = 0;
		rp->i_size1 = 0;
		for (cp = &iinode; cp < &iinode.i_addr[0]; *cp++ = *rp++);
		rp = ip;
		while (cp < &iinode.i_addr[8])
			*cp++ = 0;
		cp = &iinode;
		iupdat(cp, time);
	}
# ifndef XBUF
	for(ip = &rp->i_addr[7]; ip >= &rp->i_addr[0]; ip--)
	if(*ip) {
		if((rp->i_mode&ILARG) != 0) {
			bp = bread(rp->i_dev, *ip);
			for(cp = bp->b_addr+510; cp >= bp->b_addr; cp--)
			if(*cp) {
				if(ip == &rp->i_addr[7]) {
					dp = bread(rp->i_dev, *cp);
					for(ep = dp->b_addr+510; ep >= dp->b_addr; ep--)
					if(*ep)
						free(rp->i_dev, *ep);
					brelse(dp);
				}
				free(rp->i_dev, *cp);
			}
			brelse(bp);
		}
		free(rp->i_dev, *ip);
		*ip = 0;
	}
# endif
# ifdef XBUF
	for(ip = &rp->i_addr[7]; ip >= &rp->i_addr[0]; ip--)
	if(*ip) {
		if((rp->i_mode&ILARG) != 0) {
			bp = bread(rp->i_dev, *ip);
			*ka5 = bp->b_par;
			for(cp = &b+510; cp >= &b; cp--)
			if(*cp) {
				if(ip == &rp->i_addr[7]) {
					dp = bread(rp->i_dev, *cp);
					*ka5 = dp->b_par;
					for(ep = &b+510; ep >= &b; ep--)
					if(*ep) {
						free(rp->i_dev, *ep);
						*ka5 = dp->b_par;
					}
					brelse(dp);
					*ka5 = bp->b_par;
				}
				free(rp->i_dev, *cp);
				*ka5 = bp->b_par;
			}
			brelse(bp);
		}
		free(rp->i_dev, *ip);
		*ip = 0;
	}
# endif
	rp->i_mode =& ~ILARG;
}

/*
 * Make a new file.
 */
maknode(mode)
{
	register *ip;

	ip = ialloc(u.u_pdir->i_dev);
	if (ip==NULL)
		return(NULL);
	ip->i_flag =| IACC|IUPD;
	ip->i_mode = (mode|IALLOC)&0177707; /* set group bits to zero */
	ip->i_nlink = 1;
	ip->i_uid0 = u.u_uid;
	ip->i_uid1 = (u.u_uid>>8);
	wdir(ip);
	return(ip);
}

/*
 * Write a directory entry with
 * parameters left as side effects
 * to a call to namei.
 */
wdir(ip)
int *ip;
{
	register char *cp1, *cp2;

	u.u_dent.u_ino = ip->i_number;
	cp1 = &u.u_dent.u_name[0];
	for(cp2 = &u.u_dbuf[0]; cp2 < &u.u_dbuf[DIRSIZ];)
		*cp1++ = *cp2++;
	u.u_count = DIRSIZ+2;
	u.u_segflg = 1;
	u.u_base = &u.u_dent;
	icheck(ip);	/* icheck for valid inode number */
	writei(u.u_pdir);
	iput(u.u_pdir);
}

/*
 * Check for in bounds inode number - crash system if bad
 */
icheck(ip)
struct inode *ip;
{

	register struct inode *rip;
	register struct filsys *fp;

	rip = ip;
	fp = getfs(rip->i_dev);
	if (rip->i_number > (fp->s_isize*16)) {
		prdev("Bad inode", rip->i_dev);
		printf("inode: %d, ip:0%o, fp0%o\n", rip->i_number,
		rip, fp);
		halt();
	}
}
