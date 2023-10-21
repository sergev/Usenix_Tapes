#
/*
 *
 *      DZ11 driver
 */

#include "../h/param.h"
#include "../h/user.h"
#include "../h/conf.h"
#include "../h/tty.h"
#include "../h/proc.h"

#define DZADDR	0160010
#define NDZ11	8

struct tty dz_tty[NDZ11];
char	dz_stat;
char	dz_speeds[] {
	0,020,021,022,023,024,0,025,
	026,027,030,032,034,036,0,0,
};

#define BITS7	020
#define BITS8	030
#define TWOSB	040
#define PENABLE	0100
#define OPAR	0200
#define RCVENA	010000
#define IE	040140
#define PERROR	010000
#define FRERROR	020000
#define SSPEED	7

struct device {
	int dzcsr, dzrbuf;
	char dztcr, dzdtr;
	char dztbuf, dzbrk;
};
#define dzlpr	dzrbuf
#define dzmsr	dzbrk

#define ON	1
#define OFF	0

dzopen(dev, flag)
{
	register struct tty *tp;
	extern dzstart();

	if (dev.d_minor >= NDZ11) {
		u.u_error = ENXIO;
		return;
	}
	tp = &dz_tty[dev.d_minor];
	tp->t_addr = dzstart;
	tp->t_dev = dev;
	tp->t_state =| WOPEN|SSTART;
	if ((tp->t_state&ISOPEN) == 0) {
		tp->t_erase = CERASE;
		tp->t_kill = CKILL;
		tp->t_speeds = SSPEED | (SSPEED<<8);
		tp->t_flags = ODDP|EVENP|ECHO;
		dzparam(dev);
	}
	dzmodem(dev, ON);
	spl6();
	while ((tp->t_state&CARR_ON) ==0) {
		tp->t_state =| WOPEN;
		sleep(&tp->t_rawq, TTIPRI);
	}
	tp->t_state =& ~WOPEN;
	tp->t_state =| ISOPEN;
	if (u.u_procp->p_ttyp == 0)
		u.u_procp->p_ttyp = tp;
}

/*
 * Close a DZ11 line.
 */
dzclose(dev)
{
	register struct tty *tp;

	tp = &dz_tty[dev.d_minor];
	wflushtty(tp);
	dzmodem(dev, OFF);
	tp->t_state =& (CARR_ON|SSTART);
}

/*
 * Read from a DZ11 line.
 */
dzread(dev)
{
	ttread(&dz_tty[dev.d_minor]);
}

/*
 * write on a DZ11 line
 */
dzwrite(dev)
{
	ttwrite(&dz_tty[dev.d_minor]);
}

/*
 * DZ11 receiver interrupt.
 */
dzrint()
{
	register struct tty *tp;
	register c;

	while ((c = DZADDR->dzrbuf) < 0) {	/* char. present */
		tp = &dz_tty[((c>>8)&07)];
		if (tp >= &dz_tty[NDZ11])
			continue;
		if((tp->t_state&ISOPEN)==0 ) {
			wakeup(&tp->t_rawq);
			continue;
		}
		if (c&FRERROR)		/* break */
			if (tp->t_flags&RAW)
				c = 0;		/* null (for getty) */
			else
				c = 0177;	/* DEL (intr) */
		if (c&PERROR)
			if ((tp->t_flags&(EVENP|ODDP))==EVENP
			   || (tp->t_flags&(EVENP|ODDP))==ODDP)
				continue;
		ttyinput(c, tp);
	}
}

/*
 * stty/gtty for DZ11
 */
dzsgtty(dev, av)
int *av;
{
	register struct tty *tp;

	tp = &dz_tty[dev.d_minor];
	if (ttystty(tp, av))
		return;
	dzparam(dev);
}

/*
 * Set parameters from open or stty into the DZ hardware
 * registers.
 */
dzparam(dev)
{
	register struct tty *tp;
	register lpr;

	tp = &dz_tty[dev.d_minor];
	spl5();
	DZADDR->dzcsr = IE;
	if (dz_stat==0) {
		dzscan();
		dz_stat++;
	}
	/*
	 * Hang up line?
	 */
	if (tp->t_speeds.lobyte==0) {
		tp->t_flags =| HUPCL;
		dzmodem(dev, OFF);
		spl0();
		return;
	}
	lpr = (dz_speeds[tp->t_speeds.lobyte]<<8)|(dev.d_minor&07);
	if (tp->t_flags&RAW)
		lpr =| BITS8;
	else
		lpr =| BITS7|PENABLE;
	if ((tp->t_flags&EVENP)==0)
		lpr =| OPAR;
	if (tp->t_speeds.lobyte == 3)
		lpr =| TWOSB;
	DZADDR->dzlpr = lpr;
	spl0();
}

/*
 * DZ11 transmitter interrupt.
 * Restart each line which used to be active but has
 * terminated transmission since the last interrupt.
 */
dzxint()
{
	register struct tty *tp;
	register unit;

	while(DZADDR->dzcsr<0)  {
		unit = (DZADDR->dzcsr >> 8) & 07;
		tp = &dz_tty[unit];
		if (tp->t_state & BUSY) {
			DZADDR->dztbuf = tp->t_char;
			tp->t_state =& ~BUSY;
			dzstart(tp);
			continue;
		}
		DZADDR->dztcr =& ~(1<<unit);
	}
}

/*
 * Start (restart) transmission on the given DZ11 line.
 */
dzstart(tp)
struct tty *tp;
{
	extern ttrstrt();
	register c, unit;
	int s;

	unit = tp->t_dev;
	unit = 1<<(unit&07);
	s = PS->integ;
	spl5();
	/*
	 * If it's currently active, or delaying,
	 * no need to do anything.
	 */
	if (tp->t_state&(TIMEOUT|BUSY)) {
		PS->integ = s;
		return;
	}
	/*
	 * t_char is a delay indicator which may have been
	 * left over from the last start.
	 * Arrange for the delay.
	 */
	if ((c =getc(&tp->t_outq)) >=0) {
                if (c>=0200 && (tp->t_flags&RAW)==0) {
			DZADDR->dztcr =& ~unit;
			tp->t_state =| TIMEOUT;
			timeout(ttrstrt, tp, (c&0177)+6);
		} else {
			tp->t_char =c;
			tp->t_state =| BUSY;
			DZADDR->dztcr =| unit;
		}

		if(tp->t_outq.c_cc<=TTLOWAT && tp->t_state&ASLEEP) {
			tp->t_state =& ~ASLEEP;
			wakeup(&tp->t_outq);
		}
	}
	PS->integ = s;
}

dzmodem(dev, flag)
{
	 register bit;

	 bit =1<<(dev&07);
	 if (flag==OFF)
		DZADDR->dzdtr =& ~bit;
	 else
		DZADDR->dzdtr =| bit;
}

dzscan()
{
	register i;
	register struct tty *tp;
	char  bit;

	for (i= 0; i<NDZ11; i++) {
		tp = &dz_tty[i];
		bit = 1<<(i&07);
		if (DZADDR->dzmsr&bit) {
			if ((tp->t_state&CARR_ON)==0) {
				  wakeup(&tp->t_rawq);
				  tp->t_state =| CARR_ON;
			}
		} else {
			if (tp->t_state&CARR_ON) {
				if (tp->t_state&ISOPEN) {
					signal(tp, SIGHUP);
					DZADDR->dzdtr =& ~bit;
					flushtty(tp);
				}
				tp->t_state =& ~CARR_ON;
			}
		}
	}
	timeout(dzscan, 0, 120);
}
