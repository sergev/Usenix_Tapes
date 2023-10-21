#
/*
 */

/*
 * RP04 disk driver
 *
 * This driver has been tested on a working RP04 for a few hours.
 * It does not attempt ECC error correction and is probably
 * deficient in general in the case of errors and when packs
 * are dismounted.
 *
 * changed disk partion table "si_sizes", G. Goble, 3/7/77
 *
 * changed disk partition table "si_sizes", added si8, also
 * changed minor dev number mapping from 3 to 4 bits, allowing
 * 16 filesystems per disk instead of  8.  Minor dev numbers
 * 16. , etc refer to physical drive 1. -G. Goble, 7/17/77.
 * 
 * Convert to System Industries 9400 controller.  --G. Goble 09/17/78.
 * A "mapped" SI9400 controller connected to a CDC 9766 300 Megebyte
 * drive looks like 3 DEC RP04's.  An unmapped SI9400 looks like
 * 1 DEC RP04, except there are 33 sectors/track (instead of 22)
 * and there are 823 cylinders instead of 411.
 *
 */

/* #define UNSAFE_TDF	/* extra debug code for Rogers to find TDF UNSAFE */
#define DUMP		/* crash dump SI 9400 controller 6800 memory */

#define	UNMAPPED	/* use unmapped 9400 controller ROM */

#ifdef	UNMAPPED
# define	SECTRK	33	/* 33 sectors/track */
#endif
#ifndef UNMAPPED
# define	SECTRK	22	/* 22 sectors/track (RP04 emulator mode) */
#endif

#define	HEADS	19	/* R/W heads (tracks/cyl) */
#define	NRETRY	10	/* retries on error */
#define DEBUG		/* compile in debug code */
#include "../param.h"
#include "../buf.h"
#include "../conf.h"
#include "../user.h"
#include "../proc.h"
#include "../systm.h"
#include "../errlog.h"
#ifdef STATS
#include "../stats.h"
#endif

struct {
	int	sics1;	/* Control and Status register 1 */
	int	siwc;	/* Word count register */
	int	siba;	/* UNIBUS address register */
	int	sida;	/* Desired address register */
	int	sics2;	/* Control and Status register 2*/
	int	sids;	/* Drive Status */
	int	sier1;	/* Error register 1 */
	int	sias;	/* Attention Summary */
	int	sila;	/* Look ahead */
	int	sidb;	/* Data buffer */
	int	simr;	/* Maintenance register */
	int	sidt;	/* Drive type */
	int	sisn;	/* Serial number */
	int	siof;	/* Offset register */
	int	sica;	/* Desired Cylinder address register*/
	int	sicc;	/* Current Cylinder */
	int	sier2;	/* Error register 2 */
	int	sier3;	/* Error register 3 */
	int	sipos;	/* Burst error bit position */
	int	sipat;	/* Burst error bit pattern */
	int	sibae;	/* 11/70 bus extension */
};

#define	SIADDR	0170700
#define	NSI	3

# ifndef UNMAPPED
/* 418 blocks (sectors/cyl) */
struct {
	char	*nblocks;
	int	cyloff;
} si_sizes[] {
	5016,	192,	/* si0 cyl 192 thru 203 */
	-1,	35,	/* si1 cyl 35 thru 191 (-1 is 65535) */
	-1,	204,	/* si2 cyl 204 thru 360 */
	20900,	361,	/* si3 cyl 361 thru 410 */
	13794,	2,	/* si4 cyl 2 thru 34 */
	836,	0,	/* si5 cyl 0 thru 1(boot up file sys) */
			/* note: si6 and si7 are subsets of si4 */
	7942,	2,	/* si6 cyl 2 thru 20 (overlaps si4!!) */
	4180,	21,	/* si7 cyl 21 thru 30 (overlaps si4!!) */
	1672,	31,	/* si8 cyl 31 thru 34 */
	4180+1672, 21,	/* si9 same as si7+si8 */

};
# endif
#ifdef UNMAPPED
/* 627 blocks (sectors/cyl) */
struct {
	char	*nblocks;
	int	cyloff;
} si_sizes[] {
	65535,	0,	/* si0 cyl 000 thru 104 */
	65535,	105,	/* si1 cyl 105 thru 209 */
	65535,	210,	/* si2 cyl 210 thru 314 */
	65535,	315,	/* si3 cyl 315 thru 419 */
	12540,  420,    /* si4 cyl 420 thru 439 (root) */
	65535,  440,    /* si5 cyl 440 thru 544 */
	65535,  545,    /* si6 cyl 545 thru 649 */
	65535,  650,    /* si7 cyl 650 thru 754 */
	42636,  755,    /* si8 cyl 755 thru 822 */

};
#endif


#define	GO	01
#define	PRESET	020
#define	RECAL	06
#define RCLR	010
#define OFFSET	014

#define	IENABLE	0100	/* interrupt enable bit */
#define	READY	0200	/* sids - drive ready */
#define VV	0100	/* sids - volume valid */
#define	PIP	020000	/* sids - Positioning Operation in Progress */
#define MOL	010000	/* sids - Medium Online */
#define DRVERR	040000	/* sids - drive error */
#define	ERR	040000	/* sics1 - composite error */

#define	DU	040000	/* sier1 - Drive Unsafe	*/
#define UWR	010	/* sier3 - unsafe/heads retracted */
#define	DTE	010000  /* sier1 - Drive Timing Error	*/
#define	OPI	020000  /* sier1 - Operation Incomplete	*/
		/* Error Correction Code errors */
#define DCK	0100000	/* sier1 - Data Check error */
#define ECH	0100    /* sier1 - ECC hard error */

#define CLR	040	/* sics2 - Controller Clear */
#define NED	010000	/* sics2 - Non existant drive */

#define FMT22	010000	/* siof - 16 bit /word format */
/*
 * Use av_back to save track+sector,
 * b_resid for cylinder.
 */

#define	trksec	av_back
#define	cylin	b_resid

struct	devtab	sitab;
struct	buf	sibuf;

char	si_openf;
int     sikludge 0;     /* 3 if unit1 plug in drive */
int     si_rh70 0;      /* nz if on RH70 massbuss, 0 for Unibus */
int	si_of	FMT22;	/* value to load into siof */
int	sifault;	/* SI hung, can't clear drive fault */
int	siwait	0;	/* keep trying to clear hung drive */
int	siabort;	/* nz if fatal I/O error */
int	sisort	1;	/* if set, sort requests by cyl. */
int	si_debug 0;	/* nz to debug */
int	siretry	NRETRY;	/* retry count */
# ifdef POWERFAIL
int	sifail;		/* nz if power failure */
# endif

#ifdef UNSAFE_TDF
int	si_ptrk;	/* previous trk+sec reg*/
int	si_pcsr;	/* previous CSR reg */
#endif
			/* Drive Commands */
siopen(dev, flag)
{
	int ctr;

# ifdef POWERFAIL
		/*
		 * Our model of the SI 9400 controller does funny
		 * things on Power fail/restart.  MOL and DRDY come
		 * up when Power is applied to drive electronics,
		 * and NED is cleared, even if drive (front panel)
		 * button is turned off.  At present there are several
		 * weird unclearable states both drive and controller
		 * can get into.  Basically, one must attempt to
		 * function the drive to see if it works.
		 * also RECALIBRATE's must be issued often to keep
		 * drive happy.
		 * This power fail code assumes drive is already
		 * spun back up (drives comes up in about 10 sec)
		 * the reset loop in mch.s provides this delay.
		 * Also if only 1 phase drops or CPU power fails
		 * and drive power does not, or a short
		 * power glitch occurs, the drive will probably
		 * just fault and must be manually cleared.
		 * The drive hardware will be modified to 
		 * clear itself after awhile it is hoped.
		 * --ghg 10/08/78.
		 */

	if(dev == NODEV) {
		if (si_openf == 0) return;
		si_openf = 0;
		sifail++;
		printf("REC SI-0\n");
		SIADDR->sics2 = CLR;
		SIADDR->sics2 = sikludge;	/* for now */
		SIADDR->sics1 = PRESET|GO;	/* set VV bit */
		SIADDR->sics1 = RECAL|GO;
		if (SIADDR->sics2 & NED)
			goto fault;
		ctr = 0;
		while (((SIADDR->sids & READY) == 0) && --ctr);
		if (!ctr || SIADDR->sids& DRVERR) {
fault:
			/* kiss it off */
			sifault++;
			SIADDR->sics2 = CLR;
			SIADDR->sics1 = 0;
			printf("SI disk hung or faulted - manual recovery needed\n");
			return;
		}
	}
# endif
	if(!si_openf){
		si_openf++;
		SIADDR->sics2 = CLR;
		SIADDR->sics2 = sikludge;
		SIADDR->sics1 = RCLR|GO;
		SIADDR->sics1 = PRESET|GO;
		SIADDR->siof = FMT22;
	}
# ifdef POWERFAIL
	if (sifail) {
		while ((SIADDR->sids & READY) == 0);
		/*
		 * Just incase power fail interrupt occurs
		 * in rhstart when dev registers are being
		 * loaded, we must stuff invalid track/sec
		 * and mem addr in to avoid clobbering things.
		 */
		SIADDR->sica = -1;
		SIADDR->sida = -1;
		SIADDR->siba = 0177700; /* will get bus timeout */
		SIADDR->sics1 =| IENABLE;
	}
# endif
}

sistrategy(abp)
struct buf *abp;
{
	register struct buf *bp;
	register char *p1, *p2;

	bp = abp;
	while (sifault)
		sleep(bp, PRIBIO-3);
	if ((bp->b_flags & B_PHYS) && si_rh70 == 0)
		mapalloc(bp);
#ifdef FLAKEYBUS
	devlock();	/* kludge until bus is fixed !!!!!!!!!! */
#endif
	p1 = &si_sizes[bp->b_dev.d_minor&017];
	if (bp->b_dev.d_minor >= (NSI<<4) ||
	    siabort ||
	    bp->b_blkno >= p1->nblocks) {
		bp->b_flags =| B_ERROR;
#ifdef FLAKEYBUS
		devfree();	/* KLUDGE !!!!!!!!!!!! flakey bus */
#endif
		iodone(bp);
		return;
	}
	bp->av_forw = 0;
	bp->cylin = p1->cyloff;
	p1 = bp->b_blkno;
	p2 = lrem(p1, SECTRK);
	p1 = ldiv(p1, SECTRK);
	bp->trksec = (p1%HEADS)<<8 | p2;
	bp->cylin =+ p1/HEADS;
	spl5();
	if ((p1 = sitab.d_actf)==0)
		sitab.d_actl = sitab.d_actf = bp;
	else {
		if(sisort) {
			for (; p2 = p1->av_forw; p1 = p2) {
				if (p1->cylin <= bp->cylin
				 && bp->cylin <  p2->cylin
				 || p1->cylin >= bp->cylin
				 && bp->cylin >  p2->cylin) 
					break;
			}
			if(p2 == 0)
				sitab.d_actl = bp;
			bp->av_forw = p2;
			p1->av_forw = bp;
		}
		else {
/*
# ifdef XBUF
			*ka5 = praddr;
# endif
*/
			if(u.u_procp->p_nice <= -5) {	/* priority req */
				if((bp->av_forw = sitab.d_actf->av_forw) == 0)
					sitab.d_actl = bp; /* update tail */
				sitab.d_actf->av_forw = bp; /* link to 2nd */
			}
			else {
				sitab.d_actl->av_forw = bp;
				sitab.d_actl = bp;
			}
		}
	}
	if (sitab.d_active==0)
		sistart();
	spl0();
}

sistart()
{
	register struct buf *bp;

loop:
	if ((bp = sitab.d_actf) == 0)
		return;
	if (siabort) {
		bp->b_flags =| B_ERROR;	/* abort buffer chain */
		sitab.d_actf = bp->av_forw;
		iodone(bp);
		goto loop;
	}
#ifdef STATS
	stats.mon_cur[RPXF_CNT]++;
#endif
# ifdef POWERFAIL
	sifail = 0;
# endif
	sitab.d_active++;
	SIADDR->sics2 = (bp->b_dev.d_minor >> 4) + sikludge;
	SIADDR->sica = bp->cylin;
	if ((SIADDR->sids & VV) == 0)
		SIADDR->sics1 = PRESET|GO;
	SIADDR->siof = si_of;	/* patch to inhibit ECC */
#ifdef UNSAFE_TDF
	si_pcsr = SIADDR->sics1;
	si_ptrk = SIADDR->sida;
#endif
# ifdef DEBUG
	if (si_debug)
		printf("SISTART  cs2:%o dc:%o da:%o\n",
		SIADDR->sics2, SIADDR->sica, bp->trksec);
# endif
	if (si_rh70)
		rhstart(bp, &SIADDR->sida, bp->trksec, &SIADDR->sibae);
	else
		rhstrt(bp, &SIADDR->sida, bp->trksec, &SIADDR->sibae);

}

siintr()
{
	register struct buf *bp;
	register csr;
	int ctr, i;

# ifdef DEBUG
/* check for flakey interrupt */
	if (((csr = SIADDR->sics1) & GO) == GO || (csr & READY) == 0)
		printf("Illegal SI int cs1:0%o wc:0%o\n", csr, SIADDR->siwc);

	if (si_debug) {
	printf("INT cs1:%o wc:%o ba:%o da:%o cs2:%o ds:%o e1:%o dc%o cc%o e2:%o e3:%o\n",
		SIADDR->sics1,
		SIADDR->siwc,
		SIADDR->siba,
		SIADDR->sida,
		SIADDR->sics2,
		SIADDR->sids,
		SIADDR->sier1,
		SIADDR->sica,
		SIADDR->sicc,
		SIADDR->sier2,
		SIADDR->sier3);
	}
# endif
	if (sitab.d_active == 0)
		return;
	bp = sitab.d_actf;
	sitab.d_active = 0;
#ifdef POWERFAIL
	if (sifail) {
		sistart();
		return;
	}
# endif
		/*
		 * Our SI 9400 controller firmware doesn't always
		 * set TRE in rpcs1 when it should.  One must check
		 * for DRVERR in RPDS, TRE, and word count being
		 * non zero, all which meant something went wrong.
		 */
	if (SIADDR->sics1&ERR || SIADDR->siwc || SIADDR->sids&DRVERR) {
/* IENABLE gets cleared when CPU recognizes INT */
/*		SIADDR->sics1.lobyte =& ~IENABLE; /* this may be dangerous */
		spl0();
		deverror(bp, SIADDR->sics2, SIADDR->sics1);
/* expanded error dump for SI -09/21/76 -ghg */
printf("SI ERR - ds:%o er1:%o er2:%o er3:%o trksec:%o cyl:%o wc:%o of:%o\n",
 SIADDR->sids, SIADDR->sier1, SIADDR->sier2, SIADDR->sier3,
 SIADDR->sida, SIADDR->sicc, SIADDR->siwc,SIADDR->siof);
/* end of expanded SI error register dump */

#ifdef UNSAFE_TDF
printf("prev CSR:0%o  prev TRKSEC:0%o\n", si_pcsr, si_ptrk);
#endif
hung:
#ifdef DUMP
		sidump();
#endif
		SIADDR->sics2 = CLR;
		while ((SIADDR->sics1 & READY) == 0);
		SIADDR->sics2 = sikludge;
		SIADDR->sics1 = PRESET|GO;
		SIADDR->sics1 = (ERR|RCLR|GO);
		while ((SIADDR->sics1 & READY) == 0);
		for (ctr= -1; (SIADDR->sids & READY) == 0 && --ctr;);
		SIADDR->sics1 = RECAL|GO;
		while ((SIADDR->sics1 & READY) == 0);
		for (ctr= -1; (SIADDR->sids & READY) == 0 && --ctr;);
		if ( !ctr || (SIADDR->sics1 & ERR) || (SIADDR->sids&DRVERR)) {
			if (siwait) {	/* keep trying to clear error */
				printf("Trying to clear SI err\n");
				for (i=0; i<30; i++)
					for (ctr=0; --ctr;);	/* hard wait */
				goto hung;
			}
			printf("Can't clear SI error - hung - call SYSTEMS staff\n");
			printf("cs1:%o ds:%o ctr:%o\n", SIADDR->sics1, SIADDR->sids, ctr);
			sifail++;	/* fake power fail to retry current op */
			sifault++;	/* so no more buffers get Q'd */
			sitab.d_active++;
			return;
		}
		SIADDR->sics1 = PRESET|GO;	/* set volume valid */
		SIADDR->siof = FMT22;
		spl5();
		if (++sitab.d_errcnt <= siretry) {
			sistart();
			return;
		}
		bp->b_flags =| B_ERROR;
		spl0();
		printf("SI-0 ERROR NOT RECOVERED\n");
		spl5();
		if ((bp->b_flags&B_PHYS) == 0)	/* abort if cooked */
			siabort++;	/* abort all further I/O */
		sinop();
	}
	sitab.d_errcnt = 0;
	sitab.d_actf = bp->av_forw;
	bp->b_resid = SIADDR->siwc;
#ifdef FLAKEYBUS
	devfree();	/* KLUDGE - FLAKEY BUS !!!!!!!!! */
#endif
	iodone(bp);
	sistart();
}

siread(dev)
{

	if(siphys(dev))
	physio(sistrategy, &sibuf, dev, B_READ);
}

siwrite(dev)
{

	if(siphys(dev))
	physio(sistrategy, &sibuf, dev, B_WRITE);
}

siphys(dev)
{
	register c;

	c = lshift(u.u_offset, -9);
	c =+ ldiv(u.u_count+511, 512);
	if(c > si_sizes[dev.d_minor & 017].nblocks) {
		u.u_error = ENXIO;
		return(0);
	}
	return(1);
}

sinop()
{
}
#ifdef DUMP
/*
 * code to crash dump 6800 memory on SI 9400 controller
 */
sidump()
{

	register i, j, c;
	int addr;

	addr = 0X0101;	/* start dump at Hex 101 */
	printf("SI 9400 controller 6800 memory dump\n");
	siprb(addr.hibyte);
	siprb(addr.lobyte);
	printf("   ");
	for(i=0; i<2; i++)
		siprb(siget(addr++));
	putchar('\n');
	/*
	 * dump function buffer area 26 bytes/line
	 */
	for (i=0; i<4; i++) {	/* currently 4 buffers */
		siprb(addr.hibyte);
		siprb(addr.lobyte);
		printf("  ");
		for (j=0; j<26; j++) {
			siprb(siget(addr++));
			putchar(' ');
		}
		putchar('\n');
	}
	putchar('\n');
}

siprb(c)
{

	c =& 0377;
	putchar("0123456789ABCDEF"[c>>4]);
	putchar("0123456789ABCDEF"[c & 017]);
}
/*
 * get a byte from 9400 controller memory
 */
siget(addr)
{

	while ((SIADDR->sics1 & READY) == 0);
	SIADDR->sica = addr;
	SIADDR->sics1 = 075;	/* read controller mem function */
	while ((SIADDR->sics1 & READY) == 0);
	return(SIADDR->sidb);
}
#endif
