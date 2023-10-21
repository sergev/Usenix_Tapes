#

/*
 *	DJ-11 driver
 *
 *	This driver is from the University of New South Wales
 *		modified by Stephen J. Mahler    4/76
 *
 *	Modified by Bill Shannon - CWRU   03/05/79
 *		timeouts added.
 *		raw I/O added.
 *		major cleanup of code.
 *
 *	At this time (3/28/79) there is a bug in the DJ such that
 *	the transmitter vectors to the receiver interrupt address.
 *	The code at the beginning of djrint kludges around this.
 */


#include	"../param.h"
#include	"../conf.h"
#include	"../user.h"
#include	"../tty.h"
#include	"../proc.h"

#define	NDJ11	16		/* only 1 dj all lines */


#define	DJADDR	0160010		/* 1st dj address */
#define	DJTIE	0040000		/* xmit interupt enable */
#define	DJTSE	0000400		/* xmit scan enable */
#define	DJTR	0100000		/* xmitter ready */
#define	DVALID	0100000		/* data valid */
#define	DJRIE	0000100		/* rcvr interupt enable */
#define	DJRE	0000001		/* rcvr enable */
#define FRERROR	0020000		/* framing error */

#define	SSPEED	015		/* standard speed - 9600 baud */

#define	scan_on		DJADDR->djtcr =| 1<<dj_line
#define	scan_off	DJADDR->djtcr =& ~(1<<dj_line)

struct	tty	dj11[NDJ11];

struct	djregs	{
	int	djcsr;
	int	djrbuf;
	int	djtcr;
	int	djtbuf;
}

djopen(dev,flag)
{
	register char *addr;
	register struct tty *tp;
	extern	 djstart();

	if(dev.d_minor >= NDJ11) {
		u.u_error = ENXIO;
		return;
	}

	tp = &dj11[dev.d_minor];
	if(u.u_procp->p_ttyp == 0)
		u.u_procp->p_ttyp = tp;
	tp->t_addr = djstart;         /* a special start routine */
	tp->t_dev  = dev;

	if((tp->t_state&ISOPEN) == 0) {
		tp->t_flags = XTABS | ECHO | CRMOD;
		tp->t_erase = CERASE;
		tp->t_kill  = CKILL;
		tp->t_speeds = (SSPEED | (SSPEED << 8));
	}

	DJADDR->djcsr = DJTIE | DJTSE | DJRE | DJRIE;
	tp->t_state =| SSTART | ISOPEN | CARR_ON;
}

djclose(dev)
{
	register struct tty *tp;

	tp = &dj11[dev.d_minor];
	tp->t_state =& (SSTART | CARR_ON);
	wflushtty(tp);
	tp->t_state = 0;
}


djread(dev)
{
	ttread(&dj11[dev.d_minor]);
}


djwrite(dev)
{
	ttwrite(&dj11[dev.d_minor]);
}


djstart(atp)
struct tty *atp;
{
	register struct tty *tp;
	register int dj_line, sps;

	sps = PS->integ;
	spl5();
	tp = atp;
	if ((tp->t_state & (TIMEOUT | FREEZE)) == 0) {
		dj_line = tp->t_dev.d_minor & 017;
		scan_on;
	}
	PS->integ = sps;
}


djxint(dev)
{
	register int c, dj_line;
	register struct tty *tp;
	extern ttrstrt();

	printf("djxint\n"); /* debug */
	while (DJADDR->djcsr&DJTR) {	/* loop while xmitter happy */
		dj_line = (DJADDR->djtbuf.hibyte) & 017;
		tp = &dj11[dj_line];
		if (tp->t_state & (TIMEOUT | FREEZE)) {
			scan_off;
			return;
		}
		if((c = getc(&tp->t_outq)) >= 0) {
			if (c <= 0177 || (tp->t_flags&RAW))
				DJADDR->djtbuf = c;
			else {
				tp->t_state =| TIMEOUT;
				scan_off;
				timeout(ttrstrt, tp, c&0177);
			}
		} else
			scan_off;
		if(tp->t_outq.c_cc == 0 || tp->t_outq.c_cc == TTLOWAT)
			wakeup(&tp->t_outq);
	}
}


djrint(dev)
{
	register int c, dj_line;
	register struct tty *tp;

	printf("djrint\n"); /* debug */
	if ((DJADDR->djcsr & DONE) == 0) {	/* was it really a receiver intr ? */
		djxint(dev);	/* nope, simulate transmitter intr */
		return;
	}
	while ((c = DJADDR->djrbuf) & DVALID) {
		dj_line = (c >> 8) & 017;
		tp = &dj11[dj_line];
		if (tp >= &dj11[NDJ11])
			continue;
		if (tp->t_state & ISOPEN) {
			if (c & FRERROR) {	/* break key */
				if(tp->t_flags&RAW)
					c = 0;
				else
					c = CINTR;
			}
			ttyinput(c&0377, tp);
		}
	}
}


djsgtty(dev,v)
	int *v;
{
	register struct tty *tp;

	tp = &dj11[dev.d_minor];
	ttystty(tp,v);
}

