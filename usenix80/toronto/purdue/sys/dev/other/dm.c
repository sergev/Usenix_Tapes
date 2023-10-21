#
/*
 */

/*
 *	DM-11 driver
 */

#include "../param.h"
#include "../conf.h"
#include "../user.h"
#include "../tty.h"
#include "../proc.h"

#define	DMADDR	0175000
#define	NDM11	16	/* number of lines */
#define	DMNCH	16	/* max number of DMA chars */

struct	tty dm11[NDM11];
/*
 * Place from which to do DMA on output
 */
char	dm_clist[NDM11][DMNCH];


/*
 * Hardware control bits
 */
#define	HDUPLX	02

#define	IENABLE	010101
#define	FRERROR	040000
#define	XINT	0100000
#define CRMOD	022
#define XTABS	02

/*
 * Software copy of last dmbar
 */
int	dmsar;
int	dmopnf;		/* nz if device initialized */
int	*dmttp;		/* tumble table pointer */

struct dmreg {
	int dmcsr;
	int dmbar;
	int dmbcr;
	int dmtbr;
};

/*
 * The following struct actually has its storage allocated in
 * low.s as it must reside on a 0400 word boundry.
 */

struct dmtbl {
	int	ca[16];
	int	wc[16];
	int	bat[16];
	int	unused[16];
	int	tt[64];
} dmtbl;

/*
 * Open a DM11 line.
 */
dmopn(dev, flag)
{
	register struct tty *tp;
	extern dmstart();

	if (dev.d_minor >= NDM11) {
		u.u_error = ENXIO;
		return;
	}
	if(!dmopnf) {
		for(dmttp = &dmtbl; dmttp < &dmtbl.tt[64]; dmttp++)
			*dmttp = 0;
		dmopnf++;
		dmttp = &dmtbl.tt[0];
	}
	tp = &dm11[dev.d_minor];
	tp->t_addr = dmstart;
	tp->t_dev = dev;
	DMADDR->dmtbr = &dmtbl;
	DMADDR->dmcsr =| IENABLE;
	tp->t_state =| WOPEN|SSTART;
	if ((tp->t_state&ISOPEN) == 0) {
		tp->t_erase = CERASE;
		tp->t_kill = CKILL;
		tp->t_flags = ODDP|EVENP|ECHO|CRMOD|XTABS;
	}
	tp->t_state =& ~WOPEN;
	tp->t_state =| ISOPEN;
	if (u.u_procp->p_ttyp == 0)
		u.u_procp->p_ttyp = tp;
}

/*
 * Close a DM11 line.
 */
dmclos(dev)
{
	register struct tty *tp;

	tp = &dm11[dev.d_minor];
	tp->t_state =& (CARR_ON|SSTART);
	wflushtty(tp);
}

/*
 * Read from a DM11 line.
 */
dmread(dev)
{
	ttread(&dm11[dev.d_minor]);
}

/*
 * write on a DM11 line
 */
dmwrite(dev)
{
	ttwrite(&dm11[dev.d_minor]);
}

/*
 * DM11 receiver interrupt.
 */
dmrint()
{
	register struct tty *tp;
	register int *tt;
	register int c;

	tt = dmttp;
	while ((c = *tt) < 0) {   /* char. present */
		*tt = 0;
		tp = &dm11[(c>>9)&017];
		if (tp >= &dm11[NDM11])
			continue;
		if((tp->t_state&ISOPEN)==0) {
			wakeup(tp);
			continue;
		}
		if (c&FRERROR)		/* break */
			if (tp->t_flags&RAW)
				c = 0;		/* null (for getty) */
			else
				c = 0177;	/* DEL (intr) */
		ttyinput(c, tp);
		if(++tt >= &dmtbl.tt[64])
			tt = &dmtbl.tt[0];
	}
	dmttp = tt;
	DMADDR->dmcsr =& ~DONE;
}

/*
 * stty/gtty for DM11
 */
dmsgtty(dev, av)
int *av;
{
	register struct tty *tp;
	register r;

	tp = &dm11[dev.d_minor];
	if (ttystty(tp, av))
		return;
}


/*
 * DM11 transmitter interrupt.
 * Restart each line which used to be active but has
 * terminated transmission since the last interrupt.
 */
dmxint()
{
	register struct tty *tp;
	register ttybit, bar;

	bar = dmsar & ~DMADDR->dmbar;
	DMADDR->dmcsr =& ~XINT;
	ttybit = 1;
	for (tp = dm11; bar; tp++) {
		if(bar&ttybit) {
			dmsar =& ~ttybit;
			bar =& ~ttybit;
			tp->t_state =& ~BUSY;
			dmstart(tp);
		}
		ttybit =<< 1;
	}
}

/*
 * Start (restart) transmission on the given DM11 line.
 */
dmstart(atp)
struct tty *atp;
{
	extern ttrstrt();
	register c, nch;
	register struct tty *tp;
	int sps;
	char *cp;

	sps = PS->integ;
	spl5();
	tp = atp;
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
	cp = dm_clist[tp->t_dev.d_minor];
	nch = 0;
	/*
	 * Copy DMNCH characters, or up to a delay indicator,
	 * to the DMA area.
	 */
	while (nch > -DMNCH && (c = getc(&tp->t_outq))>=0) {
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
		DMADDR->dmcsr =| IENABLE;
		dmtbl.ca[tp->t_dev.d_minor] = cp+nch;
		dmtbl.wc[tp->t_dev.d_minor] = nch;
		c = 1<<tp->t_dev.d_minor;
		DMADDR->dmbar =| c;
		dmsar =| c;
		tp->t_state =| BUSY;
	} else if (c = tp->t_char) {
		tp->t_char = 0;
		timeout(ttrstrt, tp, (c&0177)+6);
		tp->t_state =| TIMEOUT;
	}
    out:
	PS->integ = sps;
}
