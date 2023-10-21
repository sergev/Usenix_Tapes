#
/*
 * DR11-B driver (Grinnell display processor)
 *
 * With PDP-11/70 host, CPU70 should be defined in param.h
 * Assumes either FNCT2 (D04H1) or FNCT3 (D04J2)
 * has been wired to ATTN on paddle board (M957)
 * for user device connections in DR11-B and
 * corresponding grounds are disconnected.
 * This modification is needed to stop the DR11-B
 * from waiting for input from the Grinnell
 * by resetting ready (1) (bit 7 in DRCS).
 */

#define GRADDR  0172430

#include "/usr/sys/param.h"
#include "/usr/sys/buf.h"
#include "/usr/sys/conf.h"
#include "/usr/sys/user.h"

struct {
	int grwc;
	int grba;
	int grcs;
	int grdb;
	};

#define TIMOUT  001     /* timeout in progress */
#define FORRED  002     /* foreground read in progress */
#define RETRY   004     /* restart timeout */

#define WAITING 00400   /* waiting for background read */
#define BCKGRD  01000   /* background I/O in progress */

#define OPMASK  0170000 /* Grinnel op code mask */
#define SPD     0120000 /* select peripheral device */
#define INTRP   0002000 /* cursor with interrupts peripheral device */
#define TV      0000002 /* keep camera digitizer selected */
#define RPD     0160000 /* readback peripheral data */
#define RBLEN   2       /* background pre-read command buffer length */
#define RTIME   3       /* clock ticks allowed for foreground read */
#define GRPRIO  21      /* background wait priority */

struct {
	int flags;
	struct  buf *buff;      /* active I/O buffer loc or 0 */
	int rbuf[RBLEN];        /* start of background read commands */
	int word;               /* bytes of readback word buffer */

	} grtab {

	0,
	0,
	SPD | INTRP | TV, RPD,
	0,
	};

struct  buf     grbuf;

#define	GO	01
#define WRITE   00
#define READ    02
#define STOPIO  014
#define	IENABLE	0100
#define	READY	0200
#define	CYCLE	0400
#define	ATTN	020000
#define	NEX	040000

/*
 *      Use nulldev instead of
gropen()
{
}
 */

/*
 *      Use nulldev instead of
grclose(dev, flag)
{
}
 */

grstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;
	register int *i;
	register char *cnt;

	bp = abp;

	if(bp != &grbuf) { /* Only one buffer is legal here */
		bp->b_flags = B_ERROR;
		printf("Grinnell\n"); /* this is a system error */
		iodone(bp);
		return;
	}
	cnt = bp->b_wcount;
	if(bp->b_flags & B_READ) {
		i = u.u_base;
		while(++cnt) if ((fuword(i++) & OPMASK)
			== RPD) break;
		if(!cnt) {
			bp->b_flags =| B_ERROR;
			iodone(bp);
			return;
		}
		bp->b_resid = cnt; /* save read count */
		cnt = bp->b_wcount - cnt; /* write before read word count */
	}
	/* Only for the PDP 11/70 */
#ifdef CPU70
	mapalloc(bp);
#endif CPU70
	spl6();
/*
 * Stop background I/O
 */
	if(grtab.flags & BCKGRD) {
		GRADDR->grcs = STOPIO | READ;
	}
	grtab.flags =& ~BCKGRD;
	grtab.buff = bp;        /* Remember buffer of foreground I/O */
	GRADDR->grwc = cnt;     /* words to transfer */
	GRADDR->grba = bp->b_addr;  /* Get the core address */
	GRADDR->grcs = ((bp->b_xmem & 3) << 4)
		/* | CYCLE */
		| IENABLE | WRITE | GO;  /* start command */
	spl0();
}

/*
 * GRCALOUT is actived by the clock by means of the timeout procedures
 * to make sure that reading from the Grinnell is not stalled
 * due to user program errors which try to read with bad preparations.
 * At most one GRCALOUT call should be active.
 * If no foreground reading is in progress, GRCALOUT dies quietly.
 * If a foreground read is in progress,
 * GRCALOUT stops it prematurely so that the system will not stall.
 * If a second foreground read needs to be timed
 * while a call for GRCALOUT is pending,
 * it sets RETRY so that GRCALOUT will give another time quantum.
 */

grcalout(arg)
{
	register tab;

	tab = &grtab;
	spl6();
	if(tab->flags & RETRY) { /* more time was requested */
		tab->flags =& ~RETRY;
		timeout(grcalout, 0, RTIME);
	} else {
		tab->flags =& ~TIMOUT;
		if(tab->flags & FORRED) {
		/* Stop stalled foreground read */
			GRADDR->grcs = STOPIO | READ; /* turn off interrupt */
			GRADDR->grcs = READ;
			grintr();       /* simulate read done condition */
		}
	}
}

/*
 * The interrupt routine must operate at priority 6
 * to lock out clock interrupts.
 */

grintr(dev)
{
	register struct buf *bp;
	register cs, tab;

	tab = &grtab;
	spl6();
	cs = GRADDR->grcs;
	if((bp = grtab.buff) == 0) {
		if(tab->flags & BCKGRD && GRADDR->grwc == 0) {
			if (!(cs & (READ | STOPIO))) {
			/* now ready to do background interrupt read */
				GRADDR->grcs = READ; /* Preset function bit */
				GRADDR->grwc = -1;
				GRADDR->grcs = IENABLE | READ | GO;
			} else {
				tab->flags =& ~(BCKGRD | WAITING);
				wakeup(tab);
			}
		}
		return;
	}
	if(cs < 0) {  /* Error bit in control and status set */
		if((cs & (ATTN | NEX))==0 && GRADDR->grba == 0) {
			GRADDR->grba = 0; /* Reset overflow error bit */
			cs = (cs & (WRITE | READ | 060)) + 020;
			if(GRADDR->grwc != 0) { /* continue transfer */
				GRADDR->grcs = cs | IENABLE | GO;
				return;
			}
		} else {
			bp->b_flags =| B_ERROR;
			goto done;
		}
	}
	if(!(cs & READ) && bp->b_flags & B_READ) {
		GRADDR->grcs = READ;    /* Preset function bit */
		GRADDR->grwc = bp->b_resid;
		GRADDR->grcs = (060 & cs) |
			IENABLE | READ | GO;
		/* kill a foreground read that takes too long */
		if(tab->flags & TIMOUT) {
			tab->flags =| RETRY;
		} else {
			tab->flags =| TIMOUT;
			timeout(grcalout, 0, RTIME);
		}
		tab->flags =| FORRED;
		return;
	}
  done: grtab.buff = 0; /* allow background I/O */
	/* let any timeout die quietly */
	tab->flags =& ~(FORRED | RETRY);
	bp->b_resid = GRADDR->grwc;
	grbackgr();
	iodone(bp);
}

grread(dev)
{
	register struct buf *bp;

	bp = &grbuf;
	if(u.u_count == 2) {
		spl6();
		while(bp->b_flags & B_BUSY) {
			bp->b_flags =| B_WANTED;
			sleep(bp, GRPRIO);
		}
		grtab.word.integ = 0;
		grtab.flags =| WAITING;
		/* wait for an asynchronous interrupt */
		grbackgr();
		sleep(&grtab.flags, GRPRIO);
		if(passc(grtab.word.integ) == 0) /* low byte */
			passc(grtab.word.hibyte);
		spl0();
	} else
		physio(grstrategy, bp, dev, B_READ);
}

grwrite(dev)
{
	physio(grstrategy, &grbuf, dev, B_WRITE);
}

/*
 * In order to avoid keeping programs locked in core waiting
 * for asynchronous interrupts (ENTER CURSOR or TRACK CURSOR),
 * GRBACKGR sets up the Grinnell to wait for asynchronous interrupts
 * during the periods that no other I/O is active.
 * This background I/O is preempted whenever a request for
 * foreground I/O occurs and restarted when the foreground I/O is done.
 * Processes initiate waiting for the interrupt by requesting a read
 * of exactly two bytes.
 * Multiple processes may wait for the same interrupt.
 * When an interrupt occurs all waiting processes are continued.
 * Each must determine if the interrupt was meant for it.
 */

grbackgr()
{
	register tab;

	tab = &grtab;
	if(tab->flags & WAITING && !(tab->flags & BCKGRD)) {
		tab->flags =| BCKGRD;
		GRADDR->grwc = -RBLEN;
		GRADDR->grba = &grtab.rbuf[0];
		GRADDR->grcs =
			/* CYCLE | */
			IENABLE | WRITE | GO;
	}
}
