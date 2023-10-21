/*	dhu.c	1.1	02/09/85	*/

#include "dhu.h"
#if NDHU > 0
/*
 * DHU-11 driver
 */
#include "../machine/pte.h"

#include "bk.h"
#include "../h/param.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/ioctl.h"
#include "../h/tty.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/vm.h"

#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"
#include "../vaxuba/dhureg.h"

#include "../h/bk.h"
#include "../h/clist.h"
#include "../h/file.h"
#include "../h/uio.h"

/*
 * Definition of the driver for the auto-configuration program.
 */
int	dhuprobe(), dhuattach(), dhurint(), dhuxint();
struct	uba_device *dhuinfo[NDHU];
u_short	dhustd[] = { 0 };
struct	uba_driver dhudriver =
	{ dhuprobe, 0, dhuattach, 0, dhustd, "dhu", dhuinfo };

#define	ISPEED	B9600
#define	IFLAGS	(EVENP|ODDP)

/*
 * Local variables for the driver
 */

short	dhusoftCAR[NDHU];

struct	tty dhu11[NDHU*16];
int	ndhu11	= NDHU*16;
int	dhuact;				/* mask of active dhu's */
int	dhustart(), ttrstrt();

short dhu_speed[] = {
	0,	/* 0 baud */
	0,	/* 50 baud */
	0,	/* 75 baud */
	2,	/* 110 baud */
	3,	/* 134.5 baud */
	0,	/* 150 baud */
	0,	/* 200 baud */
	5,	/* 300 baud */
	6,	/* 600 baud */
	7,	/* 1200 baud */
	0,	/* 1800 baud */
	10,	/* 2400 baud */
	11,	/* 4800 baud */
	13,	/* 9600 baud */
	0,	/* 19200 baud (EXTA) */
	15,	/* 38400 baud (EXTB) */
};


/*
 * The clist space is mapped by the driver onto each UNIBUS.
 * The UBACVT macro converts a clist space address for unibus uban
 * into an i/o space address for the DMA routine.
 */
int	dhu_ubinfo[MAXNUBA];		/* info about allocated unibus map */
int	cbase[MAXNUBA];			/* base address in unibus map */
#define	UBACVT(x, uban)		(cbase[uban] + ((x)-(char *)cfree))

/*
 * Routine for configuration to force a dhu to interrupt.
 * Set to transmit at 9600 baud, and cause a transmitter interrupt.
 */
/*ARGSUSED*/
dhuprobe(reg)
	caddr_t reg;
{
	register int br, cvec;		/* these are ``value-result'' */
	register struct dhudevice *dhuaddr = (struct dhudevice *)reg;

#ifdef lint
	br = 0; cvec = br; br = cvec;
	if (ndhu11 == 0) ndhu11 = 1;
	dhurint(0); dhuxint(0);
#endif
	dhuaddr->un.dhucsr = DHU_MR|DHU_SK;
	DELAY(100);
	dhuaddr->un.dhucsr = DHU_RIE;
	DELAY(30000);
	dhuaddr->un.dhucsr = 0;
	return (sizeof (struct dhudevice));
}

/*
 * Routine called to attach a dhu.
 */
dhuattach(ui)
	struct uba_device *ui;
{

	dhusoftCAR[ui->ui_unit] = ui->ui_flags;
}

/*
 * Open a DHU11 line, mapping the clist onto the uba if this
 * is the first dhu on this uba.  Turn on this dhu if this is
 * the first use of it.  Also do a dmopen to wait for carrier.
 */
/*ARGSUSED*/
dhuopen(dev, flag)
	dev_t dev;
{
	register struct tty *tp;
	register int unit, dhu;
	register struct dhudevice *addr;
	register struct uba_device *ui;
	int s;

	unit = minor(dev);
	dhu = unit >> 4;
	if (unit >= NDHU*16 || (ui = dhuinfo[dhu])== 0 || ui->ui_alive == 0)
		return (ENXIO);
	tp = &dhu11[unit];
	if (tp->t_state&TS_XCLUDE && u.u_uid!=0)
		return (EBUSY);
	addr = (struct dhudevice *)ui->ui_addr;
	tp->t_addr = (caddr_t)addr;
	tp->t_oproc = dhustart;
	tp->t_state |= TS_WOPEN;
	/*
	 * While setting up state for this uba and this dhu,
	 * block uba resets which can clear the state.
	 */
	s = spl5();
	if (dhu_ubinfo[ui->ui_ubanum] == 0) {
		/* 512+ is a kludge to try to get around a hardware problem */
		dhu_ubinfo[ui->ui_ubanum] =
		    uballoc(ui->ui_ubanum, (caddr_t)cfree,
			512+nclist*sizeof(struct cblock), 0);
		cbase[ui->ui_ubanum] = dhu_ubinfo[ui->ui_ubanum]&0x3ffff;
	}
	if ((dhuact&(1<<dhu)) == 0) {
		addr->un.dhucsr = DHU_IE;
		dhuact |= (1<<dhu);
	}
	addr->un.dhucsrl = DHU_RIE|(unit&017);
	addr->dhulctrl |= RXENABLE;
	splx(s);
	/*
	 * If this is first open, initialze tty state to default.
	 */
	if ((tp->t_state&TS_ISOPEN) == 0) {
		ttychars(tp);
		if (tp->t_ispeed == 0) {
			tp->t_ispeed = ISPEED;
			tp->t_ospeed = ISPEED;
			tp->t_flags = IFLAGS;
		}
		dhuparam(unit);
	}
	/*
	 * fake carrier (until modem control properly in)
	 */
	tp->t_state |= TS_CARR_ON;
	return ((*linesw[tp->t_line].l_open)(dev, tp));
}

/*
 * Close a DHU11 line
 */
/*ARGSUSED*/
dhuclose(dev, flag)
	dev_t dev;
	int flag;
{
	register struct tty *tp;
	register unit;
	int s;
	register struct dhudevice *addr;

	unit = minor(dev);
	tp = &dhu11[unit];
	addr = (struct dhudevice *)tp->t_addr;
	(*linesw[tp->t_line].l_close)(tp);
	s = spl5();
	addr->un.dhucsrl = DHU_RIE|(unit&017);
	addr->dhulctrl &= ~(RXENABLE|TXBREAK);
	splx(s);
	ttyclose(tp);
}

dhuread(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp = &dhu11[minor(dev)];

	return ((*linesw[tp->t_line].l_read)(tp, uio));
}

dhuwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp = &dhu11[minor(dev)];

	return ((*linesw[tp->t_line].l_write)(tp, uio));
}

/*
 * DHU11 receiver interrupt.
 */
dhurint(dhu)
	int dhu;
{
	register struct tty *tp;
	register c;
	register struct dhudevice *addr;
	register struct tty *tp0;
	register struct uba_device *ui;
	int overrun = 0;

	ui = dhuinfo[dhu];
	if (ui == 0 || ui->ui_alive == 0)
		return;
	addr = (struct dhudevice *)ui->ui_addr;
	tp0 = &dhu11[dhu<<4];
	/*
	 * Loop fetching characters from the silo for this
	 * dhu until there are no more in the silo.
	 */
	while ((c = addr->dhurbuf) < 0) {
		if ((c&070000) == 070000)
			continue;	/* diagnostic or modem */
		tp = tp0 + ((c>>8)&0xf);
		if ((tp->t_state&TS_ISOPEN)==0) {
			wakeup((caddr_t)tp);
			continue;
		}
		if (c & DHU_PE)
			if ((tp->t_flags&(EVENP|ODDP))==EVENP
			 || (tp->t_flags&(EVENP|ODDP))==ODDP )
				continue;
		if ((c & DHU_DO) && overrun == 0) {
			printf("dhu%d: silo overflow\n", dhu);
			overrun = 1;
		}
		if (c & DHU_FE)
			/*
			 * At framing error (break) generate
			 * a null (in raw mode, for getty), or a
			 * interrupt (in cooked/cbreak mode).
			 */
			if (tp->t_flags&RAW)
				c = 0;
			else
				c = tp->t_intrc;
#if NBK > 0
		if (tp->t_line == NETLDISC) {
			c &= 0177;
			BKINPUT(c, tp);
		} else
#endif
			(*linesw[tp->t_line].l_rint)(c, tp);
	}
}

/*
 * Ioctl for DHU11.
 */
/*ARGSUSED*/
dhuioctl(dev, cmd, data, flag)
	caddr_t data;
{
	register struct tty *tp;
	register int unit = minor(dev);
	int error;
	register struct dhudevice *addr;
	int s;

	tp = &dhu11[unit];
	error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, data, flag);
	addr = (struct dhudevice *)tp->t_addr;
	if (error >= 0)
		return (error);
	error = ttioctl(tp, cmd, data, flag);
	if (error >= 0) {
		if (cmd == TIOCSETP || cmd == TIOCSETN)
			dhuparam(unit);
		return (error);
	}
	switch (cmd) {

	case TIOCSBRK:
		s = spl5();
		addr->un.dhucsrl = DHU_RIE|(unit&017);
		addr->dhulctrl |= TXBREAK;
		splx(s);
		break;

	case TIOCCBRK:
		s = spl5();
		addr->un.dhucsrl = DHU_RIE|(unit&017);
		addr->dhulctrl &= ~TXBREAK;
		splx(s);
		break;

	case TIOCSDTR:
		break;

	case TIOCCDTR:
		break;

	default:
		return (ENOTTY);
	}
	return (0);
}

/*
 * Set parameters from open or stty into the DHU hardware
 * registers.
 */
dhuparam(unit)
	register int unit;
{
	register struct tty *tp;
	register struct dhudevice *addr;
	register int lpar;
	int s;

	tp = &dhu11[unit];
	addr = (struct dhudevice *)tp->t_addr;
	/*
	 * Block interrupts so parameters will be set
	 * before the line interrupts.
	 */
	s = spl5();
	addr->un.dhucsrl = (unit&0xf) | DHU_IE;
	if ((tp->t_ispeed)==0) {
		tp->t_state |= TS_HUPCLS;
		return;
	}
	lpar = ((dhu_speed[tp->t_ospeed])<<12) | ((dhu_speed[tp->t_ispeed])<<8);
	if ((tp->t_ispeed) == B134)
		lpar |= BITS6;
	else if ((tp->t_flags & (RAW|LITOUT)) || (tp->t_flags&ANYP) == ANYP)
		lpar |= BITS8;
	else {
		lpar |= BITS7|PENABLE;
		if (tp->t_flags & EVENP)
			lpar |= EPAR;
	}
	if ((tp->t_ospeed) == B110)
		lpar |= TWOSB;
	addr->dhulpr = lpar;
	splx(s);
}

/*
 * DHU11 transmitter interrupt.
 * Restart each line which used to be active but has
 * terminated transmission since the last interrupt.
 */
dhuxint(dhu)
	int dhu;
{
	register struct tty *tp;
	register struct dhudevice *addr;
	register struct uba_device *ui;
	register int unit;
	int cntr;
	short csr;

	ui = dhuinfo[dhu];
	addr = (struct dhudevice *)ui->ui_addr;
	while ((csr = addr->un.dhucsr) < 0) {
		unit = (csr>>8)&0xf;
		addr->un.dhucsrl = DHU_RIE|unit;
		tp = &dhu11[dhu*16+unit];
		if (csr & DHU_TDE) {
			printf("dhu%d: TDE\n", dhu);
			addr->dhutbfct = 0;
		}
		tp->t_state &= ~TS_BUSY;
		if (tp->t_state & TS_FLUSH) {
			tp->t_state &= ~TS_FLUSH;
			addr->dhutbfct = 0;
		}
		else {
			cntr = ((addr->dhutbf2 & 0x3) << 16)
			       + addr->dhutbf1
			       - UBACVT(tp->t_outq.c_cf, ui->ui_ubanum);
			ndflush(&tp->t_outq, (int)cntr);
		}
		if (tp->t_line)
			(*linesw[tp->t_line].l_start)(tp);
		else
			dhustart(tp);
	}
}

/*
 * Start (restart) transmission on the given DHU11 line.
 */
dhustart(tp)
	register struct tty *tp;
{
	register struct dhudevice *addr;
	register int car, dhu, unit, nch;
	int s;

	unit = minor(tp->t_dev);
	dhu = unit >> 4;
	unit &= 0xf;
	addr = (struct dhudevice *)tp->t_addr;

	/*
	 * Must hold interrupts in following code to prevent
	 * state of the tp from changing.
	 */
	s = spl5();
	/*
	 * If it's currently active, or delaying, no need to do anything.
	 */
	if (tp->t_state&(TS_TIMEOUT|TS_BUSY|TS_TTSTOP)) {
		goto out;
	}
	/*
	 * If there are sleepers, and output has drained below low
	 * water mark, wake up the sleepers.
	 */
	if (tp->t_outq.c_cc<=TTLOWAT(tp)) {
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
	 * Now restart transmission unless the output queue is
	 * empty.
	 */
	if (tp->t_outq.c_cc == 0) {
		goto out;
	}
	addr->un.dhucsrl = DHU_RIE|unit;
	if (addr->dhulctrl & TXABORT)
		addr->dhulctrl &= ~TXABORT;
	if (tp->t_flags & (RAW|LITOUT))
		nch = ndqb(&tp->t_outq, 0);
	else {
		nch = ndqb(&tp->t_outq, 0200);
		/*
		 * If first thing on queue is a delay process it.
		 */
		if (nch == 0) {
			nch = getc(&tp->t_outq);
			timeout(ttrstrt, (caddr_t)tp, (nch&0x7f)+6);
			tp->t_state |= TS_TIMEOUT;
			goto out;
		}
	}
	/*
	 * If characters to transmit, restart transmission.
	 */
	if (nch) {
		car = UBACVT(tp->t_outq.c_cf, dhuinfo[dhu]->ui_ubanum);
		addr->un.dhucsrl = unit|DHU_RIE;
		addr->dhutbf1 = car & 0xffff;
		addr->dhutbf2 = (car>>16) & 3;
		addr->dhutbfct = nch;
		addr->dhutbf2 |= TXDMAST|TXENABLE;
		tp->t_state |= TS_BUSY;
	}
out:
	splx(s);
}

/*
 * Stop output on a line, e.g. for ^S/^Q or output flush.
 */
/*ARGSUSED*/
dhustop(tp, flag)
	register struct tty *tp;
{
	register struct dhudevice *addr;
	register int unit, s;

	addr = (struct dhudevice *)tp->t_addr;
	/*
	 * Block input/output interrupts while messing with state.
	 */
	s = spl5();
	if (tp->t_state & TS_BUSY) {
		/*
		 * Device is transmitting; stop output
		 */
		unit = minor(tp->t_dev);
		addr->un.dhucsrl = (unit&017) | DHU_RIE;
		if ((tp->t_state&TS_TTSTOP)==0)
			tp->t_state |= TS_FLUSH;
		addr->dhulctrl |= TXABORT;
	}
	splx(s);
}

/*
 * Reset state of driver if UBA reset was necessary.
 * Reset the csrl and lpr registers on open lines, and
 * restart transmitters.
 */
dhureset(uban)
	int uban;
{
	register int dhu, unit;
	register struct tty *tp;
	register struct uba_device *ui;
	int i;

	if (dhu_ubinfo[uban] == 0)
		return;
	dhu_ubinfo[uban] = uballoc(uban, (caddr_t)cfree,
	    512+nclist*sizeof (struct cblock), 0);
	cbase[uban] = dhu_ubinfo[uban]&0x3ffff;
	dhu = 0;
	for (dhu = 0; dhu < NDHU; dhu++) {
		ui = dhuinfo[dhu];
		if (ui == 0 || ui->ui_alive == 0 || ui->ui_ubanum != uban)
			continue;
		printf(" dhu%d", dhu);
		((struct dhudevice *)ui->ui_addr)->un.dhucsr = DHU_IE;
		unit = dhu * 16;
		for (i = 0; i < 16; i++) {
			tp = &dhu11[unit];
			if (tp->t_state & (TS_ISOPEN|TS_WOPEN)) {
				dhuparam(unit);
				tp->t_state &= ~TS_BUSY;
				dhustart(tp);
			}
			unit++;
		}
	}
	dhutimer();
}

/*
 * At software clock interrupt time or after a UNIBUS reset
 * empty all the dhu silos.
 */
dhutimer()
{
	register int dhu;
	register int s = spl5();

	for (dhu = 0; dhu < NDHU; dhu++)
		dhurint(dhu);
	splx(s);
}
#endif DHU

