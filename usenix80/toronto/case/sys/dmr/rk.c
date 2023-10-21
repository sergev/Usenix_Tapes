#
/*
 */

/*
 * RK disk driver
 */

#include "../param.h"
#include "../buf.h"
#include "../conf.h"
#include "../user.h"
#include "../inst.h"

#define	RKADDR	0177400
#define	NRK	5
#define	NRKBLK	4872

#define	RESET	0
#define	GO	01
#define	DRESET	014
#define	IENABLE	0100
#define	DRY	0200
#define	ARDY	0100
#define	WLO	020000
#define	CTLRDY	0200

/* RK instrumentation */

#ifdef	MONITOR
static int RK_N;
#endif

struct {
	int rkds;
	int rker;
	int rkcs;
	int rkwc;
	int rkba;
	int rkda;
};

struct	devtab	rktab;
struct	buf	rrkbuf;


rkopen(dev, flag)
{
	if (dev.d_minor >= NRK)
		u.u_error = ENXIO;
}


rkstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;

	bp = abp;
	if (bp->b_blkno >= NRKBLK) {
		if (bp->b_flags&B_READ)
			bp->b_resid = bp->b_wcount;
		else {
			bp->b_flags =| B_ERROR;
			bp->b_error = ENXIO;
		}
		iodone(bp);
		return;
	}
	bp->av_forw = 0;
	spl5();
	if (rktab.d_actf==0)
		rktab.d_actf = bp;
	else
		rktab.d_actl->av_forw = bp;
	rktab.d_actl = bp;
	if (rktab.d_active==0)
		rkstart();
	spl0();
}

rkaddr(bp)
register struct buf *bp;
{
	register int b, d;

	b = bp->b_blkno;
	d = bp->b_dev.d_minor;
	return(d<<13 | (b/12)<<4 | b%12);
}

rkstart()
{
	register struct buf *bp;

	if ((bp = rktab.d_actf) == 0)
		return;
	rktab.d_active++;
#ifdef	MONITOR
	if (I.inst_go) {		/* If System Instrumentation Trace enabled */
		RK_N = bp->b_dev.d_minor;
		I.rk_busy =| (1<<RK_N);
		I.rk_numb[RK_N]++;
		I.rk_wds[RK_N] =+ (-bp->b_wcount);
		I.rk_time[RK_N].d_start = I.c_ticks;
	}
#endif
	devstart(bp, &RKADDR->rkda, rkaddr(bp), 0);
}

rkintr()
{
	register struct buf *bp;

	if (rktab.d_active == 0)
		return;
	bp = rktab.d_actf;
#ifdef	MONITOR
	if (I.inst_go) {		/* If System Instrumentation Trace enabled */
		RK_N = bp->b_dev.d_minor;
		I.rk_busy =& ~(1<<RK_N);
		I.rk_time[RK_N].d_accum += I.c_ticks - I.rk_time[RK_N].d_start;
	}
#endif
	rktab.d_active = 0;
	if (RKADDR->rkcs < 0) {		/* error bit */
		deverror(bp, RKADDR->rker, RKADDR->rkds);
		RKADDR->rkcs = RESET|GO;
		while((RKADDR->rkcs&CTLRDY) == 0) ;
		if (++rktab.d_errcnt <= 10) {
			rkstart();
			return;
		}
		bp->b_flags =| B_ERROR;
	}
	rktab.d_errcnt = 0;
	rktab.d_actf = bp->av_forw;
	bp->b_resid = RKADDR->rkwc;
	iodone(bp);
	rkstart();
}

rkread(dev)
{
	physio(rkstrategy, &rrkbuf, dev, B_READ, NRKBLK);
}

rkwrite(dev)
{
	physio(rkstrategy, &rrkbuf, dev, B_WRITE, NRKBLK);
}
