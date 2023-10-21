#
/*
 */

/*
 * AR-11 device driver
 *
 * Bill Shannon   CWRU   12 Jan 79
 */

#include "../param.h"
#include "../user.h"
#include "../conf.h"


#define	ARADDR		0170400		/* ar-11 address */


/*
 * hardware bits
 */

#define	DONE	0200		/* A/D done flag */
#define	IE	0100		/* interrupt enable */
#define	COE	040		/* clock overflow interrupt enable */
#define	START	01		/* start A/D conversion */
#define	REPEAT	0400		/* repeat mode for clock */


/*
 * modes settable by user through stty
 */

#define	M_UNIP	040		/* unipolar A/D conversion */
#define	M_XY	031		/* X, Y, and intensify modes */
#define	M_CLK	02		/* enable clock */

struct {
	int	adst;		/* A/D status */
	int	adbuf;		/* A/D buffer */
	int	clkst;		/* clock status */
	int	clkbuf;		/* clock buffer/preset */
	int	dispst;		/* display status */
	int	xbuf;		/* X buffer */
	int	ybuf;		/* Y buffer */
	int	clkcnt;		/* clock counter */
};

static int openf;	/* open flag for exclusive use */

 struct {
	unsigned number;	/* sample number */
	unsigned sample[16];	/* A/D values for 16 channels */
	/* the parameter order for stty/gtty is determined by the following */
	char	exp;		/* exponent for clock frequency */
	char	mant;		/* mantissa for clock frequency */
	char	maxchan;	/* highest channel number to sample */
	char	mode;		/* sampling mode */
} ar;


aropen(dev, mode)
{
	register int i;

	if (openf) {
		u.u_error = ENXIO;
		return;
	}
	openf++;
	ar.number = 0;
	ar.exp = ar.mant = ar.maxchan = ar.mode = 0;
	for (i = 0; i < 16; i++)
		ar.sample[i] = 0;
}


arclose(dev)
{
	ARADDR->adst = 0;	/* turn everything off */
	ARADDR->clkst = 0;
	ARADDR->dispst = 0;
	openf = 0;
}


arread(dev)
{
	register int i, t, *addr;

	addr = ARADDR;		/* for speed */
	t = ar.mode & M_CLK;	/* true if in clock sampling mode */
	if (passi(ar.number)) return;	/* pass sample number back */
	for (i = 0; i <= ar.maxchan; i++) {
		if (t == 0) {
			addr->adst = (((ar.mode&M_UNIP) | i) << 8) | START;
			/* wait for conversion to finish (32 us.) */
			while ((addr->adst&DONE) == 0);
			ar.sample[i] = addr->adbuf;
		}
		if (passi(ar.sample[i])) break;
	}
}


arwrite(dev)
{
	register int *addr;

	addr = ARADDR;		/* for speed */
	while ((addr->dispst&DONE) == 0);	/* wait for it to be ready */
	addr->xbuf = ipass();		/* put out X */
	addr->ybuf = ipass();		/* put out Y */
}


arsgtty(dev, v)
register int *v;
{
	register int *p, *addr;

	addr = ARADDR;		/* for speed */
	p = &ar.exp;
	if (v) {	/* gtty */
		*v++ = *p++;
		*v++ = *p++;
		*v++ = 0;
		return;
	}
	v = u.u_arg;	/* stty */
	*p++ = *v++;
	*p++ = *v++;
	if (ar.exp < 2 || ar.exp > 4) {
		u.u_error = EINVAL;
		return;
	}
	/* set modes into hardware registers */
	addr->adst = ((ar.mode&M_UNIP)<<8) | (ar.mode&M_CLK ? COE|IE : 0);
	addr->clkbuf = (-ar.mant);
	addr->clkst = REPEAT | ((7-ar.exp)<<1) | ((ar.mode&M_CLK)>>1);
	addr->dispst = ar.mode&M_XY;
	ar.number = 0;
}


arintr(dev)
{
	register int i, *addr;

	addr = ARADDR;		/* for speed */
	if ((ar.mode&M_CLK) == 0 || dev.d_minor != 0) {
		printf("AR: unknown interrupt\n");
		return;
	}
	i = addr->adst.hibyte & 017;
	ar.sample[i] = addr->adbuf;
	if (i >= ar.maxchan) {
		ar.number++;			/* increment sample number */
		i = COE | IE;			/* restart clock sampling */
	} else
		i = ((i++)<<8) | IE | START;	/* continue manual sampling */
	addr->adst = ((ar.mode&M_UNIP) << 8) | i;
}


passi(i)
int i;
{
	return (passc(i.lobyte) || passc(i.hibyte));
}


ipass()
{
	return ((cpass() << 8) || cpass());
}
