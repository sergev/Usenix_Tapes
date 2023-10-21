#
/*
 */

#include "../param.h"
#include "../systm.h"
#include "../filsys.h"
#include "../conf.h"
#include "../buf.h"
#include "../inode.h"
#include "../user.h"
#include "../proc.h"

/*
 * iinit is called once (from main)
 * very early in initialization.
 * It reads the root's super block
 * and initializes the current date
 * from the last modified date.
 *
 * panic: iinit -- cannot read the super
 * block. Usually because of an IO error.
 */
iinit()
{
	register *cp, *bp;

	(*bdevsw[rootdev.d_major].d_open)(rootdev, 1);
	bp = bread(rootdev, 1);
	cp = getblk(NODEV);
	if(u.u_error)
		panic("iinit");
# ifndef XBUF
	bcopy(bp->b_addr, cp->b_addr, 256);
# endif
# ifdef XBUF
	*ka5 = bp->b_par;
	bcopy(&b, cp->b_addr, 256);
# endif
	brelse(bp);
	mount[0].m_bufp = cp;
	mount[0].m_dev = rootdev;
	cp = cp->b_addr;
	cp->s_flock = 0;
	cp->s_ilock = 0;
	cp->s_ronly = 0;
	time[0] = cp->s_time[0];
	time[1] = cp->s_time[1];
}

/*
 * alloc will obtain the next available
 * free disk block from the free list of
 * the specified device.
 * The super block has up to 100 remembered
 * free blocks; the last of these is read to
 * obtain 100 more . . .
 *
 * WAIT SPACE on dev x/y -- when
 * the free list is exhausted.
 */
alloc(dev)
{
	int bno;
	register *bp, *ip, *fp;
	int devfull;

	devfull = 0;
loop:
	fp = getfs(dev);
	while(fp->s_flock)
		sleep(&fp->s_flock, PINOD);
	do {
		if(fp->s_nfree <= 0)
			goto nospace;
		bno = fp->s_free[--fp->s_nfree];
		if(bno == 0)
			goto nospace;
	} while (badblock(fp, bno, dev));
	if(fp->s_nfree <= 0) {
		fp->s_flock++;
		bp = bread(dev, bno);
# ifndef XBUF
		ip = bp->b_addr;
# endif
# ifdef XBUF
		*ka5 = bp->b_par;
		ip = &b;
# endif
		fp->s_nfree = *ip++;
		bcopy(ip, fp->s_free, 100);
		brelse(bp);
		fp->s_flock = 0;
		wakeup(&fp->s_flock);
	}
	bp = getblk(dev, bno);
	clrbuf(bp);
	fp->s_fmod = 1;
	fp->s_tfree--;
	return(bp);

nospace:
/*
	prdev("no space", dev);
	u.u_error = ENOSPC;
	return(NULL);
*/
/* changed to wait for more space ... 11/19/77 --ghg */
	fp->s_nfree = 0;
	if (devfull++ == 0)  {
		prdev("WAIT SPACE", dev);
#ifdef XBUF
		*ka5 = praddr;
#endif
		printf("uid:%d pid:%d\n", u.u_uid, u.u_procp->p_pid);
	}
	sleep(&lbolt, PFNT);
# ifdef XBUF
	*ka5 = praddr;
# endif
	if (u.u_procp->p_sig == SIGKIL) {
		u.u_error = ENOSPC;
		return(NULL);
	}
	goto loop;
/* .................................................. */
}

/*
 * place the specified disk block
 * back on the free list of the
 * specified device.
 */
free(dev, bno)
{
	register *fp, *bp, *ip;

	fp = getfs(dev);
	fp->s_fmod = 1;
	while(fp->s_flock)
		sleep(&fp->s_flock, PINOD);
	if (badblock(fp, bno, dev))
		return;
	if(fp->s_nfree <= 0) {
		fp->s_nfree = 1;
		fp->s_free[0] = 0;
	}
	if(fp->s_nfree >= 100) {
		fp->s_flock++;
		bp = getblk(dev, bno);
# ifndef XBUF
		ip = bp->b_addr;
# endif
# ifdef XBUF
		*ka5 = bp->b_par;
		ip = &b;
# endif
		*ip++ = fp->s_nfree;
		bcopy(fp->s_free, ip, 100);
		fp->s_nfree = 0;
		bwrite(bp);
		fp->s_flock = 0;
		wakeup(&fp->s_flock);
	}
	fp->s_free[fp->s_nfree++] = bno;
	fp->s_fmod = 1;
	fp->s_tfree++;
}

/*
 * Check that a block number is in the
 * range between the I list and the size
 * of the device.
 * This is used mainly to check that a
 * garbage file system has not been mounted.
 *
 * bad block on dev x/y -- not in range
 */
badblock(afp, abn, dev)
{
	register struct filsys *fp;
	register char *bn;

	fp = afp;
	bn = abn;
	if (bn < fp->s_isize+2 || bn >= fp->s_fsize) {
		prdev("bad block", dev);
		halt();
		return(1);
	}
	return(0);
}

/*
 * Allocate an unused I node
 * on the specified device.
 * Used with file creation.
 * The algorithm keeps up to
 * 100 spare I nodes in the
 * super block. When this runs out,
 * a linear search through the
 * I list is instituted to pick
 * up 100 more.
 */
ialloc(dev)
{
	register *fp, *bp, *ip;
	int i, j, k, ino;
	int noinode;

	noinode = 0;
loop0:
	fp = getfs(dev);
	while(fp->s_ilock)
		sleep(&fp->s_ilock, PINOD);
loop:
	if(fp->s_ninode > 0) {
		ino = fp->s_inode[--fp->s_ninode];
		ip = iget(dev, ino);
		if (ip==NULL)
			return(NULL);
		if(ip->i_mode == 0) {
			for(bp = &ip->i_mode; bp < &ip->i_addr[8];)
				*bp++ = 0;
			fp->s_fmod = 1;
			fp->s_tinode--;
			return(ip);
		}
		/*
		 * Inode was allocated after all.
		 * Look some more.
		 */
		iput(ip);
		goto loop;
	}
	fp->s_ilock++;
	ino = 0;
	for(i=0; i<fp->s_isize; i++) {
		bp = bread(dev, i+2);
# ifndef XBUF
		ip = bp->b_addr;
# endif
# ifdef XBUF
		*ka5 = bp->b_par;
		ip = &b;
# endif
		for(j=0; j<256; j=+16) {
			ino++;
			if(ip[j] != 0)
				continue;
			for(k=0; k<NINODE; k++)
			if(dev==inode[k].i_dev && ino==inode[k].i_number)
				goto cont;
			fp->s_inode[fp->s_ninode++] = ino;
			if(fp->s_ninode >= 100)
				break;
		cont:;
		}
		brelse(bp);
		if(fp->s_ninode >= 100)
			break;
	}
	fp->s_ilock = 0;
	wakeup(&fp->s_ilock);
	if (fp->s_ninode > 0)
		goto loop;
/* Changed to wait for more inodes  11/19/77 -- ghg 
	prdev("Out of inodes", dev);
	u.u_error = ENOSPC;
	return(NULL);
*/
	if (noinode++ == 0)
		prdev("WAIT INODES", dev);
	sleep(&lbolt, PFNT);
# ifdef XBUF
	*ka5 = praddr;
# endif
	if (u.u_procp->p_sig == SIGKIL) {
		u.u_error = ENOSPC;
		return(NULL);
	}
	goto loop0;
}

/*
 * Free the specified I node
 * on the specified device.
 * The algorithm stores up
 * to 100 I nodes in the super
 * block and throws away any more.
 */
ifree(dev, ino)
{
	register *fp;

	fp = getfs(dev);
	fp->s_tinode++;
	if(fp->s_ilock)
		return;
	if(fp->s_ninode >= 100)
		return;
	fp->s_inode[fp->s_ninode++] = ino;
	fp->s_fmod = 1;
}

/*
 * getfs maps a device number into
 * a pointer to the incore super
 * block.
 * The algorithm is a linear
 * search through the mount table.
 * A consistency check of the
 * in core free-block and i-node
 * counts.
 *
 * bad count on dev x/y -- the count
 *	check failed. At this point, all
 *	the counts are zeroed which will
 *	almost certainly lead to "no space"
 *	diagnostic
 * panic: no fs -- the device is not mounted.
 *	this "cannot happen"
 */
getfs(dev)
{
	register struct mount *p;
	register char *n1, *n2;

	for(p = &mount[0]; p < &mount[NMOUNT]; p++)
	if(p->m_bufp != NULL && p->m_dev == dev) {
		p = p->m_bufp->b_addr;
		n1 = p->s_nfree;
		n2 = p->s_ninode;
		if(n1 > 100 || n2 > 100) {
			prdev("bad count", dev);
			halt();
			p->s_nfree = 0;
			p->s_ninode = 0;
		}
		return(p);
	}
	panic("no fs");
}

/*
 * update is the internal name of
 * 'sync'. It goes through the disk
 * queues to initiate sandbagged IO;
 * goes through the I nodes to write
 * modified nodes; and it goes through
 * the mount table to initiate modified
 * super blocks.
 */
update()
{
	register struct inode *ip;
	register struct mount *mp;
	register *bp;

	if(updlock)
		return;
	updlock++;
	for(mp = &mount[0]; mp < &mount[NMOUNT]; mp++)
		if(mp->m_bufp != NULL) {
			ip = mp->m_bufp->b_addr;
			if(ip->s_fmod==0 || ip->s_ilock!=0 ||
			   ip->s_flock!=0 || ip->s_ronly!=0)
				continue;
			bp = getblk(mp->m_dev, 1);
			ip->s_fmod = 0;
			ip->s_time[0] = time[0];
			ip->s_time[1] = time[1];
# ifndef XBUF
			bcopy(ip, bp->b_addr, 256);
# endif
# ifdef XBUF
			*ka5 = bp->b_par;
			bcopy(ip, &b, 256);
# endif
			bwrite(bp);
		}
	for(ip = &inode[0]; ip < &inode[NINODE]; ip++)
		if((ip->i_flag&ILOCK) == 0 && ip->i_count) { 
			ip->i_flag =| ILOCK;
			ip->i_count++;
			iupdat(ip, time);
			iput(ip);
		}
	updlock = 0;
	bflush(NODEV);
}
