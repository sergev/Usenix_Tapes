#
/*
 */

/*
 *	DM-BB driver
 */
#include "../param.h"
#include "../tty.h"
#include "../conf.h"

#define	DMADDR	0170500

struct	tty dh11[];
int	ndh11;		/* Set by dh.c to number of lines */
int t_dmaddr[] { DMADDR, DMADDR + 010, };

/*
 * t_dmaddr contains the UNIBUS address of the DM-BB's which
 * start at 0170500 and go by 010 to 0170670
 * if you have a mixture of dh's with and without dhdm's
 * you are going to have to modify the code further
 */

#define	DONE	0200
#define	SCENABL	040
#define	CLSCAN	01000
#define	TURNON	07	/* RQ send, CD lead, line enable */
#define	TURNOFF	1	/* line enable only */
#define	CARRIER	0100

struct dmregs {
	int	dmcsr;
	int	dmlstat;
};

/*
 * Turn on the line associated with the (DH) device dev.
 */
dmopen(dev)
{
	register struct tty *tp;
	register dmaddr;

	dmaddr = t_dmaddr[(dev&255) >> 4];
	tp = &dh11[dev.d_minor];
	dmaddr->dmcsr = (dev.d_minor & 017);
	dmaddr->dmlstat = TURNON;
	if (dmaddr->dmlstat&CARRIER)
		tp->t_state =| CARR_ON;
	dmaddr->dmcsr = IENABLE|SCENABL;
	spl5();
	while ((tp->t_state&CARR_ON)==0)
		sleep(&tp->t_rawq, TTIPRI);
	spl0();
}

/*
 * If a DH line has the HUPCL mode,
 * turn off carrier when it is closed.
 */
dmclose(dev)
{
	register struct tty *tp;
	register dmaddr;

	dmaddr = t_dmaddr[(dev&255) >>4];
	tp = &dh11[dev.d_minor];
	if (tp->t_flags&HUPCL) {
		dmaddr->dmcsr = (dev.d_minor &017);
		dmaddr->dmlstat = TURNOFF;
		dmaddr->dmcsr = IENABLE|SCENABL;
	}
}

/*
 * DM11 interrupt.
 * Mainly, deal with carrier transitions.
 */
dmint(dev)
{
	register struct tty *tp;
	register dmaddr;

	dmaddr = t_dmaddr[dev.d_minor];
	if (dmaddr->dmcsr&DONE) {
		tp = &dh11[(dmaddr->dmcsr&017) | (dev.d_minor << 4) ];
		if (tp < &dh11[ndh11]) {
			wakeup(tp);
			if ((dmaddr->dmlstat&CARRIER)==0) {
				if ((tp->t_state&WOPEN)==0) {
					signal(tp, SIGHUP);
					dmaddr->dmlstat = 0;
					flushtty(tp);
				}
				tp->t_state =& ~CARR_ON;
			} else
				tp->t_state =| CARR_ON;
		}
		dmaddr->dmcsr = IENABLE|SCENABL;
	}
}
