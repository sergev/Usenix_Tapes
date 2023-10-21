/*
 * DMF32 driver
 *
 * From Chris Maloney: MDDC Version 1.00 June 23, 1984
 * Modified by Chris Torek, U of M, for 4.1BSD and dial out (``dmfo'' device)
 * and for #define EMULEX garbage
 *
 * Converted back to 4.2BSD 29 Mar 1985, Chris Torek
 * Formatting completely revised 15 Apr 1985, Chris Torek
 *
 * TODO:
 *	test reset code
 *
 * DMF32 line printer driver
 * The line printer on dmfx is indicated by a minor device code of 128+x.
 *
 *
 * The flags field of the config file is interpreted as:
 *   bits	meaning
 *   ----	-------
 *   0-7	soft carrier bits for ttys part of dmf32
 *   8-15	number of cols/line on the line printer
 *		if 0, 132 will be used.
 *   16-23	number of lines/page on the line printer
 *		if 0, 66 will be used.
 */

#include "dmf.h"
#if NDMF > 0
#include "dmfo.h"

/* Define this if you have Emulex CS21Fs *and no DEC DMFs*, then change your
   config file to list only "dmfrint dmfxint" as the interrupt vectors.  This
   also disables the line printer driver. */
/* #define EMULEX */

#include "../machine/pte.h"

#include "bk.h"
#include "../h/param.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/ioctl.h"
#include "../h/tty.h"
#include "../h/uio.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/vm.h"
#include "../h/bk.h"
#include "../h/clist.h"
#include "../h/file.h"

#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"
#include "../vaxuba/dmfreg.h"

/*
 * SILO TIME OUT constant:  if you get data overruns, or have problems with
 * XON/XOFF not responing quick enough, set this lower, but keep it at least
 * 1.  If you make this 0, you are in trouble.
 */
#define SILO_TIME_OUT	3

/*
 * DO DMA COUNT constant:  number of characters at which driver uses DMA mode.
 * DEC uses 32, but this is too high for Unix.  (I think VMS has to remap UBA
 * memory to do the DMA.)  Setting this to 0 disables silo mode.
 *
 * N.B.:  You probably want to increase CBSIZE and CROUND in param.h.
 * We have found clist size = 128 to be about right (CBSIZE=124, CROUND=127).
 */
#define DO_DMA_COUNT	16

/*
 * Definition of the driver for the auto-configuration program.
 */
int     dmfprobe(), dmfattach(), dmfrint(), dmfxint();
#ifndef EMULEX
int	dmflint();
#endif
struct  uba_device *dmfinfo[NDMF];
u_short dmfstd[] = { 0160340, 0 };
struct uba_driver dmfdriver =
	{ dmfprobe, 0, dmfattach, 0, dmfstd, "dmf", dmfinfo };

/*
 * Local variables for the driver
 */
int     dmfact;			/* mask of active dmf's */

#ifndef EMULEX
/* states for line printer (1 per DMF) */
#define ASLP		1	/* waiting for interrupt from dmf */
#define OPEN		2	/* line printer is open */
#define ERROR		4	/* error while printing; driver refuses to
				   do anything till closed */
#define MORETODO	8	/* more output on the way */

struct dmfl_softc {
	short	sc_state;	/* soft state bits */
	u_char	sc_lines;	/* lines per page (66 def.) */
	u_char	sc_cols;	/* cols per line (132 def.) */
	u_int	sc_info;	/* uba info */
	char	sc_buf[DMFL_SIZ];
} dmfl_softc[NDMF];
#endif

char   dmf_speeds[] = { 0, 0, 1, 2, 3, 4, 0, 5, 6, 7, 8, 10, 12, 14, 15, 15};
int    dmfstart();

#ifndef lint
int    ndmf = NDMF * 8;		/* used by iostat, among others */
#endif

int    ttrstrt();
struct tty dmf_tty[NDMF * 8];

char   dmfsoftCAR[NDMF];	/* soft carrier flags */
#if NDMFO > 0
char   dmf_inout[NDMF];		/* outgoing mode flags */
char   dmf_flags[NDMF];		/* permanent copy of flags */
#endif

/* states for async lines (8 per DMF) */
#define ST_TXOFF	0x01	/* tx turned off */
#define ST_DMA		0x02	/* dma operation inprogress */
#define ST_INBUSY	0x04	/* stop tx in busy */
struct dmfa_softc {
	short	sc_state;	/* dmf state */
	short	sc_count;	/* dmf dma count */
} dmfa_softc[NDMF * 8];


/*
 * The clist space is mapped by the driver onto each UNIBUS.  The UBACVT
 * macro converts a clist space address for UNIBUS uban into an I/O space
 * address for the DMA routine.  <<Ripped off from dh.c>>
 */
int     dmf_ubinfo[MAXNUBA];	/* info about allocated unibus map */
static int cbase[MAXNUBA];	/* base address in unibus map */
#define	UBACVT(x,uban) (cbase[uban] + ((x) - (char *)cfree))

extern int hz;			/* clock frequency */

/*
 * Routine for configuration to set dmf interrupt.
 */
/* ARGSUSED */
dmfprobe(reg, ctlr)
	caddr_t reg;
	int ctlr;
{
	register int br, cvec;	/* these are ``value-result'' */
	register struct dmfdevice *dmf_addr = (struct dmfdevice *) reg;
	register int i;

#ifdef lint
	br = 0; cvec = br; br = cvec;
	dmfxint (0); dmfrint (0); dmfsrint (); dmfsxint ();
	dmfdaint (); dmfdbint (); dmflint ();
#endif

	/*
	 * Interupt level is 15.  We get to chose our own vector, and then
	 * tell the controller what it is.
	 */
	br = 0x15;
#ifndef EMULEX
	cvec = (uba_hd[numuba].uh_lastiv -= 8 * 4);
	dmf_addr->dmfc_config = cvec >> 2;
#else
	cvec = (uba_hd[numuba].uh_lastiv -= 2 * 4);
	dmf_addr->dmfc_config = (cvec - 020) >> 2;
#endif
	if ((dmf_addr->dmfc_config & 0x8000) == 0) {
		printf("no DMF async flag!?\n");
		return (0);
	}
	dmf_addr->dmfl_ctrl = DMFL_RST;
	return (sizeof (struct dmfdevice));
}

/*
 * Routine to "attach" a DMF.  Get columns, lines and other device dependent
 * information for the specific device.
 */
dmfattach(ui)
	register struct uba_device *ui;
{
#ifndef EMULEX
	register int cols, lines;

	cols = (ui->ui_flags >> 8) & 0xff;
	lines = (ui->ui_flags >> 16) & 0xff;
	dmfl_softc[ui->ui_unit].sc_cols = (cols == 0 ? DMFL_DCL : cols);
	dmfl_softc[ui->ui_unit].sc_lines = (lines == 0 ? DMFL_DLN : lines);
#endif
#if NDMFO == 0
	dmfsoftCAR[ui->ui_unit] = ui->ui_flags & 0xff;
#else
	dmfsoftCAR[ui->ui_unit] = dmf_flags[ui->ui_unit] =
		ui->ui_flags & 0xff;
#endif
}

/*
 * Open a DMF32 line, mapping the clist onto the UBA if this is the first
 * DMF on this UBA.  Turn on this DMF if this is the first use of it.
 */
/* ARGSUSED */
dmfopen(dev, flag)
	dev_t dev;
	int flag;
{
	register struct tty *tp;
	register int unit, ctlr;
	register struct dmfdevice *dmf;
	register struct uba_device *ui;
	int s, bit;

	unit = minor(dev);
#ifndef EMULEX
	if (unit & 0200)	/* printer open request */
		return (dmflopen(dev, flag));
#endif
	ctlr = unit >> 3;
	if (unit >= NDMF * 8 || (ui = dmfinfo[ctlr]) == NULL || !ui->ui_alive)
		return (ENXIO);
	tp = &dmf_tty[unit];
	dmf = (struct dmfdevice *)ui->ui_addr;
	tp->t_addr = (caddr_t)dmf;
	tp->t_oproc = dmfstart;

	/*
	 * While setting up state for this UBA+DMF, block UBA resets which can
	 * clear the state.
	 */
	s = spl5();
	if (dmf_ubinfo[ui->ui_ubanum] == 0) {
		dmf_ubinfo[ui->ui_ubanum] = uballoc(ui->ui_ubanum,
			(caddr_t)cfree, nclist * sizeof (struct cblock), 0);
		cbase[ui->ui_ubanum] = dmf_ubinfo[ui->ui_ubanum] & 0x3ffff;
	}
	bit = 1 << ctlr;
	if ((dmfact & bit) == 0) {
		dmfact |= bit;
		dmf->dmfa_csr |= DMFA_IE;
		dmf->dmfa_sato = SILO_TIME_OUT;
	}
	splx(s);

	/*
	 * If this is first open, initialze tty state to default.
	 */
	if ((tp->t_state&TS_ISOPEN) == 0) {
		ttychars(tp);
		if (tp->t_ispeed == 0) {
			tp->t_ispeed = tp->t_ospeed = B2400;
			tp->t_flags = ODDP | EVENP | ECHO;
		}
		dmfparam(unit);
		dmfa_softc[unit].sc_state = 0;
	}
	else {
#if NDMFO == 0
		if (tp->t_state&TS_XCLUDE && u.u_uid != 0)
			return (EBUSY);
#else
		if ((tp->t_state&TS_XCLUDE || flag < 0) && u.u_uid != 0)
			return (EBUSY);
#endif
	}
	/*
	 * Turn on the line and wait for a carrier.  Carrier tests must be
	 * done with dmfrint held, since carrier transitions look like input.
	 */
#if NDMFO == 0
	if (dmfmctl(dev, DMFA_ON, DMSET) & (DMFA_CAR << 8))
		tp->t_state |= TS_CARR_ON;
	s = spl5();
	while ((tp->t_state&TS_CARR_ON) == 0) {
		tp->t_state |= TS_WOPEN;
		sleep((caddr_t)&tp->t_rawq, TTIPRI);
	}
#else
	bit = 1 << (unit & 7);
	if (flag < 0) {
		dmf_inout[ctlr] |= bit;	/* outgoing mode on */
		dmfsoftCAR[ctlr] |= bit;/* fake carrier on */
	}
	s = spl5();
	for (;;) {		/* keep turning it on */
		if (dmfmctl(dev, DMFA_ON, DMSET) & (DMFA_CAR << 8))
			tp->t_state |= TS_CARR_ON;
		if (tp->t_state&TS_CARR_ON &&
		    ((dmf_inout[ctlr]&bit) == 0 || flag < 0))
			break;
		tp->t_state |= TS_WOPEN;
		sleep((caddr_t)&tp->t_rawq, TTIPRI);
	}
#endif
	splx(s);
	return ((*linesw[tp->t_line].l_open)(dev, tp));
}

/*
 * Set parameters from open or stty into the DMF hardware registers.
 * Block interrupts so parameters will be set before the line interrupts.
 * If terminal speed is set to 0, hang up on last close, turm modem off
 * and return to caller.
 */
dmfparam(unit)
	register int unit;
{
	register struct tty *tp;
	register struct dmfdevice *dmf;
	register int lparm, lctl;
	int s;

	tp = &dmf_tty[unit];
	dmf = (struct dmfdevice *) tp->t_addr;

	s = spl5();
	if (tp->t_ispeed == 0) {
		tp->t_state |= TS_HUPCLS;
		(void) dmfmctl(unit, DMFA_OFF, DMSET);
		splx(s);
		return;
	}

	/*
	 * Set up line parameters and control info.  Emulex CS21s don't
	 * support split baud rates properly so we just use the output speed.
	 */
#ifndef EMULEX
	lparm = (dmf_speeds[tp->t_ospeed]<<12)|(dmf_speeds[tp->t_ispeed]<<8);
#else
	lparm = dmf_speeds[tp->t_ospeed] << 8;
	lparm |= lparm << 4;
#endif
	lctl = DMFA_LCE;
	if (tp->t_ispeed == B134)
		lparm |= DMFA_6BT | DMFA_PEN;
	else if (tp->t_flags & (RAW|LITOUT))
		lparm |= DMFA_8BT;
	else
		lparm |= DMFA_7BT | DMFA_PEN;
	if (tp->t_flags & EVENP)
		lparm |= DMFA_EPR;
	if (tp->t_ospeed == B110)
		lparm |= DMFA_SCD;

	unit &= 7;
	dmf->dmfa_lparm = lparm | unit;
	dmf->dmfa_csr = DMFA_IE | IR_LCTMR | unit;
	dmf->dmfa_lctmr |= lctl & 0xff;
	splx(s);
}

#if NDMFO > 0
/*
 * Finish closing a DMF line (after holding DTR low)
 */
dmfpause(tp)
	struct tty *tp;
{

	wakeup((caddr_t)&tp->t_state);
}
#endif

/*
 * Close a DMF32 line.
 */
dmfclose(dev, flag)
	dev_t dev;
	int flag;
{
	register struct tty *tp;
	register int unit;

	unit = minor(dev);
	tp = &dmf_tty[unit];

#ifndef EMULEX
	if (unit & 0200) {
		dmflclose(dev, flag);
		return;
	}
#endif
	(*linesw[tp->t_line].l_close)(tp);

	/*
	 * un-break, hang-up and close the modem.
	 */
	(void) dmfmctl(unit, DMFA_BRK, DMBIC);
#if NDMFO == 0
	if (tp->t_state&TS_HUPCLS || (tp->t_state&TS_ISOPEN) == 0)
		(void) dmfmctl(unit, DMFA_OFF, DMSET);
	ttyclose(tp);
#else
	if (tp->t_state&TS_HUPCLS || (tp->t_state&TS_ISOPEN) == 0 ||
	    flag < 0) {
		(void) dmfmctl(unit, DMFA_OFF, DMSET);
		/*
		 * Force DTR to remain low for ~1/2 second
		 */
		timeout(dmfpause, (caddr_t)tp, hz >> 1);
		sleep((caddr_t)&tp->t_state, TTOPRI);
	}
	ttyclose(tp);
	if (flag < 0) {		/* line was open in outgoing mode */
		register int dmf = unit >> 3;
		register int bit = 1 << (unit & 7);

		dmf_inout[dmf] &= ~bit;
		if (dmf_flags[dmf] & bit)	/* restore softCAR */
			dmfsoftCAR[dmf] |= bit;
		else
			dmfsoftCAR[dmf] &= ~bit;
		wakeup((caddr_t)&tp->t_rawq);
	}
#endif NDMFO == 0
}

/*
 * Reset state of driver if UBA reset was necessary.
 * Reset the csr, lpr, and lctmr registers on open lines,
 * and restart transmitters.
 */
dmfreset(uban)
	int uban;
{
	register int ctlr, unit;
	register struct tty *tp;
	register struct dmfdevice *dmf;
	register struct uba_device *ui;
	register struct dmfa_softc *sc;
	int i;

	/* 
	 * Set up Unibus map registers.
	 */
	if (dmf_ubinfo[uban] == 0)
		return;
	dmf_ubinfo[uban] = uballoc(uban, (caddr_t)cfree,
		nclist * sizeof (struct cblock), 0);
	cbase[uban] = dmf_ubinfo[uban] & 0x3ffff;
	for (ctlr = 0; ctlr < NDMF; ctlr++) {
		ui = dmfinfo[ctlr];
		if (ui == 0 || ui->ui_alive == 0 || ui->ui_ubanum != uban)
			continue;
		printf (" dmf%d", ctlr);
		dmf = (struct dmfdevice *)ui->ui_addr;
		dmf->dmfa_csr |= DMFA_IE;
		dmf->dmfa_sato = SILO_TIME_OUT;
		/* 
		 * For each unit on a dmf controller, if the unit is open
		 * or waiting for open to complete, reset it.
		 */
		unit = ctlr << 3;
		sc = &dmfa_softc[unit];
		tp = &dmf_tty[unit];
		for (i = 0; i < 8; i++) {
			sc->sc_state = 0;
			if (tp->t_state & (TS_ISOPEN|TS_WOPEN)) {
				dmfparam(unit);
				(void) dmfmctl(unit, DMFA_ON, DMSET);
				tp->t_state &= ~TS_BUSY;
				dmfstart(tp);
			}
			sc++, unit++, tp++;
		}
	}
}

/*
 * Routine to read a dmf32 line.
 */
dmfread(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp;

#ifndef EMULEX
	if (minor(dev) & 0200)
		return (ENXIO);
#endif
	tp = &dmf_tty[minor(dev)];
	return ((*linesw[tp->t_line].l_read)(tp, uio));
}

/*
 * Routine to write to a dmf32 line.
 */
dmfwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp;

#ifndef EMULEX
	if (minor(dev) & 0200)
		return (dmflwrite(dev, uio));
#endif
	tp = &dmf_tty[minor(dev)];
	return ((*linesw[tp->t_line].l_write)(tp, uio));
}

/*
 * Routine to process a DMF32 receiver interrupt.
 */
dmfrint(ctlr)
	int ctlr;
{
	register int c;
	register struct tty *tp, *tp0;
	register struct dmfdevice *dmf;
	register struct uba_device *ui;
	int overrun, unit, s;

	overrun = 0;
	if ((ui = dmfinfo[ctlr]) == NULL || ui->ui_alive == 0)
		return;
	dmf = (struct dmfdevice *)ui->ui_addr;
	tp0 = &dmf_tty[ctlr << 3];

	/*
	 * Loop fetching characters from the silo for this
	 * dmf until there are no more in the silo.
	 */
	while ((c = dmf->dmfa_rbuf) & DMFA_DV) {
		unit = (c >> 8) & 7;
		tp = &tp0[unit];
		if (c & DMFA_DSC) {
			dmfdsc(ctlr, unit, dmf, tp);
			continue;
		}
		if ((tp->t_state&TS_ISOPEN) == 0) {
			wakeup((caddr_t)tp);
			continue;
		}
		if (c & DMFA_PE) {
			if ((tp->t_flags & (EVENP|ODDP)) == EVENP ||
				(tp->t_flags & (EVENP|ODDP)) == ODDP)
			continue;
		}
		if ((c & DMFA_DO) && overrun == 0) {
			printf ("dmf%d: silo overflow\n", ctlr);
			overrun++;
		}
		if (c & DMFA_FE) {
			if (tp->t_flags & RAW)
				c = 0;
			else
				c = tp->t_intrc;
		}
#if NBK > 0
		if (tp->t_line == NETLDISC) {
			c &= 0177;
			BKINPUT(c, tp);
		}
		else
#endif
		(*linesw[tp->t_line].l_rint)(c, tp);
	}
}

/*
 * Process a carrier state change.  Find out what the real carrier
 * state is; if on, and in use outgoing, clear soft carrier.
 * N.B.: no MDMBUF-style flow control, sorry...
 */
dmfdsc(ctlr, unit, dmf, tp)
	register int ctlr, unit;
	register struct dmfdevice *dmf;
	register struct tty *tp;
{
	int realc, bit;

	bit = 1 << unit;
	dmf->dmfa_csr = DMFA_IE | IR_RMS | unit;
	realc = dmf->dmfa_rms & DMFA_CAR;
	if (realc) {
#if NDMFO > 0
		if (dmf_inout[ctlr] & bit)
			dmfsoftCAR[ctlr] &= ~bit;
#endif
		if ((tp->t_state&TS_CARR_ON) == 0)
			tp->t_state |= TS_CARR_ON;
		wakeup((caddr_t)&tp->t_rawq);
	}
	else {
		if ((dmfsoftCAR[ctlr] & bit) == 0 && tp->t_state&TS_CARR_ON) {
			gsignal(tp->t_pgrp, SIGHUP);
			gsignal(tp->t_pgrp, SIGCONT);
			dmf->dmfa_csr = DMFA_IE | IR_LCTMR | unit;
			dmf->dmfa_lctmr &= (DMFA_OFF << 8) | 0xff;
			ttyflush(tp, FREAD|FWRITE);
			tp->t_state &= ~TS_CARR_ON;
		}
	}
}

/*
 * Start (restart) transmission on the given DMF32 line.
 */
dmfstart(tp)
	register struct tty *tp;
{
	register struct dmfdevice *dmf;
	register struct dmfa_softc *sc;
	register int unit, nch;
	int ctlr, use_dma, s;

	unit = minor(tp->t_dev);
	ctlr = unit >> 3;
	dmf = (struct dmfdevice *)tp->t_addr;
	sc = &dmfa_softc[unit];
	unit &= 7;
	s = spl5();
	/*
	 * If it's currently active, or delaying, no need to do anything.
	 */
	if (tp->t_state & (TS_TIMEOUT|TS_BUSY|TS_TTSTOP))
		goto out;

	/*
	 * If the transmitter has been disabled reenable it.
	 * If the transmitter was disabled before the interrupt
	 * (BUSY was still on), then reset the BUSY state and
	 * we will wait for the interrupt.  If !BUSY we already
	 * saw the interrupt so we can start another transmission.
	 */
	if (sc->sc_state & ST_TXOFF) {
		dmf->dmfa_csr = DMFA_IE | IR_LCTMR | unit;
		dmf->dmfa_lctmr |= DMFA_TE;
		sc->sc_state &= ~ST_TXOFF;
		if (sc->sc_state & ST_INBUSY) {
			sc->sc_state &= ~ST_INBUSY;
			tp->t_state |= TS_BUSY;
			goto out;
		}
	}

	/*
	 * If there are sleepers, and output has drained below low
	 * water mark, wake 'em up.
	 */
	if (tp->t_outq.c_cc <= TTLOWAT(tp)) {
		if (tp->t_state&TS_ASLEEP) {
			tp->t_state &= ~TS_ASLEEP;
			wakeup((caddr_t)&tp->t_outq);
		}
		if (tp->t_wsel) {
			selwakeup(tp->t_wsel, tp->t_state & TS_WCOLL);
			tp->t_wsel = 0;
			tp->t_state &= ~TS_WCOLL;
		}
	}

	/*
	 * Now restart transmission unless the output queue is empty.
	 */
	if (tp->t_outq.c_cc == 0)
		goto out;
	if (tp->t_flags & (RAW|LITOUT))
		nch = ndqb(&tp->t_outq, 0);
	else {
		/*
		 * If first thing on queue is a delay process it.
		 */
		nch = ndqb(&tp->t_outq, 0200);
		if (nch == 0) {
			nch = getc(&tp->t_outq);
			timeout(ttrstrt, (caddr_t)tp, (nch&0x7f) + 6);
			tp->t_state |= TS_TIMEOUT;
			goto out;
		}
	}

	/*
	 * If characters to transmit, (re)start transmission:
	 * enable transmitter and interrupts and set the mode to busy;
	 * pick DMA or SILO mode based on the number of characters to
	 * transmit and the amount of space left in the silo.
	 * For DMA:
	 *	Calculate the DMA address; load it and the character count
	 *	into the DMF.  Have to wait for the transmit interrupt before
	 *	flushing.
	 * For non DMA:
	 *	Move characters into the silo (in pairs when possible) and
	 *	flush them from the queue.
	 */
	if (nch) {
		dmf->dmfa_csr = DMFA_IE | IR_LCTMR | unit;
		dmf->dmfa_lctmr |= DMFA_TE;
		tp->t_state |= TS_BUSY;
		use_dma = 0;
		if (nch >= DO_DMA_COUNT)
			use_dma++;
		else {
			register int room;

			dmf->dmfa_csr = DMFA_IE | IR_TSC | unit;
			room = DMFA_SIZ - dmf->dmfa_tsc;
			if (room < nch)
				use_dma++;
		}
		if (use_dma) {
			register int car;
	    
			car = UBACVT(tp->t_outq.c_cf, dmfinfo[ctlr]->ui_ubanum);
			sc->sc_count = nch;
			sc->sc_state |= ST_DMA;
			dmf->dmfa_csr = DMFA_IE | IR_TBA | unit;
			dmf->dmfa_tba = car;
			dmf->dmfa_tcc = ((car >> 2) & 0xc000) | nch;
		}
		else {
			sc->sc_state &= ~ST_DMA;
			dmf->dmfa_csr = DMFA_IE | IR_TBUF | unit;
			unit = nch;
#ifdef notdef
			/*
			 * I'm feeling particularly register- and
			 * instruction-thrifty today . . . guess
			 * what this code does!
			 */
			{ register char *cp = tp->t_outq.c_cf;
			  if ((int)cp & 1)
				dmf->dmfa_tbf = *cp++, unit--;
			}
			{ register short *cp;
			  while ((unit -= 2) >= 0)
				dmf->dmfa_tbf2 = *cp++;
			  if (unit & 1)
				dmf->dmfa_tbf = *(char *)cp;
			}
#else
			/*
			 * However, I'm also prepared to be reasonable.
			 */
			{ register short *cp = (short *)tp->t_outq.c_cf;
			  while ((unit -= 2) >= 0)
				dmf->dmfa_tbf2 = *cp++;
			  if (unit & 1)
				dmf->dmfa_tbf = *(char *)cp;
			}
#endif
			ndflush(&tp->t_outq, nch);
		}
	}
out: 
	splx(s);
}

/*
 * Stop output on a line, e.g. for ^S/^Q or output flush.  Block I/O
 * interrupts (must use spl6(), not spl5(), as the stop routine is
 * called from flushtty() at spl6).  Disable the transmitter.  We must
 * disable the transmitter even if tp->t_state says the terminal is not
 * busy, because in DMA mode, there are characters left in the silo after
 * the controller interrupts with transmit complete.  If flush request
 * (i.e. a non stopped-by-^S request), then flush the silo by setting
 * the appropriate bit.  This will cause a transmit interrupt when the
 * flush is complete so mark it busy as well.  Otherwise, if busy, mark
 * as stopped while busy.
 */
dmfstop(tp)
	register struct tty *tp;
{
	register struct dmfdevice *dmf;
	register struct dmfa_softc *sc;
	register int unit;
	int s;
    
	dmf = (struct dmfdevice *)tp->t_addr;
	unit = minor(tp->t_dev);
	sc = &dmfa_softc[unit];
	s = spl6();
	dmf->dmfa_csr = IR_LCTMR | (unit & 7) | DMFA_IE;
	dmf->dmfa_lctmr &= ~DMFA_TE;
	sc->sc_state |= ST_TXOFF;
	if ((tp->t_state&TS_TTSTOP) == 0) {
		tp->t_state |= TS_FLUSH | TS_BUSY;
		dmf->dmfa_lctmr |= DMFA_FLS;
	}
	else {
		if (tp->t_state&TS_BUSY) {
			sc->sc_state |= ST_INBUSY;
			tp->t_state &= ~TS_BUSY;
		}
	}
	splx(s);
}

/*
 * Routine to handle a DMF32 transmitter interrupt.
 * Restart the idle line(s).  Clear the TS_BUSY from tp->t_state.
 * Test for DMA error and print error message.
 * SHOULD RESTART OR SOMETHING...
 * Test (& clear) DMA flush bit.  Reenable the transmitter.
 */
dmfxint(ctlr)
	int ctlr;
{
	register struct tty *tp;
	register struct dmfdevice *dmf;
	register int unit, t;
	register struct dmfa_softc *sc;
	int s;

	dmf = (struct dmfdevice *)dmfinfo[ctlr]->ui_addr;
	s = spl5();
	while ((t = dmf->dmfa_csr) & DMFA_TRDY) {
		unit = (t >> 8) & 7;
		unit |= ctlr << 3;
		tp = &dmf_tty[unit];
		sc = &dmfa_softc[unit];
		unit &= 7;
		tp->t_state &= ~TS_BUSY;
		if (t & DMFA_NXM)
			printf("dmf%d: NXM line %d\n", ctlr, unit);
		if (tp->t_state & TS_FLUSH) {
			tp->t_state &= ~TS_FLUSH;
			dmf->dmfa_csr = DMFA_IE | IR_LCTMR | unit;
			dmf->dmfa_lctmr |= DMFA_TE;
		}
		else if (sc->sc_state & ST_DMA)
			ndflush(&tp->t_outq, sc->sc_count);
		sc->sc_state = 0;

		/*
		 * Start (restart) transmission.
		 */
		if (tp->t_line)
			(*linesw[tp->t_line].l_start)(tp);
		else
			dmfstart(tp);
	}
	splx(s);
}

/*
 * Routine for ioctl for DMF32.
 * Perform line specific ioctl for Berkeley line discipline,
 * and tty ioctl then process any errors.
 */
/* ARGSUSED */
dmfioctl(dev, cmd, data, flag)
	dev_t dev;
	caddr_t data;
{
	register struct tty *tp;
	register int unit = minor(dev);
	int error;

#ifndef EMULEX
	if (unit & 0200)
		return (ENOTTY);
#endif
	tp = &dmf_tty[unit];
	error = ttioctl(tp, cmd, data, flag);
	if (error >= 0) {
		if (cmd == TIOCSETP || cmd == TIOCSETN || cmd == TIOCLSET ||
		    cmd == TIOCLBIS || cmd == TIOCLBIC)
			dmfparam(unit);
		return (error);
	}
	/*
	 * Process line-specific ioctl command.
	 */
	switch (cmd) {

	case TIOCSBRK: 
		(void) dmfmctl(dev, DMFA_BRK, DMBIS);
		break;

	case TIOCCBRK: 
		(void) dmfmctl(dev, DMFA_BRK, DMBIC);
		break;

	case TIOCSDTR: 
		(void) dmfmctl(dev, DMFA_DTR | DMFA_RTS, DMBIS);
		break;

	case TIOCCDTR: 
		(void) dmfmctl(dev, DMFA_DTR | DMFA_RTS, DMBIC);
		break;

	case TIOCMSET: 
		(void) dmfmctl(dev, dmtodmf(*(int *)data), DMSET);
		break;

	case TIOCMBIS:
		(void) dmfmctl(dev, dmtodmf(*(int *)data), DMBIS);
		break;

	case TIOCMBIC: 
		(void) dmfmctl(dev, dmtodmf(*(int *)data), DMBIC);
		break;

	case TIOCMGET: 
		*(int *)data = dmftodm(dmfmctl(dev, 0, DMGET));
		break;

	default: 
		return (ENOTTY);
	}
	return (0);
}

/*
 * Routine for DMF32 modem control
 */
dmfmctl(dev, bits, how)
	dev_t dev;
	int bits, how;
{
	register struct dmfdevice *dmf;
	register int unit, modem_status, line_control;
	register int temp;
	int s;

	unit = minor(dev);
	dmf = (struct dmfdevice *)dmf_tty[unit].t_addr;

	/*
	 * Block I/O interrupts.
	 * Get receive modem status, transmit modem status & line control info.
	 */
	s = spl5();
	dmf->dmfa_csr = DMFA_IE | IR_RMS | (unit & 7);
	modem_status = dmf->dmfa_rms << 8;
    
	dmf->dmfa_csr = DMFA_IE | IR_LCTMR | (unit & 7);
	temp = dmf->dmfa_lctmr;
	modem_status |= ((temp >> 8) & (0x1f));
	/*
	 * Fake up a carrier if software carrier is set.
	 */
	if (dmfsoftCAR[unit >> 3] & (1 << (unit & 7)))
		modem_status |= DMFA_CAR << 8;
	line_control = temp & 0x3f;
	if (line_control & DMFA_RBK)
		modem_status |= DMFA_BRK;

	/*
	 * Process modem control commands.
	 */
	switch (how) {

	case DMSET: 
		modem_status = (modem_status & 0xff00) | bits;
		break;

	case DMBIS: 
		modem_status |= bits;
		break;

	case DMBIC: 
		modem_status &= ~bits;
		break;

	case DMGET: 
		(void) splx(s);
		return (modem_status);
	}

	/*
	 * Process the break bit and set up the modem status and line control
	 * information to be replaced in the transmit modem register for the
	 * selected line.
	 */
	if (modem_status & DMFA_BRK)
		line_control |= DMFA_RBK;
	else
		line_control &= ~DMFA_RBK;
	dmf->dmfa_csr = DMFA_IE | IR_LCTMR | (unit & 7);
	dmf->dmfa_lctmr = ((modem_status&0x1f) << 8) | (line_control&0x3f);

	(void) splx(s);
	return (modem_status);
}

/*
 * Routine to convert modem status from dm to dmf format.
 * Pull bits 1 & 3 through unchanged. If dm secondary transmit bit is set,
 * and/or dm request to send bit is set, and/or dm user modem signal bit
 * is set, set the corresponding dmf bits.
 */
dmtodmf(bits)
	register int bits;
{
	register int b;

	b = bits & 012;
	if (bits & DM_ST)
		b |= DMFA_RAT;
	if (bits & DM_RTS)
		b |= DMFA_RTS;
	if (bits & DM_USR)
		b |= DMFA_USW;
	return (b);
}

/*
 * Routine to convert modem status from dmf to dm format.
 * Pull bits 1 & 3 through unchanged. Pull bits 11 - 15 through as bits
 * 4 - 8 and set bit 0 to dm line enable.  If dmf user modem signal bit
 * set, and/or dmf request to send bit set, then set the corresponding
 * dm bit also.
 */
dmftodm(bits)
	register int bits;
{
	register int b;

	b = (bits & 012) | ((bits >> 7) & 0760) | DM_LE;
	if (bits & DMFA_USR)
		b |= DM_USR;
	if (bits & DMFA_RTS)
		b |= DM_RTS;
	return (b);
}

#ifndef EMULEX
/*
 * Routine to open the line printer port on a dmf32
 */
dmflopen(dev, flag)
	dev_t dev;
	int flag;
{
	register int ctlr = minor(dev) & 7;
	register struct dmfl_softc *sc;
	register struct uba_device *ui;
	register struct dmfdevice *dmf;

	if (ctlr >= NDMF || (ui = dmfinfo[ctlr]) == 0 || ui->ui_alive == 0)
		return (ENXIO);
	sc = &dmfl_softc[ctlr];
	if (sc->sc_state & OPEN)
		return (EBUSY);	/* none of this ENXIO trash, please! */
	dmf = (struct dmfdevice *)ui->ui_addr;
	if (dmf->dmfl_ctrl & DMFL_OFL) {
		/* printf("dmf: line printer offline/jammed\n"); */
		return (EIO);
	}
	if (dmf->dmfl_ctrl & DMFL_CNV) {
		printf("dmf: line printer disconnected\n");
		return (EIO);
	}
	dmf->dmfl_ctrl = 0;
	sc->sc_state |= OPEN;
	return (0);
}

/*
 * Routine to close the line printer part of a dmf32.
 */
dmflclose(dev, flag)
	dev_t dev;
	int flag;
{
	register int ctlr = minor(dev) & 7;
	register struct dmfl_softc *sc;
	register struct uba_device *ui;

	dmflout(dev, "\f", 1);
	ui = dmfinfo[ctlr];
	sc = &dmfl_softc[ctlr];
	sc->sc_state = 0;
	if (sc->sc_info)
		ubarelse(ui->ui_ubanum, &sc->sc_info);
	((struct dmfdevice *)ui->ui_addr)->dmfl_ctrl = 0;
	return (0);
}

/*
 * Routine to write to the line printer part of a dmf32.
 */
dmflwrite(dev, uio)
	dev_t dev;
	register struct uio *uio;
{
	register int n, error = 0;
	register struct dmfl_softc *sc = &dmfl_softc[minor(dev) & 7];

	if (sc->sc_state & ERROR)
		return (EIO);
	while ((n = uio->uio_resid) > 0) {
		if (n > DMFL_SIZ) {
			n = DMFL_SIZ;
			sc->sc_state |= MORETODO;
		}
		else
			sc->sc_state &= ~MORETODO;
		error = uiomove(sc->sc_buf, n, UIO_WRITE, uio);
		if (error)
			break;
		error = dmflout(dev, sc->sc_buf, n);
		if (error)
			break;
	}
	return (error);
}

/*
 * Routine to start I/O operation to DMF line printer.
 * ``cp'' is ptr to ``n'' chars (in kernel space) to be sent.
 * DMF will be put in formatted output mode; this can be made
 * selectable from an ioctl if the need ever arises.
 */
#define DMFL_OPTIONS ((1 << 8) | (1 << 9) | (1 << 15))
		/* auto cr, use real ff, no ul conversion */

dmflout(dev, cp, n)
	dev_t dev;
	char *cp;
	int n;
{
	register struct dmfl_softc *sc;
	register int ctlr = minor(dev) & 7;
	register struct uba_device *ui;
	register struct dmfdevice *d;
	register unsigned info;
	int s;

	sc = &dmfl_softc[ctlr];
	if (sc->sc_state & ERROR)
		return (EIO);
	ui = dmfinfo[ctlr];

	/*
	 * Allocate UNIBUS resources, if needed
	 */
	if (sc->sc_info == 0)
		sc->sc_info = uballoc(ui->ui_ubanum, cp, n, 0);
	info = sc->sc_info;
	d = (struct dmfdevice *)ui->ui_addr;
	d->dmfl_ctrl = (2 << 8) | DMFL_FMT;	/* indir reg 2 */

	/*
	 * indir reg auto increments on r/w . . .
	 * SO DON'T CHANGE THE ORDER OF THIS CODE!
	 */
	d->dmfl_indrct = 0;		/* prefix chars & num */
	d->dmfl_indrct = 0;		/* suffix chars & num */
	d->dmfl_indrct = info;		/* dma lo 16 bits address */
	d->dmfl_indrct = -n;		/* number of chars, 2's complement */
	d->dmfl_indrct = ((info>>16) & 3) | DMFL_OPTIONS;/* dma hi, opts */
	d->dmfl_indrct = sc->sc_lines | (sc->sc_cols << 8);
					/* page length, width */
	sc->sc_state |= ASLP;
	s = spl5();
	d->dmfl_ctrl |= DMFL_PEN | DMFL_IE;
	while (sc->sc_state & ASLP) {
		sleep((caddr_t)sc->sc_buf, PZERO + 8);
		while (sc->sc_state & ERROR) {
			timeout(dmflint, ctlr, 10 * hz);
			sleep((caddr_t)&sc->sc_state, PZERO + 8);
		}
		/* if (sc->sc_state & ERROR) return (EIO); */
	}
	splx(s);
	return (0);
}

/*
 * Routine to handle an interrupt from the line printer part of the dmf32
 */
dmflint(ctlr)
	int ctlr;
{
	register struct uba_device *ui = dmfinfo[ctlr];
	register struct dmfl_softc *sc = &dmfl_softc[ctlr];
	register struct dmfdevice *d;

	d = (struct dmfdevice *)ui->ui_addr;
	d->dmfl_ctrl &= ~DMFL_IE;
	if (sc->sc_state & ERROR) {
		printf("dmfl: intr while in error state\n");
		if ((d->dmfl_ctrl & DMFL_OFL) == 0)
			sc->sc_state &= ~ERROR;
		wakeup((caddr_t)&sc->sc_state);
		return;
	}
	if (d->dmfl_ctrl & DMFL_DER)
		printf("dmf: NXM\n");
	if (d->dmfl_ctrl & DMFL_OFL) {
		printf("dmf: printer error\n");
		sc->sc_state |= ERROR;
	}
#ifdef notdef
	if (d->dmfl_ctrl & DMFL_PDN) {
		printf("bytes= %d\n", d->dmfl_indrct);
		printf("lines= %d\n", d->dmfl_indrct);
	}
#endif
	sc->sc_state &= ~ASLP;
	wakeup((caddr_t)sc->sc_buf);
	if (sc->sc_info && (sc->sc_state & MORETODO) == 0)
		ubarelse(ui->ui_ubanum, &sc->sc_info);
}

/*
 * NON-SUPPORTED INTERFACES...Parallel and synchronous
 */

/*
 * Interrupt routine for parallel interface, not yet supported
 */
dmfdaint()
{

	printf("dmfdaint\n"); 
}

/*
 * Interrupt routine for parallel interface, not yet supported
 */
dmfdbint()
{

	printf("dmfdbint\n"); 
}

/*
 * Interrupt routine for synchronous interface, not yet supported
 */
dmfsrint()
{

	printf("dmfsrint\n"); 
}

/*
 * interrupt routine for synchronous interface, not yet supported
 */
dmfsxint()
{

	printf("dmfsxint\n"); 
}
#endif not EMULEX
#endif NDMF > 0
