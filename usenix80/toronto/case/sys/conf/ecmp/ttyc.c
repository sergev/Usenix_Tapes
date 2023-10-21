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

#define	NKL11	2	/* total number of kl's and dl's */

struct	tty kl11[NKL11];
int	nkl11	NKL11;	/* for the kl driver */
char	*kladdr[NKL11]
	{ 0177560, 0175611 };
