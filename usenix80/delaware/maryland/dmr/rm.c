#
/*
 * RM02/RM03 disk driver copied from RP04 driver by
 * Robert L. Kirby, Computer Science Center, University of Maryland.
 *
 * This driver has not been tested on a working RM02/RM03.
 * It does not attempt ECC error correction and is probably
 * deficient in general in the case of errors.
 * When packs are remounted, prints message "remounted".
 */

#include "../h/param.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/user.h"

struct {
	int	rmcs1;	/* Control and Status register 1 */
	int	rmwc;	/* Word count register */
	int	rmba;	/* UNIBUS address register */
	int	rmda;	/* Desired address register */
	int	rmcs2;	/* Control and Status register 2 */
	int	rmds;	/* Drive Status */
	int	rmer1;	/* Error register 1 */
	int	rmas;	/* Attention Summary */
	int	rmla;	/* Look ahead */
	int	rmdb;	/* Data buffer */
	int	rmmr1;	/* Maintenance register 1 */
	int	rmdt;	/* Drive type */
	int	rmsn;	/* Serial number */
	int	rmof;	/* Offset register */
	int	rmdc;	/* Desired Cylinder address register*/
	int	rmhr;	/* Holding register */
	int     rmmr2;  /* Maintenance register 2 */
	int	rmer2;	/* Error register 2 */
	int	rmpos;	/* Burst error bit position */
	int	rmpat;	/* Burst error bit pattern */
	int	rmbae;	/* 11/70 bus extension */
	int	rmcs3;	/* Control and Status register 3 */
};

#define	RMADDR	0176700
#define NRM	2

struct {
	char	*nblocks;
	int	cyloff;
} rm_sizes[] {
	32800,	0,		/* cyl 0 thru 204 */
	32800,	205,		/* cyl 205 thru 409 */
	32800,	410,		/* cyl 410 thru 614 */
	32800,	615,		/* cyl 615 thru 819 */
	-1,	0,		/* cyl 0 thru 408 */
	160,	409,		/* cyl 409 thru 409 */
	-1,	410,		/* cyl 410 thru 818 */
	480,	819,		/* cyl 819 thru 822 */
};

struct	devtab	rmtab;
struct	buf	rmbuf;

char	rm_openf[NRM];

/* Drive Commands */
#define	GO	01
#define	PRESET	020
#define	RECAL	06
#define RCLR	010
#define OFFSET	014
#define PACKN	022

#define	READY	0200	/* rmds - drive ready */
#define	PIP	020000	/* rmds - Positioning Operation in Progress */
#define VV	0100	/* rmds - Volume Valid */
#define	ERR	040000	/* rmcs1 - composite error */

#define	DU	040000	/* rmer1 - Drive Unsafe	*/
#define	DTE	010000  /* rmer1 - Drive Timing Error	*/
#define	OPI	020000  /* rmer1 - Operation Incomplete	*/
		/* Error Correction Code errors */
#define DCK	0100000	/* rmer1 - Data Check error */
#define ECH	0100    /* rmer1 - ECC hard error */

#define CLR	040	/* rmcs2 - Controller Clear */

#define FMT22	010000	/* rmof - 16 bit /word format */
/*
 * Use av_back to save track+sector,
 * b_resid for cylinder.
 */

#define	trksec	av_back
#define	cylin	b_resid

rmopen(dev)
{
	register char *d;

	d = dev.d_minor>>3;
	if(d >= NRM) {
		u.u_error = ENXIO;
		return;
	}
	if(!rm_openf[d]){
		rm_openf[d]++;
		RMADDR->rmcs2 = CLR|d;
		RMADDR->rmcs1 = RCLR|GO;
		RMADDR->rmcs1 = PRESET|GO;
		RMADDR->rmof = FMT22;
	}
}

rmstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;
	register char *p1, *p2;

	bp = abp;
	p1 = &rm_sizes[bp->b_dev.d_minor&07];
	if (bp->b_dev.d_minor >= (NRM<<3) ||
	    bp->b_blkno >= p1->nblocks) {
		bp->b_flags =| B_ERROR;
		iodone(bp);
		return;
	}
	bp->av_forw = 0;
	bp->cylin = p1->cyloff;
	p1 = bp->b_blkno;
	p2 = lrem(p1, 32);
	p1 = ldiv(p1, 32);
	bp->trksec = (p1%5)<<8 | p2;
	bp->cylin =+ p1/5;
	spl5();
	if ((p1 = rmtab.d_actf)==0)
		rmtab.d_actf = bp;
	else {
		for (; p2 = p1->av_forw; p1 = p2) {
			if (p1->cylin <= bp->cylin
			 && bp->cylin <  p2->cylin
			 || p1->cylin >= bp->cylin
			 && bp->cylin >  p2->cylin)
				break;
		}
		bp->av_forw = p2;
		p1->av_forw = bp;
	}
	if (rmtab.d_active==0)
		rmstart();
	spl0();
}

rmstart()
{
	register struct buf *bp;

	if ((bp = rmtab.d_actf) == 0)
		return;
	rmtab.d_active++;
	RMADDR->rmcs2 = bp->b_dev.d_minor >> 3;
	RMADDR->rmdc = bp->cylin;
	rhstart(bp, &RMADDR->rmda, bp->trksec, &RMADDR->rmbae);
}

rmintr()
{
	register struct buf *bp;
	register int ctr;

	if (rmtab.d_active == 0)
		return;
	bp = rmtab.d_actf;
	rmtab.d_active = 0;
	if (RMADDR->rmcs1 & ERR) {		/* error bit */
		deverror(bp, RMADDR->rmer1, RMADDR->rmcs2);
		if(RMADDR->rmer1 & (DU|DTE|OPI)) {
			RMADDR->rmcs2 = CLR | (bp->b_dev.d_minor >> 3);
			RMADDR->rmcs1 = RECAL|GO;
			ctr = 0;
			while ((RMADDR->rmds&PIP) && --ctr);
		}
		if((RMADDR->rmer1 & VV) == 0) {
			RMADDR->rmcs1 = PACKN|GO;
			printf("remounted\n");
			RMADDR->rmof = FMT22;
		}
		RMADDR->rmcs1 = RCLR|GO;
		if (++rmtab.d_errcnt <= 10) {
			rmstart();
			return;
		}
		bp->b_flags =| B_ERROR;
	}
	rmtab.d_errcnt = 0;
	rmtab.d_actf = bp->av_forw;
	bp->b_resid = RMADDR->rmwc;
	iodone(bp);
	rmstart();
}

rmread(dev)
{

	if(rmphys(dev))
	physio(rmstrategy, &rmbuf, dev, B_READ);
}

rmwrite(dev)
{

	if(rmphys(dev))
	physio(rmstrategy, &rmbuf, dev, B_WRITE);
}

rmphys(dev)
{
	register c;

	c = lshift(u.u_offset, -9);
	c =+ ldiv(u.u_count+511, 512);
	if(c > rm_sizes[dev.d_minor & 07].nblocks) {
		u.u_error = ENXIO;
		return(0);
	}
	return(1);
}

