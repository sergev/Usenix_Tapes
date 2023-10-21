#
/*
 */

/*
 *	tty configuration parameters
 *
 *	located here to make it easy to change tty
 *	configuration without recompiling the world.
 */

#include "../../tty.h"

#define	NKL11	4	/* total number of kl's and dl's */

struct	tty kl11[NKL11];
int	nkl11	NKL11;	/* for the kl driver */
int	*kladdr[NKL11]
	{ 0177560, 0175610, 0175620, 0175630 };
