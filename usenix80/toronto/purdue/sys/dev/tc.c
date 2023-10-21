#
/*
 */

/*
 * TC-11 DECtape driver
 */

#include "../param.h"
#include "../conf.h"
#include "../buf.h"
#include "../user.h"
#include "../errlog.h"
#include "../systm.h"
#define KLUDGE	/* five volt power supply still sick */

struct {
	int	tccsr;
	int	tccm;
	int	tcwc;
	int	tcba;
	int	tcdt;
};

struct	devtab	tctab;
struct	buf	rtcbuf;
char	tcper[8];
char	tcopnf[8];	/* nz if unit is open */
int	tcdbg;		/* nz for debug mode or maint */
int	tctick -1;	/* >=0 timer running, <0 for not active */
int	tcabtf;		/* nz for drive hung, abort chain ahead */
int	tcignor;	/* if nz, don't set B_ERROR on dev err */

#ifdef FLAKEYBUS
int     tclock  0;      /* tc has dev lock reserved */
extern  int devlck;
#endif

#define TCTIMEOUT 20	/* abort drive if function not finished (sec) */
#define	TCADDR	0177340
#define	NTCBLK	578

#define	TAPERR	0100000
#define	TREV	04000
#define	READY	0200
#define	IENABLE	0100
#define	UPS	0200
#define	ENDZ	0100000
#define PAR	040000
#define MTE	020000
#define	BLKM	02000
#define DATM	01000
#define NEXM	0400
#define	ILGOP	010000
#define	SELERR	04000

#define	SAT	0
#define	RNUM	02
#define	RDATA	04
#define	SST	010
#define	WDATA	014
#define	GO	01

#define	SFORW	1
#define	SREV	2
#define	SIO	3

tcopen(dev)
{
	extern tcck();
	register int i;

#ifdef POWERFAIL
	if(dev == NODEV)
		return;	/* no powerfail recovery yet */
# endif
	i = dev&07;
	if(tcopnf[i]) {
		u.u_error = ENXIO;
		return;
	}
	tcopnf[i]++;
	if(tctick < 0) {
		tctick = 0;
		timeout(&tcck, 0, 60); /* start watchdog timer */
	}
}

/*
 * tcck - check for hung drive and abort the current function 
 * This isn't needed for regular Dectape drives, however Linc
 * tape drives made by Computer Operations (Dectape compatible)
 * will leave the computer thinking they are still in motion
 * (forward or reverse) if the tape motion is stopped by turning
 * the drive offline during a period of non data transfer (search)
 * The drive will detect a switch to offline if a data transfer
 * is in progress.
 * -ghg 7/16/77.
 */
tcck()
{
	if(tctick < 0)
		return;
	else {
		if(tctab.d_active && (tctick++ >= TCTIMEOUT)) {
			tcabtf++;
			TCADDR->tccm = SAT|IENABLE|GO;
		}
		if(tctick++ == 32767)
			tctick = 0;
		timeout(&tcck, 0, 60);
	}
}


tcclose(dev)
{
	bclose(dev, &tctab);	/* let dev finish and release all buffers */
#ifdef FLAKEYBUS
	if (tclock) {
		tclock = 0;
		devfree();
	}
#endif
	tcper[dev&07] = 0;
	tcopnf[dev&07] = 0;
}

tcstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;

	bp = abp;
	if(bp->b_flags&B_PHYS)
		mapalloc(bp);
#ifdef FLAKEYBUS
	if (!tclock) {
		devlock();
		tclock++;
	}
#endif
	if(bp->b_blkno >= NTCBLK || tcper[bp->b_dev&07]) {
		bp->b_flags =| B_ERROR;
		iodone(bp);
#ifdef FLAKEYBUS
		if (devlck & B_WANTED) { /* Dirty - fight fire with fire */
			tclock = 0;
			devfree();
		}
#endif
		return;
	}
	bp->av_forw = 0;
	spl6();
	if (tctab.d_actf==0)
		tctab.d_actf = bp;
	else
		tctab.d_actl->av_forw = bp;
	tctab.d_actl = bp;
	if (tctab.d_active==0)
		tcstart();
	spl0();
}

tcstart()
{
	register struct buf *bp;
	register int *tccmp, com;

loop:
	tccmp = &TCADDR->tccm;
	if ((bp = tctab.d_actf) == 0)
		return;
	if(tcper[bp->b_dev&07]) {
		if((tctab.d_actf = bp->av_forw) == 0)
			(*tccmp).lobyte = SAT|GO;
		bp->b_flags =| B_ERROR;
#ifdef FLAKEYBUS
		tclock = 0;
		devfree();
#endif
		iodone(bp);
		goto loop;
	}
	if (((*tccmp).hibyte&07) != bp->b_dev.d_minor)
		(*tccmp).lobyte = SAT|GO;
	tctab.d_errcnt = 20;
	tctab.d_active = SFORW;
	com = (bp->b_dev.d_minor<<8) | IENABLE|RNUM|GO;
	if ((TCADDR->tccsr & UPS) == 0) {
		com =| TREV;
		tctab.d_active = SREV;
	}
	*tccmp = com;
	tctick = 0; /* start timeout */
#ifdef KLUDGE
/* 
 * This code checks for the hardware failure caused by an intermittant
 * five volt power supply in the remote controller.  When this supply
 * drops out, the controller is always "ready" according to tccm.
 * no matter what is written in tccm, a 0200 is always read back.
 * This will cause the system to lock up waiting for buffers since
 * the function will never complete, also the abort timer cannot
 * force an interrupt to abort the buffer chain.  I really hate to
 * put this in, but the system has crashed twice because of this
 * problem.  The hardware staff has been unable to fix or obtain
 * a new five volt power supply in over four months.
 * --ghg 12/11/77
 */
	if (*tccmp == 0200) {
		errlg++;
	/*
	 * Ccontrol-A's in printf's to route to dectape terminal
	 */
printf("\001Dectape controller 5 volt power supply failure\n");
printf("\001Make sure both the drive and controller are on\n");
printf("\001If still dead, KICK the controller at the KICK HERE spot\n");
printf("\001If that fails, call ghg... good luck.\n");
		errlg = 0;
		tcper[bp->b_dev&07]++;
		tctab.d_active = 0;	/* controller hung, abort */
		goto loop;
	}
#endif
}

tcintr()
{
	register struct buf *bp;
	register int *tccmp;
	register int *tcdtp;

	tccmp = &TCADDR->tccm;
	tcdtp = &TCADDR->tccsr;
	if((bp = tctab.d_actf) == 0) return;
	tctick = 0;
	if(tcabtf) {
		tcabtf = 0;
		errlg++;
		printf("\001DECTAPE Hung - function aborted, BN=%d\n", bp->b_blkno);
		errlg = 0;
		goto err;
	}
	if (*tccmp&TAPERR) {
		if((*tcdtp & (PAR|MTE|DATM|NEXM)) || tcdbg) {
			errlg++; /* send printf to errlog device */
 printf("\001DECTAPE ERR - st%o cm%o wc%o ba%o dt%o bn%d rt%d\n",
 *tcdtp, *tccmp, TCADDR->tcwc, TCADDR->tcba, TCADDR->tcdt,
 bp->b_blkno, tctab.d_errcnt);
			errlg = 0;
		}
		if(*tcdtp & (ILGOP|SELERR)) {
err:			tcper[bp->b_dev&07]++;
			tctab.d_errcnt = 0;
			*tccmp =& ~TAPERR;
			bp->b_flags =| B_ERROR;
			goto done; /* no deverror on SEL or Writlock err */
		}
		*tccmp =& ~TAPERR;
		if (--tctab.d_errcnt  <= 0) {
			errlg++;
			deverror(bp,*tcdtp,*tccmp);
			errlg = 0;
			if (!tcignor)
				bp->b_flags =| B_ERROR;
			goto done;
		}
		if (*tccmp&TREV) {
		setforw:
			tctab.d_active = SFORW;
			*tccmp =& ~TREV;
		} else {
		setback:
			tctab.d_active = SREV;
			*tccmp =| TREV;
		}
		(*tccmp).lobyte = IENABLE|RNUM|GO;
		return;
	}
	tcdtp = &TCADDR->tcdt;
	switch (tctab.d_active) {

	case SIO:
	done:
		tctab.d_active = 0;
		if (tctab.d_actf = bp->av_forw)
			tcstart();
		else
			TCADDR->tccm.lobyte = SAT|GO;
		iodone(bp);
#ifdef FLAKEYBUS
		if ((devlck & B_WANTED) && tctab.d_actf == 0) {
			devfree();
			tclock = 0;
		}
#endif
		return;

	case SFORW:
		if (*tcdtp > bp->b_blkno)
			goto setback;
		if (*tcdtp < bp->b_blkno)
			goto setforw;
		*--tcdtp = bp->b_addr;		/* core address */
		*--tcdtp = bp->b_wcount;
		tccmp->lobyte = ((bp->b_xmem & 03) << 4) | IENABLE|GO
		    | (bp->b_flags&B_READ?RDATA:WDATA);
		tctab.d_active = SIO;
		return;

	case SREV:
		if (*tcdtp+3 > bp->b_blkno)
			goto setback;
		goto setforw;
	}
}

tcread(dev)
{
	physio(tcstrategy, &rtcbuf, dev, B_READ);
}

tcwrite(dev)
{
	physio(tcstrategy, &rtcbuf, dev, B_WRITE);
}
