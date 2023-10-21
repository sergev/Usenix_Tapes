#
/*
 */

/*
 *   KL/DL-11 driver
 *
 *	Modified to allow the number of ttys and their
 *	addresses to be specified externally in
 *	../conf/<sys_name>/ttyc.c
 *	This allows the same driver to be used with
 *	many different system configurations easily.
 *
 *	Bill Shannon - CWRU  28 Dec 78
 *
 *	Modified for modem control.  Dialup lines
 *	are specified by an odd address in the
 *	kladdr array (175611 for 175610 with modem
 *	control).  Define DIALUP to include this code.
 *
 *	Bill Shannon - CWRU  19 Feb 79
 */
#include "../param.h"
#include "../conf.h"
#include "../user.h"
#include "../tty.h"
#include "../proc.h"

#define	CARRIER	010000		/* carrier detect */
#define	DSIENB	040		/* dataset interrupt enable */
#define DTR	02
#define	RDRENB	01

extern	struct	tty kl11[];
extern	char *kladdr[];
extern	int nkl11;	/* number of kl's and dl's */

struct klregs {
	int klrcsr;
	int klrbuf;
	int kltcsr;
	int kltbuf;
}

klopen(dev, flag)
{
	register char *addr;
	register struct tty *tp;

	if(dev.d_minor >= nkl11) {
		u.u_error = ENXIO;
		return;
	}
	tp = &kl11[dev.d_minor];
	/*
	 * get address from kladdr array
	 */
	addr = kladdr[dev.d_minor] & ~01;	/* make it even */
	tp->t_addr = addr;
	tp->t_state =| WOPEN;
	if ((tp->t_state&ISOPEN) == 0) {
		tp->t_flags = XTABS|ECHO|CRMOD;
		tp->t_erase = CERASE;
		tp->t_kill = CKILL;
	}
#ifdef DIALUP
	if ((kladdr[dev.d_minor] & 01) == 0 || addr->klrcsr & CARRIER)
#endif
		tp->t_state =| CARR_ON;
	spl5();
	addr->klrcsr =| IENABLE|DTR|RDRENB;
	addr->kltcsr =| IENABLE;
#ifdef DIALUP
	if (kladdr[dev.d_minor] & 01)
		addr->klrcsr =| DSIENB;
	while ((tp->t_state & CARR_ON) == 0)
		sleep(tp, TTIPRI + 1);
#endif
	spl0();
	tp->t_state =& ~WOPEN;
	tp->t_state =| ISOPEN;
	if (u.u_procp->p_ttyp == 0) {
		u.u_procp->p_ttyp = tp;
		tp->t_dev = dev;
	}
}

klclose(dev)
{
	register struct tty *tp;
	register char *addr;

	tp = &kl11[dev.d_minor];
	addr = tp->t_addr;
	tp->t_state = 0;
	if (tp->t_flags & HUPCL)
		addr->klrcsr =& ~DTR;
	wflushtty(tp);
	addr->klrcsr =& ~IENABLE;
	addr->kltcsr =& ~IENABLE;
}

klread(dev)
{
	ttread(&kl11[dev.d_minor]);
}

klwrite(dev)
{
	ttwrite(&kl11[dev.d_minor]);
}

klxint(dev)
{
	register struct tty *tp;

	tp = &kl11[dev.d_minor];
	ttstart(tp);
	if (tp->t_outq.c_cc == 0 || tp->t_outq.c_cc == TTLOWAT)
		wakeup(&tp->t_outq);
}

klrint(dev)
{
	register int c;
	register char *addr;
	register struct tty *tp;
	int csr;

	tp = &kl11[dev.d_minor];
	addr = tp->t_addr;
	c = addr->klrbuf;
#ifdef DIALUP
	if ((csr = addr->klrcsr) < 0) {	/* dataset status change */
		if ((csr & CARRIER) && tp->t_state & WOPEN) {	/* carrier came up */
			tp->t_state =| CARR_ON;		/* turn on software carrier */
			wakeup(tp);		/* wakeup anyone waiting for carrier */
		} else
		if (((csr & CARRIER) == 0) && (tp->t_state & CARR_ON)) {
			addr->klrcsr =& ~DTR;	/* carrier dropped, hang up */
			signal(tp, SIGHUP);	/* tell all the processes */
			flushtty(tp);
			tp->t_state =& ~CARR_ON;
		}
		return;
	}
#endif
	ttyinput(c, tp);
}

klsgtty(dev, v)
int *v;
{
	register struct tty *tp;

	tp = &kl11[dev.d_minor];
	ttystty(tp, v);
}
