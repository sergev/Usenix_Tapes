#

/*
 */

/*
 *	DZ-11 driver
 *	This driver is capable of handling
 *	more than one DZ-11 (i.e., more
 *	than eight lines).
 */

/*
 *	Modified 05/19/78 by Bill Shannon
 *
 *	Modem control mode added to use request-to-send (RTS)
 *	and clear-to-send (CTS).  Intended to allow printers
 *	and plotters to be driven by the DZ11, not for use
 *	with a real modem.  Understands printer box protocol,
 *	see lx.c driver.  Wiring is as follows:
 *
 *		RTS - pin 20 (DTR)
 *		CTS - pin 8  (Carrier)
 *
 *
 *	Modified 06/14/79 by Bill Shannon
 *
 *	Added code to use the DZ-11 receiver silo.  Code is enabled
 *	by defining symbol SILO.  The silo must be flushed every so
 *	often in case of low system activity when the silo doesn't
 *	fill up.  The procedure dzclock takes care of this.  The
 *	variable dz_on tells whether a dzclock is running for each
 *	DZ.  Dzsilo is called from the clock interrupt using the
 *	timeout mechanism.  Since the clock interrupts at level 6
 *	and the DZ interrupts at level 5, dzclock could interrupt
 *	dzrint.  The variable dz_lock is used to prevent this, so
 *	that only one process is executing the interrupt handler at
 *	a time.  Note that dzclock could also be used to watch for
 *	carrier transitions if a modem is used.
 */

#include "../param.h"
#include "../conf.h"
#include "../user.h"
#include "../tty.h"
#include "../proc.h"

#define DZADDR	0160100
#define NDZ11	16
#define	SILO	SILO		/* use silo alarm */
/*
 *	Hardware control bits
 */
#define RIE	0100		/* receiver interrupt enable */
#define TIE	040000		/* transmitter interrupt enable */
#define MSE	040		/* master scan enable */
#define SSPEED	015		/* standard speed: 9600 baud */
#define DVALID	0100000		/* data valid */
#define FRERROR	020000		/* framing error */
#ifdef	SILO
#define	SAE	010000		/* silo alarm enable */
#else
#define	SAE	0		/* no silo alarm */
#endif

#define	scan_on 	dz_addr->dztcr =| 1<<dz_line	/* enable scan */
#define	scan_off	dz_addr->dztcr =& ~(1<<dz_line)	/* disable scan */

struct tty dz11[NDZ11];

#ifdef	SILO
static char dz_on[NDZ11>>3], dz_lock[NDZ11>>3];
#endif

struct dzrregs {
	int dzcsr;
	int dzrbuf;
	int dztcr;
	int dzmsr;
};
struct dzwregs {
	int dzcsrw;
	int dzlpr;
	int dztcrw;
	int dztdr;
};

/*
 *	Table to convert DH11 speeds to DZ11 speeds.
 *	DH11 speed is used as index, DZ11 speed is value.
 */
char dzspeed[16] {
	016,000,001,002,	/* 9600,   50,   75,  110 */
	003,004,016,005,	/* 134.5, 150, 9600,  300 */
	006,007,010,012,	/*  600, 1200, 1800, 2400 */
	014,016,013,015		/* 4800, 9600, 3600, 7200 */
};

/*
 *	Open a DZ11 line.
 */
dzopen(dev,flag)
{
	register *dz_addr, dz_line;
	register struct tty *tp;
	int t;
	extern dzstart(), dzclock();

	if (dev.d_minor >= NDZ11) {
		u.u_error = ENXIO;
		return;
	}
	tp = &dz11[dev.d_minor];
	if (u.u_procp->p_ttyp == 0) {
		u.u_procp->p_ttyp = tp;
	}
	tp->t_dev = dev;
	tp->t_addr = dzstart;	/* special start routine */
	dz_addr = DZADDR + dev.d_minor&(~7);
	dz_line = dev.d_minor&7;
	if ((tp->t_state&ISOPEN) == 0) {
		tp->t_erase = CERASE;
		tp->t_kill = CKILL;
		tp->t_speeds = (SSPEED | (SSPEED<<8));
		tp->t_flags = XTABS | ECHO | CRMOD;
		dzparam(tp);
	}
#ifdef	SILO
	if (dz_on[dev.d_minor>>3] == 0) {
		timeout(dzclock, dev.d_minor>>3, 2);
		dz_on[dev.d_minor>>3]++;
	}
#endif
	t = dz_addr->dzcsr;	/* can't BIS the csr */
	dz_addr->dzcsr = t | RIE | TIE | MSE | SAE;
	tp->t_state =| ISOPEN | SSTART | CARR_ON;
}

/*
 *	Close a DZ11 line.
 */
dzclose(dev)
{
	register struct tty *tp;
	register *dz_addr, dz_line;

	tp = &dz11[dev.d_minor];
	tp->t_state =& (SSTART | CARR_ON);
	wflushtty(tp);
	tp->t_state = 0;
	dz_addr = DZADDR + dev.d_minor&(~7);
	dz_line = dev.d_minor&7;
	scan_off;
	dz_addr->dztcr.hibyte =& ~(1<<dz_line);	/* reset RTS for line */
}

/*
 *	Read from a DZ11 line.
 */
dzread(dev)
{
	ttread(&dz11[dev.d_minor]);
}

/*
 *	Write on a DZ11 line.
 */
dzwrite(dev)
{
	ttwrite(&dz11[dev.d_minor]);
}

/*
 *	DZ11 receiver interrupt.
 */
dzrint(dev)
{
	register struct tty *tp;
	register int c;
	register int dz_addr;
	int d;

	/* at this point dev.d_minor contains	*/
	/* the dz11 device number.		*/

#ifdef	SILO
	dz_lock[dev.d_minor]++;
#endif
	d = dev.d_minor<<3;
	dz_addr = DZADDR + d;
	while ((c = dz_addr->dzrbuf) & DVALID) {
		tp = &dz11[((c>>8)&7) + d];
		if (tp >= &dz11[NDZ11])
			continue;
		if (tp->t_state&ISOPEN) {
			if (c&FRERROR) {
				if (tp->t_flags&RAW)
					c = 0;
				else
					c = CINTR;
			}
			ttyinput(c&0377, tp);
		}
	}
#ifdef	SILO
	dz_lock[dev.d_minor] = 0;
#endif
}

/*
 *	DZ11 transmitter interrupt.
 */
dzxint(dev)
{
	extern ttrstrt();
	register struct tty *tp;
	register int c, dz_addr;
	int dz_line;

	dz_addr = DZADDR + 8*dev.d_minor;
	dz_line = dz_addr->dzcsr.hibyte&7;
	tp = &dz11[dz_line + 8*dev.d_minor];
	if (tp->t_state&(TIMEOUT|FREEZE)) {
		scan_off;
		return;
	}
	/* if MODEM mode is set, must have CTS; else timeout */
	if ((tp->t_flags&MODEM) && (dz_addr->dzmsr.hibyte&(1<<dz_line))==0) {
		scan_off;
		tp->t_state =| TIMEOUT;
		timeout(ttrstrt, tp, 60);
		return;
	}
	if ((c = getc(&tp->t_outq)) >= 0) {
		if (c <= 0177 || (tp->t_flags&RAW))
			dz_addr->dztdr = c&0377; /* make sure break bits */
		else {				/* don't get set.	*/
			tp->t_state =| TIMEOUT;
			scan_off;
			timeout(ttrstrt, tp, c&0177);
		}
	} else {
		scan_off;
	}
	if (tp->t_outq.c_cc <= TTLOWAT && tp->t_state&ASLEEP) {
		tp->t_state =& ~ASLEEP;
		wakeup(&tp->t_outq);
	}
}

/*
 *	Enable output on a given DZ11 line.
 */
dzstart(atp)
struct tty *atp;
{
	register struct tty *tp;
	register int dz_addr, dz_line;
	int sps;

	sps = PS->integ;
	spl5();
	tp = atp;
	if ((tp->t_state&(TIMEOUT|FREEZE)) == 0) {
		dz_addr = DZADDR + tp->t_dev.d_minor&~7;
		dz_line = tp->t_dev.d_minor&7;
		scan_on;
	}
	PS->integ = sps;
}

/*
 *	stty/gtty for DZ11.
 */
dzsgtty(dev, av)
int *av;
{
	register struct tty *tp;

	tp = &dz11[dev.d_minor];
	if (ttystty(tp, av))
		return;
	dzparam(tp);
}

/*
 *	Set parameters from open or stty
 *	into the DZ11 hardware registers.
 */
dzparam(atp)
struct tty *atp;
{
	register struct tty *tp;
	register int lpr, *dz_addr;
	int speed, dz_line;

	spl5();
	tp = atp;
	speed = dzspeed[tp->t_speeds&017];
	lpr = 010030 | (speed<<8) | (tp->t_dev.d_minor&7);
	if (speed == 2)	/* 110 baud */
		lpr =| 040;	/* 2 stop bits */
	dz_addr = DZADDR + tp->t_dev.d_minor&~7;
	dz_addr->dzlpr = lpr;
	/* if modem control requested, turn on RTS */
	dz_line = tp->t_dev.d_minor&7;
	if (tp->t_flags&MODEM) {
		dz_addr->dztcr.hibyte =| 1<<dz_line;	/* set RTS for line */
	} else {
		dz_addr->dztcr.hibyte =& ~(1<<dz_line);	/* reset RTS for line */
	}
	spl0();
}


#ifdef	SILO
/*
 *	empty the DZ11 silo every so often
 */
dzclock(dev)
{
	if (dz_lock[dev] == 0)
		dzrint(dev);		/* simulate receiver interrupt */
	timeout(dzclock, dev, 2);	/* restart 'clock' */
}
#endif
