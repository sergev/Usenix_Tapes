#
/*
 * Floppy disk driver by Dale DeJager  CB x4502
 *
 * Modified 5/24/79 by Bill Shannon, CWRU
 *	sector computation modified to be RT-11 compatible.
 */

#include "../param.h"
#include "../buf.h"
#include "../user.h"
#include "../proc.h"
#include "../conf.h"

/* The following structure is used to access av_back as an integer */

struct {
	int	dummy0;
	struct	buf *dummy1;
	struct	buf *dummy2;
	struct	buf *dummy3;
	int	seccnt;
};

struct {
	int	rxcs;
	int	rxdb;
};

#define	RXADDR	0177170
#define	NSPB	4	/* Number of floppy sectors per unix block */
#define	NBPS	128	/* Number of bytes per floppy sector	*/
#define	NRXBLKS	2002/4	/* Number of unix blocks on a floppy */

struct	devtab	rxtab;

#define	GO	0000001	/* execute command function	*/
#define	UNIT1	0000020	/* unit select (drive 0=0, 1=1)	*/
#define	RXDONE	0000040	/* function complete		*/
#define	INTENB	0000100	/* interrupt enable		*/
#define	TRANREQ	0000200	/* transfer request (data only)	*/
#define	RXINIT	0040000	/* rx11 initialize		*/
#define	RXERROR	0100000	/* general error bit		*/

/*
 *	rx11 control function bits 1-3 of rxcs
 */
#define	FILL	0000000	/* fill buffer			*/
#define	EMPTY	0000002	/* empty buffer			*/
#define	WRITE	0000004	/* write buffer to disk		*/
#define	READ	0000006	/* read disk sector to buffer	*/
#define	RSTAT	0000012	/* read disk status		*/
#define	WSDD	0000014	/* write sector deleted data	*/
#define	RDERR	0000016	/* read error register function	*/


rxopen(dev, flag)
{
	if(dev.d_minor >= 2)
		u.u_error = ENXIO;
}

rxstrategy(bp)
register struct buf *bp;
{

	if(bp->b_blkno >= NRXBLKS) {
		if(bp->b_flags&B_READ)
			bp->b_resid = bp->b_wcount;
		else {
			bp->b_flags =| B_ERROR;
			bp->b_error = ENXIO;
		}
		iodone(bp);
		return;
	}
	bp->av_forw = 0;
	bp->seccnt = 0;	/* seccnt has special meaning: see below */
	spl5();
	if(rxtab.d_actf == 0)
		rxtab.d_actf = bp;
	else
		rxtab.d_actl->av_forw = bp;
	rxtab.d_actl = bp;
	if(rxtab.d_active == 0)
		rxstart();
	spl0();
}

rxstart()
{
	register int *ptcs, *ptdb;
	register char *ptbf;
	struct buf *bp;
	int track, sector, unit;

	if((bp = rxtab.d_actf) == 0) {
		rxtab.d_active = 0;
		return;
	}

	rxtab.d_active = 1;
	unit = bp->b_dev.d_minor ? UNIT1 : 0;
	ptcs = &RXADDR->rxcs;
	ptdb = &RXADDR->rxdb;
	/*
	 * seccnt is actually the number of 128 byte floppy blocks of
	 * the current unix disk buffer which have been processed.
	 */
	ptbf = bp->b_addr + NBPS*bp->seccnt;

	rxfactr(NSPB*bp->b_blkno + bp->seccnt, &sector, &track);

	if(bp->b_flags&B_READ)
		*ptcs = READ|GO|INTENB|unit;
	else {
		spl0();
		*ptcs = FILL|GO;
		do {
			while(ptcs->lobyte < 0)
				ptdb->lobyte = *ptbf++;
		} while(ptcs->lobyte <= 0);
		spl5();
		*ptcs = WRITE|GO|INTENB|unit;
	}
	while(ptcs->lobyte > 0);
	ptdb->lobyte = sector;
	while(ptcs->lobyte > 0);
	ptdb->lobyte = track;
}

rxintr() {
	register int *ptcs, *ptdb;
	register char *ptbf;
	struct buf *bp;

	if((bp = rxtab.d_actf) == 0)
		return;

	ptcs = &RXADDR->rxcs;
	ptdb = &RXADDR->rxdb;
	ptbf = bp->b_addr + NBPS*bp->seccnt;

	if(*ptcs < 0) {
		*ptcs = RXINIT;
		*ptcs = INTENB;
		if(rxtab.d_errcnt++ > 8) {
			bp->b_flags =| B_ERROR;
			goto done;
		}
		rxtab.d_active = 2;
		return;
	}
	*ptcs = NULL;
	if(rxtab.d_active == 2)
		goto recall;
	if(bp->b_flags&B_READ) {
		spl0();
		*ptcs = EMPTY|GO;
		do {
			while(ptcs->lobyte < 0)
				*ptbf++ = ptdb->lobyte;
			} while(ptcs->lobyte <= 0);
		spl5();
	}
	if(++bp->seccnt >= NSPB) {
	    done:
		rxtab.d_errcnt = 0;
		rxtab.d_actf = bp->av_forw;
		iodone(bp);
	}
    recall:
	rxstart();
}

/*
 *	rxfactr -- calculates the physical sector and physical
 *	track on the disk for a given logical sector.
 *	call:
 *		rxfactr(logical_sector,&p_sector,&p_track);
 *	the logical sector number (0 - 2001) is converted
 *	to a physical sector number (1 - 26) and a physical
 *	track number (0 - 76).
 *	the logical sectors specify physical sectors that
 *	are interleaved with a factor of 2. thus the sectors
 *	are read in the following order for increasing
 *	logical sector numbers (1,3, ... 23,25,2,4, ... 24,26)
 *	There is also a 6 sector slew between tracks.
 *	Logical sectors start at track 1, sector 1; go to
 *	track 76 and then to track 0.  Thus, for example, unix block number
 *	498 starts at track 0, sector 21 and runs thru track 0, sector 2.
 */
rxfactr(sectr, psectr, ptrck)
register int sectr;
int *psectr, *ptrck;
{
	register int p1, p2;

	p1 = sectr/26;
	p2 = sectr%26;
	/* 2 to 1 interleave */
	p2 = (2*p2 + (p2 >= 13 ? 1 : 0)) % 26;
	/* 6 sector per track slew */
	*psectr = 1 + (p2 + 6*p1) % 26;
	if (++p1 >= 77)
		p1 = 0;
	*ptrck = p1;
}
