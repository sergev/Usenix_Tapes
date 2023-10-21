#
/*
 */

/*
 * RM03 disk driver
 *
 * This driver has been tested on a working RM03 for a few hours.
 * It does not attempt ECC error correction and is probably
 * deficient in general in the case of errors and when packs
 * are dismounted.
 *
 * changed disk partition table "hm_sizes", added hm8, also
 * changed minor dev number mapping from 3 to 4 bits, allowing
 * 16 filesystems per disk instead of  8.  Minor dev numbers
 * 16. , etc refer to physical drive 1. -G. Goble, 7/17/77.
 *
 * Converted to RM03 by Mike O'Dell 16-Nov-78 by patching
 * magic numbers (sectors/track, tracks/cylinder, etc.),
 * register layout (minor changes) and renaming everything
 * from "hp....." to "hm.....".  These modifications were
 * sufficient to get a running driver from the stock RM03
 * driver in distributed v6 unix, and they should work 
 * here, but no promises.
 *
 */

/* #define UNSAFE_TDF	/* extra debug code for Rogers to find TDF UNSAFE */
#include "../param.h"
#include "../buf.h"
#include "../conf.h"
#include "../user.h"
#include "../proc.h"
#include "../systm.h"
#ifdef STATS
#include "../stats.h"
#endif

struct {
	int	hmcs1;	/* Control and Status register 1 */
	int	hmwc;	/* Word count register */
	int	hmba;	/* UNIBUS address register */
	int	hmda;	/* Desired address register */
	int	hmcs2;	/* Control and Status register 2*/
	int	hmds;	/* Drive Status */
	int	hmer1;	/* Error register 1 */
	int	hmas;	/* Attention Summary */
	int	hmla;	/* Look ahead */
	int	hmdb;	/* Data buffer */
	int	hmmr;	/* Maintenance register */
	int	hmdt;	/* Drive type */
	int	hmsn;	/* Serial number */
	int	hmof;	/* Offset register */
	int	hmca;	/* Desired Cylinder address register*/
	int	hmcc;	/* Current Cylinder */
	int	hmmr2;	/* Maintenence register 2 */
	int	hmer2;	/* Error register 2 */
	int	hmpos;	/* Burst error bit position */
	int	hmpat;	/* Burst error bit pattern */
	int	hmbae;	/* 11/70 bus extension */
	int	hmcs3;	/* Control and Status register 3 */
};

#define	HMADDR	0176700
#define	NHM	1

struct {
	char	*nblocks;
	int	cyloff;
} hm_sizes[] {
/*
 * fetch the standard disk layout
 */
#include "rm03.layout"

};


struct	devtab	hmtab;
struct	buf	hmbuf;

char	hm_openf;
int	hmsort	1;	/* if set, sort requests by cyl. */
# ifdef POWERFAIL
int	hmfail;		/* nz if power failure */
# endif

#ifdef UNSAFE_TDF
int	hm_ptrk;	/* previous trk+sec reg*/
int	hm_pcsr;	/* previous CSR reg */
#endif
			/* Drive Commands */
#define	GO	01
#define	PRESET	020
#define	RECAL	06
#define RCLR	010
#define OFFSET	014

#define	IENABLE	0100	/* interrupt enable bit */
#define	READY	0200	/* hmds - drive ready */
#define	PIP	020000	/* hmds - Positioning Operation in Progress */
#define MOL	010000	/* hmds - Medium Online */
#define	ERR	040000	/* hmcs1 - composite error */

#define	DU	040000	/* hmer1 - Drive Unsafe	*/
#define	DTE	010000  /* hmer1 - Drive Timing Error	*/
#define	OPI	020000  /* hmer1 - Operation Incomplete	*/
		/* Error Correction Code errors */
#define DCK	0100000	/* hmer1 - Data Check error */
#define ECH	0100    /* hmer1 - ECC hard error */

#define CLR	040	/* hmcs2 - Controller Clear */

#define FMT22	010000	/* hmof - 16 bit /word format */
/*
 * Use av_back to save track+sector,
 * b_resid for cylinder.
 */

#define	trksec	av_back
#define	cylin	b_resid

hmopen(dev, flag)
{

# ifdef POWERFAIL
	if(dev == NODEV) {
		hm_openf = 0;
		hmfail++;
		printf("REC RM03-hm\n");
		while((HMADDR->hmds & (MOL|READY)) != (MOL|READY));
	}
# endif
	if(!hm_openf){
		hm_openf++;
		HMADDR->hmcs2 = CLR;
		HMADDR->hmcs1 = RCLR|GO;
		HMADDR->hmcs1 = PRESET|GO;
		HMADDR->hmof = FMT22;
	}
# ifdef POWERFAIL
	if (hmfail) {
		while ((HMADDR->hmds & READY) == 0);
		/*
		 * Just incase power fail interrupt occurs
		 * in rhstart when dev registers are being
		 * loaded, we must stuff invalid track/sec
		 * and mem addr in to avoid clobbering things.
		 */
		HMADDR->hmca = -1;
		HMADDR->hmda = -1;
		HMADDR->hmba = 0177700; /* will get bus timeout */
		HMADDR->hmcs1 =| IENABLE;
	}
# endif
}

hmstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;
	register char *p1, *p2;

	bp = abp;
	p1 = &hm_sizes[bp->b_dev.d_minor&017];
	if (bp->b_dev.d_minor >= (NHM<<4) ||
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
	if ((p1 = hmtab.d_actf)==0)
		hmtab.d_actl = hmtab.d_actf = bp;
	else {
		if(hmsort) {
			for (; p2 = p1->av_forw; p1 = p2) {
				if (p1->cylin <= bp->cylin
				 && bp->cylin <  p2->cylin
				 || p1->cylin >= bp->cylin
				 && bp->cylin >  p2->cylin) 
					break;
			}
			if(p2 == 0)
				hmtab.d_actl = bp;
			bp->av_forw = p2;
			p1->av_forw = bp;
		}
		else {
# ifdef XBUF
			*ka5 = praddr;
# endif
			if(u.u_procp->p_nice <= -5) {	/* priority req */
				if((bp->av_forw = hmtab.d_actf->av_forw) == 0)
					hmtab.d_actl = bp; /* update tail */
				hmtab.d_actf->av_forw = bp; /* link to 2nd */
			}
			else {
				hmtab.d_actl->av_forw = bp;
				hmtab.d_actl = bp;
			}
		}
	}
	if (hmtab.d_active==0)
		hmstart();
	spl0();
}

hmstart()
{
	register struct buf *bp;

	if ((bp = hmtab.d_actf) == 0)
		return;
#ifdef STATS
	stats.mon_cur[HMXF_CNT]++;
#endif
# ifdef POWERFAIL
	hmfail = 0;
# endif
	hmtab.d_active++;
	HMADDR->hmcs2 = bp->b_dev.d_minor >> 4;
	HMADDR->hmca = bp->cylin;
#ifdef UNSAFE_TDF
	hm_pcsr = HMADDR->hmcs1;
	hm_ptrk = HMADDR->hmda;
#endif
	rhstart(bp, &HMADDR->hmda, bp->trksec, &HMADDR->hmbae);
}

hmintr()
{
	register struct buf *bp;
	int ctr, i;

	if (hmtab.d_active == 0)
		return;
	bp = hmtab.d_actf;
	hmtab.d_active = 0;
#ifdef POWERFAIL
	if (hmfail) {
		hmstart();
		return;
	}
# endif
	if (HMADDR->hmcs1 & ERR) {		/* error bit */
		HMADDR->hmcs1 =& ~IENABLE; /* this may be dangerous */
		spl0();
		deverror(bp, HMADDR->hmcs2, HMADDR->hmcs1);
/* expanded error dump for RM03 */
printf("RM03-hm ERR - ds:0%o er1:0%o er2:0%o cs3:0%o trksec:0%o cyl:0%o\n",
 HMADDR->hmds, HMADDR->hmer1, HMADDR->hmer2, HMADDR->hmcs3,
 HMADDR->hmda, HMADDR->hmcc);
/* end of expanded RM03 error register dump */

#ifdef UNSAFE_TDF
printf("prev CSR:0%o  prev TRKSEC:0%o\n", hm_pcsr, hm_ptrk);
#endif
		if (HMADDR->hmer1 & DU) {
			printf("RM03 UNSAFE\n");
			for(i=0; i<1; i++) /* delay to let disk settle down*/
				while(--ctr);
/*
			printf("WE ASSUME HEADS RETRACTED\n");
			halt();
*/
			HMADDR->hmcs2 = CLR;
			while((HMADDR->hmds & MOL) == 0);
		}
/*
		printf("ATTEMPTING TO CLEAR CONTROLLER AND DRIVES\n");
*/
		HMADDR->hmcs2 = CLR;
		while ((HMADDR->hmcs1 & READY) == 0);
		HMADDR->hmcs1 = (ERR|RCLR|GO);
		while ((HMADDR->hmcs1 & READY) == 0);
		HMADDR->hmcs1 = CLR;
		while ((HMADDR->hmcs1 & READY) == 0);
		while ((HMADDR->hmds & READY) == 0);
		if (HMADDR->hmcs1 & ERR) {
			printf("Can't clear RM03 error - halt - call SYSTEMS staff\n");
			halt();
		}
		HMADDR->hmcs1 = PRESET|GO;	/* set volume valid */
		HMADDR->hmof = FMT22;
		spl5();
		if (++hmtab.d_errcnt <= 25) {
			hmstart();
			return;
		}
		bp->b_flags =| B_ERROR;
		printf("RM03-hm ERROR NOT RECOVERED\n");
		printf("Panic: Fatal system disk error\n");
		halt();
	}
	hmtab.d_errcnt = 0;
	hmtab.d_actf = bp->av_forw;
	bp->b_resid = HMADDR->hmwc;
	iodone(bp);
	hmstart();
}

hmread(dev)
{

	if(hmphys(dev))
	physio(hmstrategy, &hmbuf, dev, B_READ);
}

hmwrite(dev)
{

	if(hmphys(dev))
	physio(hmstrategy, &hmbuf, dev, B_WRITE);
}

hmphys(dev)
{
	register c;

	c = lshift(u.u_offset, -9);
	c =+ ldiv(u.u_count+511, 512);
	if(c > hm_sizes[dev.d_minor & 017].nblocks) {
		u.u_error = ENXIO;
		return(0);
	}
	return(1);
}

