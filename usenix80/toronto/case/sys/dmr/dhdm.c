#
/*
 *	DM-BB driver
 *
 *      This driver has been modified to support multiple  DM11s,
 *      in  order  to  avail  yourself  of  it  you  must  do the
 *      following.
 *
 *      In this module:
 *           Initialize the array dmaddrs to contain the  base
 *           addresses of your DM11s.
 *
 *      In l.s:
 *           When you set up the  interrupt  vectors  for  the
 *           DM11s,  you should add a DM11 index number to the
 *           interrupt PS for each vector  (0  for  the  first
 *           one,  1  for the second, etc.) This index will be
 *           passed to the interrupt routine so  that  it  can
 *           tell  which  DM11  caused  the interrupt. This is
 *           done so that one interrupt  routine  can  service
 *           the interrupts from all of the DM11s.
 *
 *      globals:
 *         dmaddrs      array of pointers to the sets of dm11 registers.
 *
 *      history:
 *
 *      Started out as a standard release 6 DM11 driver.
 *      Modified for Multi DM support by Dennis L. Mumaugh
 *              15 April 1977
 *      Thanks to Mark Kampe (UCLA) for assistance.
 */

#include "../param.h"
#include "../tty.h"
#include "../conf.h"

int dmaddrs[2] {0170500, 0170510};

struct	tty dh11[];
int	ndh11;		/* Set by dh.c to number of lines */

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
	register struct dmregs *dmaddr;

	dmaddr = dmaddrs[dev.d_minor >> 4];
	tp = &dh11[dev.d_minor];
	dmaddr->dmcsr = dev.d_minor&017;
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
	register struct dmregs *dmaddr;

	dmaddr = dmaddrs[dev.d_minor>>4];
	tp = &dh11[dev.d_minor];
	if (tp->t_flags&HUPCL) {
		dmaddr->dmcsr = dev.d_minor&017;
		dmaddr->dmlstat = TURNOFF;
		dmaddr->dmcsr = IENABLE|SCENABL;
	}
}

/*
 * DM11 interrupt.
 * Mainly, deal with carrier transitions.
 */
dmint(dev)
int dev;
{
	register struct tty *tp;
	register struct dmregs *dmaddr;

	dmaddr = dmaddrs[dev];
	if (dmaddr->dmcsr&DONE) {
		tp = &dh11[ (dev<<4) | (dmaddr->dmcsr&017) ];
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
