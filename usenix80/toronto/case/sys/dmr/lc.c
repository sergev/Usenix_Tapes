#
/*
 */

/*
 * TALLY printer driver. This driver is set up to drive a TALLY printer
 * interfaced to the computer through a dc11 communications interface.
 * This driver was adapted from the Unix lp11 driver.
 */

#include "../param.h"
#include "../conf.h"
#include "../user.h"

#define	LCADDR	0175710

/*
 * control bits in dc11 transmitter status register (dctcsr)
 */

#define SPEED   000
#define STOP    04000
#define RQSEND  01
#define CLSEND  02
#define	IENABLE	0100
#define	LCREADY	0200

/*
 * Driver control flags
 */

#define	LCPRI	10
#define	LCLWAT	50
#define	LCHWAT	100
#define	EJLINE	66
#define	MAXCOL	132
#define CLIP    1000			/* Close in progress */

struct {			/* dc11 device registers */
	int dcrcsr;
	int dcrbuf;
	int dctcsr;
	int dctbuf;
};

struct  {
	int	cc;
	int	cf;
	int	cl;
	int	flag;
	int	mcc;
	int	ccc;
	int	mlc;
} lc;

#define	CAP	00		/* Set to 0 for 96-char printer, else to 01 */
#define	EJECT	02
#define	OPEN	04
#define IND	00		/* Set to 0 for no indent, else to 010 */

#define	FORM	014

lcopen(dev, flag)
{

	if(lc.flag & OPEN || LCADDR->dctcsr < 0) {
		u.u_error = EIO;
		return;
	}
	lc.flag =| (IND|EJECT|OPEN);
	lc.mlc = lc.mcc = 0;
	LCADDR->dctcsr =| IENABLE |SPEED | STOP | RQSEND;
	lcoutput(015);
	lcoutput(FORM);
}

lcclose(dev, flag)
{
	lcoutput(015);
	lcoutput(FORM);
	if(lc.cc > 0)
	{
		lc.flag =|CLIP;
		sleep(&lc.flag,LCPRI);
	}
	LCADDR->dctcsr =& ~IENABLE & ~RQSEND;
	lc.flag = 0;
}

lcwrite()
{
	register int c;

	while ((c=cpass())>=0)
		lccanon(c);
}

lccanon(c)
{
	register c1, c2;

	c1 = c;
	if(lc.flag&CAP) {
		if(c1>='a' && c1<='z')
			c1 =+ 'A'-'a'; else
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
			lccanon(c2);
			lc.ccc--;
			c1 = '-';
		}
	}

	switch(c1) {

	case '\t':
		lc.ccc = (lc.ccc+8) & ~7;
		return;

	case FORM:
	case '\n':
		if((lc.flag&EJECT) == 0 ||
		   lc.mcc!=0 || lc.mlc!=0) {
			lc.mcc = 0;
			lc.mlc++;
			if(lc.mlc >= EJLINE && lc.flag&EJECT)
			{
				c1 = FORM;
				lcoutput(c1);
				lc.mlc = 0;
			}
			else
				{
				c1 =015;
				lcoutput(c1);
				c1 = 012;
				lcoutput(c1);
			}
		}

	case '\r':
		lc.ccc = 0;
		if(lc.flag&IND)
			lc.ccc = 8;
		return;

	case 010:
		if(lc.ccc > 0)
			lc.ccc--;
		return;

	case ' ':
		lc.ccc++;
		return;

	default:
		if(lc.ccc < lc.mcc) {
			lcoutput('\r');
			lc.mcc = 0;
		}
		if(lc.ccc < MAXCOL) {
			while(lc.ccc > lc.mcc) {
				lcoutput(' ');
				lc.mcc++;
			}
			lcoutput(c1);
			lc.mcc++;
		}
		lc.ccc++;
	}
}

lcstart()
{
	register int c;

	while(LCADDR->dctcsr & LCREADY &&
	      LCADDR->dctcsr & CLSEND &&
	      ( c = getc(&lc)) >=0)
		LCADDR->dctbuf = c;
	if(lc.flag & CLIP && lc.cc == 0)
		wakeup(&lc.flag);
}

lcint()
{
	register int c;

	lcstart();
	if (lc.cc == LCLWAT || lc.cc == 0)
		wakeup(&lc);
}

lcoutput(c)
{
	if (lc.cc >= LCHWAT)
		sleep(&lc, LCPRI);
	putc(c, &lc);
	spl4();
	lcstart();
	spl0();
}
