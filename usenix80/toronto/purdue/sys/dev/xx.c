#
/* xx.c - fake tty highspeed input device
 * this routine is called from ttyinput()
 * and dumps character into circular buffer
 * in user space.
 */

#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../tty.h"
#include "../proc.h"
#include "../seg.h"
#include "../conf.h"

struct	tty dh11[];

xxopen(dev)
{
	register struct tty *tp;

# ifdef POWERFAIL
	if (dev == NODEV)
		return;
# endif
	tp = &dh11[dev.d_minor];
	if(tp->t_xxpid) {
		u.u_error = ENXIO;
		return;
	}
	tp->t_xxpid = u.u_procp;
}

xxread(dev)
{
	register int nb;
	register struct tty *tp;

	tp = &dh11[dev.d_minor];
# ifdef XBUF
	*ka5 = praddr;
# endif
	u.u_procp->p_flag =| SLOCK; /* lock guy into core */
	nb = (u.u_base>>6) & 01777;
	tp->t_xxoff = u.u_base&077; /* buffer offset from PAR */
	tp->t_xxpar = (u.u_sep? UDSA: UISA)->r[nb>>7] + (nb&0177);
}

xxclose(dev)
{
	register struct tty *tp;

	tp = &dh11[dev.d_minor];
	tp->t_xxpar = 0;
	tp->t_xxoff = 0;
# ifdef XBUF
	*ka5 = praddr;
# endif
	u.u_procp->p_flag =& ~SLOCK;
	tp->t_xxpid = 0;
}

