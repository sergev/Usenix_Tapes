#
/*
 * RK06/RK07 disk driver modeled after RP04 driver by
 * Robert L. Kirby, Computer Science Center, University of Maryland.
 *
 * This driver has not been tested on a working RK06/RK07.
 * It does not attempt ECC error correction and is probably
 * deficient in general in the case of errors.
 * When packs are remounted, prints message "remounted".
 * If this driver may be used on an 11/70,
 * the symbol "CPU70" must be defined in param.h.
 */

#include "../h/param.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/user.h"

struct {
	int	hkcs1;	/* Control and Status register 1 */
	int	hkwc;	/* Word count register */
	int	hkba;	/* UNIBUS address register */
	int	hkda;	/* Desired address register */
	int	hkcs2;	/* Control and Status register 2 */
	int	hkds;	/* Drive Status */
	int	hker;	/* Error register */
	int	hkas;	/* Attention Summary */
	int	hkdc;	/* Desired Cylinder address register*/
	int     hknr;   /* No register */
	int	hkdb;	/* Data buffer register */
	int	hkmr1;	/* Maintenance register 1 */
	int	hkpos;	/* Burst error bit position */
	int	hkpat;	/* Burst error bit pattern */
	int	hkmr2;	/* Maintenance register 2 */
	int	hkmr3;	/* Maintenance register 3 */
};
#define hkof	hkas	/* Offset register */

#define HKADDR	0177440
#define NHK	1

struct	devtab	hktab;
struct	buf	hkbuf;

int     hktype[NHK];    /* Controller Drive Type bit for hkcs1 */
char    *hksize[NHK];   /* Number of blocks on each disk */

#define GO	01
#define PACKN   02
#define RCLR    04
#define RECAL   012
#define OFFSET  014
#define HKREAD	020
#define HKWRIT	022

#define VV	0100	/* hkds - Volume Valid */
#define	READY	0200	/* hkds - drive ready */
#define DDT     0400    /* hkds - disk drive type */
#define	PIP	020000	/* hkds - Positioning Operation in Progress */
#define SVAL	0100000	/* hkds - Status Valid */
#define CERR    0100000 /* hkcs1 - composite error/controller clear */
#define CDT	02000	/* hkcs1 - Controller Drive Type */
#define IENABLE 0100

#define DU      040000  /* hker - Drive Unsafe */
#define DTE     010000  /* hker - Drive Timing Error   */
#define OPI     020000  /* hker - Operation Incomplete */
		/* Error Correction Code errors */
#define DCK     0100000 /* hker - Data Check error */
#define ECH     0100    /* hker - ECC hard error */

#define SCLR    040     /* hkcs2 - Subsystem Clear */

hkstrategy(abp)
struct buf *abp;
{
	register struct buf *bp, *p1, *p2;

	bp = abp;
	if (bp->b_dev.d_minor >= NHK ||
	    (bp->b_blkno >= hksize[bp->b_dev.d_minor])
	     && hksize[bp->b_dev.d_minor]) {
		bp->b_flags =| B_ERROR;
		iodone(bp);
		return;
	}
#ifdef CPU70
	if (bp->b_flags&B_PHYS)
		mapalloc(bp);
#endif CPU70
	spl5();
	if ((p1 = hktab.d_actf)==0) {
		hktab.d_actf = bp;
		bp->av_forw = 0;
	} else {
		for (; p2 = p1->av_forw; p1 = p2) {
			if (p1->b_blkno <= bp->b_blkno
			 && bp->b_blkno <  p2->b_blkno
			 || p1->b_blkno >= bp->b_blkno
			 && bp->b_blkno >  p2->b_blkno)
				break;
		}
		bp->av_forw = p2;
		p1->av_forw = bp;
	}
	if (hktab.d_active==0)
		hkstart();
	spl0();
}

hkstart()
{
	register struct buf *bp;
	register int cylin, dev;
	int trksec;

	if ((bp = hktab.d_actf) == 0)
		return;
	hktab.d_active++;
	dev = bp->b_dev.d_minor;
	HKADDR->hkcs2 = dev;
	if(hksize[dev] == 0) {
		for(cylin = 0; HKADDR->hkds & SVAL == 0 && --cylin;);
		hksize[dev] =   /* RK07 vs RK06 */
		   (hktype[dev] = HKADDR->hkds & DDT ? CDT : 0)
			? 53790 : 27126;
		HKADDR->hkcs1 = PACKN | GO | hktype[dev];
	}
	cylin = bp->b_blkno;
	trksec = lrem(cylin, 22);
	cylin = ldiv(cylin, 22);
	trksec =| (cylin % 3) << 8;
	HKADDR->hkda = trksec;
	HKADDR->hkdc = cylin / 3;
	HKADDR->hkba = bp->b_addr;      /* buffer address */
	HKADDR->hkwc = bp->b_wcount;    /* word count */
	HKADDR->hkcs1 = hktype[dev] | ((bp->b_xmem & 03) << 8)
		| (bp->b_flags&B_READ	/* x-mem + command */
			? (HKREAD | IENABLE | GO)
			: (HKWRIT | IENABLE | GO));
}

hkintr()
{
	register struct buf *bp;
	register int ctr, dev;

	if (hktab.d_active == 0)
		return;
	bp = hktab.d_actf;
	hktab.d_active = 0;
	if (HKADDR->hkcs1 & CERR) {		/* error bit */
		deverror(bp, HKADDR->hker, HKADDR->hkcs2);
		dev = bp->b_dev.d_minor;
		if(HKADDR->hker & (DU|DTE|OPI)) {
			HKADDR->hkcs2 = SCLR | dev;
			HKADDR->hkcs1 = hktype[dev] | RECAL | GO;
			ctr = 0;
			while (HKADDR->hkds&PIP || HKADDR->hkds&SVAL == 0)
			       if(--ctr == 0) break;
		}
		if((HKADDR->hker & VV) == 0) {
			HKADDR->hkcs1 = hktype[dev] | PACKN | GO;
			printf("remounted\n");
		}
		HKADDR->hkcs1 = hktype[dev] | RCLR | GO;
		if (++hktab.d_errcnt <= 10) {
			hkstart();
			return;
		}
		bp->b_flags =| B_ERROR;
	}
	hktab.d_errcnt = 0;
	hktab.d_actf = bp->av_forw;
	bp->b_resid = HKADDR->hkwc;
	iodone(bp);
	hkstart();
}

hkread(dev)
{

	if(hkphys(dev))
		physio(hkstrategy, &hkbuf, dev, B_READ);
}

hkwrite(dev)
{

	if(hkphys(dev))
		physio(hkstrategy, &hkbuf, dev, B_WRITE);
}

hkphys(dev)
{
	register char *c;

	c = lshift(u.u_offset, -9);
	c =+ ldiv(u.u_count+511, 512);
	if(c > hksize[dev.d_minor]) {
		u.u_error = ENXIO;
		return(0);
	}
	return(1);
}

