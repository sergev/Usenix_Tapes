#
/*
 * errorlog - to handle internal printf's for non critical areas
 *
 * -ghg 07/04/77
 */

#define MAXSTA	1200	/* maximum numb of chars to put on clist */
#define MAXERL	1200	/* maximum numb of chars to put on clist */
#define ERLPRI	2	/* priority for errlog sleep */

int	errlg;		/* if set nz, errlog will be used instead */
			/* if internal (without interrupts) print */
struct errlog {
	int	cc;	/* clist pointers for errlog */
	int	cf;
	int	cl;
	int	EOpen;	/* flag indicating that errlog dev is active */
} errlog;

struct errlog statlog;	/* same as errlog, but another device and clist */
