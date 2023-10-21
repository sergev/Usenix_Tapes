#
/*
 */

/*
 * LQ-11 Line printer driver
 * just a second copy of lp.c for testing Potter line printer
 * at CSR = 177520, vec = 210 (in low.s)
 */

#include "../param.h"
#include "../conf.h"
#include "../user.h"

#define	LQADDR	0177520

#define	IENABLE	0100
#define	DONE	0200

#define	LQPRI	10-1
#define	LQLWAT	100
#define	LQHWAT	600
#define	EJLINE	60
#define	MAXCOL	132

int	lqhangs; /* kludge!! number of times our interface has hung */
int	lqhngs;  /* kludge - nz if watchdog active */
struct {
	int lqsr;
	int lqbuf;
};

struct  {
	int	cc;
	char	*cf;
	char	*cl;
	int	flag;
	int	mcc;
	int	ccc;
	int	mlc;
} lq11;

#define	CAP	00		/* Set to 0 for 96-char printer, else to 01 */
#define	EJECT	02
#define	OPEN	04
#define BS	010		/* ASCII backspace */
#define IND	0		/* Set to 0 for no indent, else to 010 */

#define	FORM	014

lqopen(dev, flag)
{
	extern lqck();

	if(lq11.flag & OPEN || LQADDR->lqsr < 0) {
		u.u_error = EIO;
		return;
	}

#ifndef CENTRONICS
/*  kludge code for our Centronics which looses interrupts */
	if(lqhngs == 0)
		timeout(&lqck,0,900);	/* start interrupt watchdog */
		lqhngs++;
/* .......................................................... */
#endif
	lq11.flag =| (IND|EJECT|OPEN|CAP);
	LQADDR->lqsr =| IENABLE;
	lqcanon(FORM);
}

/*  Kludge code to force lq interface to interrupt again */
lqck()
{
	if(LQADDR->lqsr == 0300) {
		LQADDR->lqsr = 0300; /* kick lq interface in side */
		lqhangs++;
	}
	timeout(&lqck,0,3600);	/* go away for a minute */
}
/* .................................................... */

lqclose(dev, flag)
{
	lqcanon(FORM);
	lqoutput(0); /* kludge for Houston printer  hdwe problem */
	lq11.flag = 0;
}

lqwrite()
{
	register int c;

	while ((c=cpass())>=0)
		lqcanon(c);
}

lqcanon(c)
{
	register c1, c2;

	c1 = c;
	if(lq11.flag&CAP) {
		if(c1>='a' && c1<='z')
			c1 =- 040 ; else
		switch(c1) {

		case '{':
			c2 = '(';
			goto esc;

		case '}':
			c2 = ')';
			goto esc;

		case '`':
			c2 = '\'';
			goto esc;

		case '|':
			c2 = '!';
			goto esc;

		case '~':
			c2 = '^';

		esc:
			lqcanon(c2);
			lq11.ccc--;
			c1 = '-';
		}
	}

	switch(c1) {

	case '\t':
		lq11.ccc = (lq11.ccc+8) & ~7;
		return;

	case FORM:
	case '\n':
		if((lq11.flag&EJECT) == 0 ||
		   lq11.mcc!=0 || lq11.mlc!=0) {
			lq11.mcc = 0;
			lq11.mlc++;
			if(lq11.mlc >= EJLINE && lq11.flag&EJECT)
				c1 = FORM;
			lqoutput(c1);
			if(c1 == FORM)
				lq11.mlc = 0;
		}

	case '\r':
		lq11.ccc = 0;
		if(lq11.flag&IND)
			lq11.ccc = 8;
		return;

	case BS:
		if(lq11.ccc > 0)
			lq11.ccc--;
		return;

	case ' ':
		lq11.ccc++;
		return;

	default:
		if(lq11.ccc < lq11.mcc) {
			lqoutput('\r');
			lq11.mcc = 0;
		}
		if(lq11.ccc < MAXCOL) {
			while(lq11.ccc > lq11.mcc) {
				lqoutput(' ');
				lq11.mcc++;
			}
			lqoutput(c1);
			lq11.mcc++;
		}
		lq11.ccc++;
	}
}

lqstart()
{
	register int c;

	while (LQADDR->lqsr&DONE && (c = getc(&lq11)) >= 0) {
		LQADDR->lqbuf = c;
		c++; /* delay so we won't inter on each char */
		c++;
		c++;
	}
}

lqint()
{
	register int c;

	lqstart();
	if (lq11.cc == LQLWAT || lq11.cc == 0)
		wakeup(&lq11);
}

lqoutput(c)
{
	if (lq11.cc >= LQHWAT)
		sleep(&lq11, LQPRI);
	putc(c, &lq11);
	spl4();
	lqstart();
	spl0();
}
