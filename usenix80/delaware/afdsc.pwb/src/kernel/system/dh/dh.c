#
/*
 */

/*
 *	DH-11 driver
 *	This driver calls on the DHDM driver.
 *	If the DH has no DM11-BB, then the latter will
 *	be fake. To insure loading of the correct DM code,
 *	lib2 should have dhdm.o, dh.o and dhfdm.o in that order.
 */

#include "../param.h"
#include "../conf.h"
#include "../user.h"
#include "../tty.h"
#include "../proc.h"

#define	DHADDR	0160020
#define	NDH11	32	/* number of lines */
#define	DHNCH	8	/* max number of DMA chars */
#define NDHL	2	/* number of dh's */

struct	tty dh11[NDH11];
/*
 * Place from which to do DMA on output
 */
char	dh_clist[NDH11][DHNCH];

/*
 * Used to communicate the number of lines to the DM
 */
int	ndh11	NDH11;

/*
 *	table of dh addresses which start at DHADDR
 *	and occupy 020(8) for each dh
 */
int 	t_dhaddr[] { DHADDR, DHADDR + 020, };

/*
 * Hardware control bits
 */
#define	BITS6	01
#define	BITS7	02
#define	BITS8	03
#define	TWOSB	04
#define	PENABLE	020
/* DEC manuals incorrectly say this bit causes generation of even parity. */
#define	OPAR	040
#define	HDUPLX	040000

#define	IENABLE	030100
#define	PERROR	010000
#define	FRERROR	020000
#define	XINT	0100000
#define	SSPEED	7	/* standard speed: 300 baud */

/*
 * Software copy of last dhbar
 *one for each dh device
 */
int	dhsar[NDHL];

struct dhregs {
	int dhcsr;
	int dhnxch;
	int dhlpr;
	int dhcar;
	int dhbcr;
	int dhbar;
	int dhbreak;
	int dhsilo;
};

/*
 * Open a DH11 line.
 */
dhopen(dev, flag)
{
	register struct tty *tp;
	register dhaddr;
	extern dhstart();


	if (dev.d_minor >= NDH11) {
		u.u_error = ENXIO;
		return;
	}
	tp = &dh11[dev.d_minor];
	dhaddr = t_dhaddr[(dev&255)>>4];/* index into t_dhaddr for the proper dh address*/
	tp->t_addr = dhstart;
	tp->t_dev = dev;
	dhaddr->dhcsr =| IENABLE;
	tp->t_state =| WOPEN|SSTART;
	if ((tp->t_state&ISOPEN) == 0) {
		tp->t_erase = CERASE;
		tp->t_kill = CKILL;
		tp->t_speeds = SSPEED | (SSPEED<<8);
		tp->t_flags = ODDP|EVENP|ECHO;
		dhparam(tp);
	}
	dmopen(dev);
	tp->t_state =& ~WOPEN;
	tp->t_state =| ISOPEN;
	if (u.u_procp->p_ttyp == 0)
		u.u_procp->p_ttyp = tp;
}

/*
 * Close a DH11 line.
 */
dhclose(dev)
{
	register struct tty *tp;

	tp = &dh11[dev.d_minor];
	dmclose(dev);
	tp->t_state =& (CARR_ON|SSTART);
	wflushtty(tp);
}

/*
 * Read from a DH11 line.
 */
dhread(dev)
{
	ttread(&dh11[dev.d_minor]);
}

/*
 * write on a DH11 line
 */
dhwrite(dev)
{
	ttwrite(&dh11[dev.d_minor]);
}

/*
 * DH11 receiver interrupt.
 */
dhrint(dev)
	/* dev is the dh device	*/
{
	register struct tty *tp;
	register int c;
	register dhaddr;

	dhaddr = t_dhaddr[dev&017]; /* index into table for the address of the device */
	while ((c = dhaddr->dhnxch) < 0) {	/* char. present */
		tp = &dh11[((c>>8)&017)|(dev.d_minor << 4)];
		if (tp >= &dh11[NDH11])
			continue;
		if((tp->t_state&ISOPEN)==0 || (c&PERROR)) {
			wakeup(tp);
			continue;
		}
		if (c&FRERROR)		/* break */
			if (tp->t_flags&RAW)
				c = 0;		/* null (for getty) */
			else
				c = 0177;	/* DEL (intr) */
		ttyinput(c, tp);
	}
}

/*
 * stty/gtty for DH11
 */
dhsgtty(dev, av)
int *av;
{
	register struct tty *tp;
	register r;

	tp = &dh11[dev.d_minor];
	if (ttystty(tp, av))
		return;
	dhparam(tp);
}

/*
 * Set parameters from open or stty into the DH hardware
 * registers.
 */
dhparam(atp)
struct tty *atp;
{
	register struct tty *tp;
	register int lpr;
	register dhaddr;

	tp = atp;
	spl5();
	dhaddr = t_dhaddr[(tp->t_dev.d_minor)>> 4];
	dhaddr->dhcsr.lobyte = (tp->t_dev.d_minor & 017) | IENABLE;
	/*
	 * Hang up line?
	 */
	if (tp->t_speeds.lobyte==0) {
		tp->t_flags =| HUPCL;
		dmclose(tp->t_dev);
		return;
	}
	lpr = (tp->t_speeds.hibyte<<10) | (tp->t_speeds.lobyte<<6);
	if (tp->t_speeds.lobyte == 4)		/* 134.5 baud */
		lpr =| BITS6|PENABLE|HDUPLX; else
		if (tp->t_flags&EVENP)
			if (tp->t_flags&ODDP)
				lpr =| BITS8; else
				lpr =| BITS7|PENABLE; else
			lpr =| BITS7|OPAR|PENABLE;
	if (tp->t_speeds.lobyte == 3)	/* 110 baud */
		lpr =| TWOSB;
	dhaddr->dhlpr = lpr;
	spl0();
}

/*
 * DH11 transmitter interrupt.
 * Restart each line which used to be active but has
 * terminated transmission since the last interrupt.
 */
dhxint(dev)
	/* dev is actually the dh unit */
{
	register struct tty *tp;
	register ttybit, bar;
	int dhaddr;

	dhaddr = t_dhaddr[dev&017];
	bar = dhsar[dev&017] & ~dhaddr->dhbar;
	dhaddr->dhcsr =& ~XINT;
	ttybit = 1;
	for (tp = &dh11[(dev.d_minor) << 4]; bar; tp++) {
	/* on a transmitter interrupt the sixteen lines
	 * associated with each  dh unit are restarted
	 * if it is necessary
	 */
		if(bar&ttybit) {
			dhsar[dev&017] =& ~ttybit;
			bar =& ~ttybit;
			tp->t_state =& ~BUSY;
			dhstart(tp);
		}
		ttybit =<< 1;
	}
}

/*
 * Start (restart) transmission on the given DH11 line.
 */
dhstart(atp)
struct tty *atp;
{
	extern ttrstrt();
	register c, nch;
	register struct tty *tp;
	int dhaddr;
	int sps;
	char *cp;

	sps = PS->integ;
	spl5();
	tp = atp;
	dhaddr = t_dhaddr[(tp->t_dev.d_minor) >> 4];
	/*
	 * If it's currently active, or delaying,
	 * no need to do anything.
	 */
	if (tp->t_state&(TIMEOUT|BUSY))
		goto out;
	/*
	 * t_char is a delay indicator which may have been
	 * left over from the last start.
	 * Arrange for the delay.
	 */
	if (c = tp->t_char) {
		tp->t_char = 0;
		timeout(ttrstrt, tp, (c&0177)+6);
		tp->t_state =| TIMEOUT;
		goto out;
	}
	cp = dh_clist[tp->t_dev.d_minor];
	nch = 0;
	/*
	 * Copy DHNCH characters, or up to a delay indicator,
	 * to the DMA area.
	 */
	while (nch > -DHNCH && (c = getc(&tp->t_outq))>=0) {
		if (c >= 0200) {
			tp->t_char = c;
			break;
		}
		*cp++ = c;
		nch--;
	}
	/*
	 * If the writer was sleeping on output overflow,
	 * wake him when low tide is reached.
	 */
	if (tp->t_outq.c_cc<=TTLOWAT && tp->t_state&ASLEEP) {
		tp->t_state =& ~ASLEEP;
		wakeup(&tp->t_outq);
	}
	/*
	 * If any characters were set up, start transmission;
	 * otherwise, check for possible delay.
	 */
	if (nch) {
		dhaddr->dhcsr.lobyte = (tp->t_dev.d_minor & 017) | IENABLE;
		dhaddr->dhcar = cp+nch;
		dhaddr->dhbcr = nch;
		c = 1<<(tp->t_dev.d_minor &017);
		dhaddr->dhbar =| c;
		dhsar[(tp->t_dev.d_minor)>> 4] =| c;
		tp->t_state =| BUSY;
	} else if (c = tp->t_char) {
		tp->t_char = 0;
		timeout(ttrstrt, tp, (c&0177)+6);
		tp->t_state =| TIMEOUT;
	}
    out:
	PS->integ = sps;
}
