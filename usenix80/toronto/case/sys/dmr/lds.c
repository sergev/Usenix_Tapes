#

/*
 *	LDS-2 device driver
 *
 *	first attempt at a driver to control the graphics pipeline.
 *
 *	Bill Shannon	CWRU	7 Feb 79
 */

#include "../param.h"
#include "../user.h"
#include "../proc.h"
#include "../conf.h"
#include "../seg.h"

#define	LDSADDR	0167770

#define	OPEN	01			/* open state */

#define	LDS_INTR	0		/* LDS-2 interrupted PDP-11 */
#define	INTR_LDS	0100		/* interrupt LDS-2 */
#define	IENABLE		040		/* interrupt enable */

struct {
	int	state;		/* state of interface */
	struct proc *procp;	/* pointer to current user */
} lds;


struct {
	int	lds_csr;	/* interrupt control register */
	int	lds_base;	/* base address register */
	int	lds_reset;	/* master clear LDS-2 */
};



ldsopen(dev, mode)
{
	if (lds.state & OPEN) {
		u.u_error = ENXIO;
		return;
	}
	lds.state |= OPEN;		/* set it open */
	lds.procp = u.u_procp;		/* and assign it to this proc */
	if (LDSADDR->lds_reset);	/* master clear the LDS-2 */
	LDSADDR->lds_csr = 0;		/* set registers to safe value */
	LDSADDR->lds_base = 0177777;
	u.u_procp->p_flag =| SLOCK;	/* lock process in core */
}


ldsclose(dev)
{
	lds.state = 0;
	lds.procp = 0;
	if (LDSADDR->lds_reset);	/* master clear it */
	LDSADDR->lds_csr = 0;		/* turn it off */
	u.u_procp->p_flag =& ~SLOCK;	/* unlock process */
}


ldssgtty(dev, v)
register int *v;
{
	register int nb;
	register char *base;
	char *addr;

	if (v) {	/* gtty */
		*v++ = LDSADDR->lds_csr;
		*v++ = LDSADDR->lds_base;
		*v++ = lds.state;
		return;
	}
	v = u.u_arg;	/* stty */
	base = v[1] & ~1;	/* make it even */
	nb = (base>>6) & 01777;
	addr = (base&077) >> 1;
	base = (u.u_sep ? UDSA : UISA)->r[nb>>7] + (nb&0177);
	addr += base<<5;
	LDSADDR->lds_base = addr;
	LDSADDR->lds_csr = *v;
}


ldsintr()
{
	if ( /* (LDSADDR->lds_csr & LDS_INTR) && */ (lds.state & OPEN))
		psignal(lds.procp, SIGLDS);
	else
		printf("Spurious LDS-2 interrupt\n");
}
