#
/*
 * modified 7/26/77, G Goble, return EFBIG error if files is over
 * "maxblk" blocks long and not super user.  Helps avoid run away
 * programs in disk output loop.  Super user limit is still 32768.
 * Initial value of maxblk is value of "MAXBLK", a define in param.h.
 */

#include "../param.h"
#include "../conf.h"
#include "../inode.h"
#include "../user.h"
#include "../buf.h"
#include "../systm.h"

/*
 * Bmap defines the structure of file system storage
 * by returning the physical block number on a device given the
 * inode and the logical block number in a file.
 * When convenient, it also leaves the physical
 * block number of the next block of the file in rablock
 * for use in read-ahead.
 *
 * Changed to write indirect blocks via bwrite instead of bdwrite
 * on initial allocation.  In general, filesystem recoveries
 * after crashes, are made less difficult since the chances of
 * having a garbaged indirect block (caused by indir block not
 * getting written at all) are greatly reduced.
 * Moved code setting "rablock" up a few lines since it is a little
 * dangerous to play with a buffer's contents after it it brelse'd.
 * Also fixed a couple of places where IUPD was not getting set
 * where it should have been.
 * --ghg 04/16/78.
 */
bmap(ip, bn)
struct inode *ip;
int bn;
{
	register *bp, *bap, nb;
	int *nbp, d, i;

	d = ip->i_dev;
/*	code to limit non super-user files to 2048 blocks */
/*	-ghg 3/13/77 */
	if (u.u_uid > MAXBLKU)
		if(bn > maxblk) {
			u.u_error = EFBIG;
			return(0);
		}
	else
		if (u.u_uid)
			if (bn > maxblk2) {
				u.u_error = EFBIG;
				return(0);
			}
/* ........................................................ */
	if(bn & ~077777) {
		u.u_error = EFBIG;
		return(0);
	}

	if((ip->i_mode&ILARG) == 0) {

		/*
		 * small file algorithm
		 */

		if((bn & ~7) != 0) {

			/*
			 * convert small to large
			 */

			if ((bp = alloc(d)) == NULL)
				return(NULL);
# ifndef XBUF
			bap = bp->b_addr;
# endif
# ifdef XBUF
			*ka5 = bp->b_par;
			bap = &b;
# endif
			for(i=0; i<8; i++) {
				*bap++ = ip->i_addr[i];
				ip->i_addr[i] = 0;
			}
			ip->i_addr[0] = bp->b_blkno;
			bwrite(bp);
			ip->i_mode =| ILARG;
			ip->i_flag =| IUPD;
			goto large;
		}
		nb = ip->i_addr[bn];
		if(nb == 0 && (bp = alloc(d)) != NULL) {
			bdwrite(bp);
			nb = bp->b_blkno;
			ip->i_addr[bn] = nb;
			ip->i_flag =| IUPD;
		}
		rablock = 0;
		if (bn<7)
			rablock = ip->i_addr[bn+1];
		return(nb);
	}

	/*
	 * large file algorithm
	 */

    large:
	i = bn>>8;
	if(bn & 0174000)
		i = 7;
	if((nb=ip->i_addr[i]) == 0) {
		if ((bp = alloc(d)) == NULL)
			return(NULL);
		ip->i_addr[i] = bp->b_blkno;
		ip->i_flag =| IUPD;
	} else
		bp = bread(d, nb);
# ifndef XBUF
	bap = bp->b_addr;
# endif
# ifdef XBUF
	*ka5 = bp->b_par;
	bap = &b;
# endif

	/*
	 * "huge" fetch of double indirect block
	 */

	if(i == 7) {
		i = ((bn>>8) & 0377) - 7;
		if((nb=bap[i]) == 0) {
			if((nbp = alloc(d)) == NULL) {
				brelse(bp);
				return(NULL);
			}
# ifdef XBUF
			*ka5 = bp->b_par;
# endif
			bap[i] = nbp->b_blkno;
			if (bap[1])
				bdwrite(bp);
			else
				bwrite(bp);	/* crash insurance */
		} else {
			brelse(bp);
			nbp = bread(d, nb);
		}
		bp = nbp;
# ifndef XBUF
		bap = bp->b_addr;
# endif
# ifdef XBUF
		*ka5 = bp->b_par;
		bap = &b;
# endif
	}

	/*
	 * normal indirect fetch
	 */

	i = bn & 0377;
	rablock = 0;
	if(i < 255)
		rablock = bap[i+1];
	if((nb=bap[i]) == 0 && (nbp = alloc(d)) != NULL) {
# ifdef XBUF
		*ka5 = bp->b_par;
# endif
		nb = nbp->b_blkno;
		bap[i] = nb;
		bdwrite(nbp);
		if (bap[1])
			bdwrite(bp);
		else
			bwrite(bp);	/* crash insurance */
	} else
		brelse(bp);
	return(nb);
}

/*
 * Pass back  c  to the user at his location u_base;
 * update u_base, u_count, and u_offset.  Return -1
 * on the last character of the user's read.
 * u_base is in the user address space unless u_segflg is set.
 */
passc(c)
char c;
{

	if(u.u_segflg)
		*u.u_base = c; else
		if(subyte(u.u_base, c) < 0) {
			u.u_error = EFAULT;
			return(-1);
		}
	u.u_count--;
	if(++u.u_offset[1] == 0)
		u.u_offset[0]++;
	u.u_base++;
	return(u.u_count == 0? -1: 0);
}

/*
 * Pick up and return the next character from the user's
 * write call at location u_base;
 * update u_base, u_count, and u_offset.  Return -1
 * when u_count is exhausted.  u_base is in the user's
 * address space unless u_segflg is set.
 */
cpass()
{
	register c;

	if(u.u_count == 0)
		return(-1);
	if(u.u_segflg)
		c = *u.u_base; else
		if((c=fubyte(u.u_base)) < 0) {
			u.u_error = EFAULT;
			return(-1);
		}
	u.u_count--;
	if(++u.u_offset[1] == 0)
		u.u_offset[0]++;
	u.u_base++;
	return(c&0377);
}

/*
 * Routine which sets a user error; placed in
 * illegal entries in the bdevsw and cdevsw tables.
 */
nodev()
{

	u.u_error = ENODEV;
}

/*
 * Null routine; placed in insignificant entries
 * in the bdevsw and cdevsw tables.
 */
nulldev()
{
}

/*
 * copy count words from from to to.
 */
bcopy(from, to, count)
int *from, *to;
{
	register *a, *bb, c;

	a = from;
	bb = to;
	c = count;
	do
		*bb++ = *a++;
	while(--c);
}
