#
/*
 */

/*
 * TALLY printer driver. This driver is set up to drive a TALLY printer
 * interfaced to the computer through a dl11 communications interface.
 * This driver was adapted from the Unix lp11 driver.
 *
 * This driver will handle a printer connected to a printer scanner
 * box, which works as follows:
 *	When you want to use the printer, set request-to-send.
 *	When you can use it, you'll get clear-to-send back.
 *	When you're done using it, drop request-to-send.
 *
 * You may also notice the use of maintenance mode.  This is done
 * to defeat the double buffering done by the dl.  Otherwise,
 * when clear-to-send drops, there may already be a character in
 * the buffer that will go out anyway.  This causes the Tally
 * much pain.
 *
 *
 * Conversion from lp11 to dl11 by W. McLemore
 * Adapted for Tally by W. Shannon
 * with help from R. Shectman
 *
 */

#include "../param.h"
#include "../conf.h"
#include "../user.h"

#define	LLADDR	0175630

/*
 * control bits in dl11 status registers
 */

#define RQSEND	 04
#define CLSEND	 020000
#define IENABLE  0100
#define DIENABLE 040
#define MAINT   04

/*
 * Driver control flags
 */

#define	LLPRI	10
#define	LLLWAT	50
#define	LLHWAT	100
#define	EJLINE	66
#define	MAXCOL	132

struct {			/* dl11 device registers */
	int dlrcsr;
	int dlrbuf;
	int dltcsr;
	int dltbuf;
};

struct	{
	int	cc;
	int	cf;
	int	cl;
	int	flag;
	int	mcc;
	int	ccc;
	int	mlc;
} ll;

#define	CAP	00		/* Set to 0 for 96-char printer, else to 01 */
#define	EJECT	02
#define	OPEN	04
#define IND	00		/* Set to 0 for no indent, else to 010 */
#define CLIP	020		/* close in progress */
#define NEEDC	040		/* low level routine needs a char to output */

#define	FORM	014

llopen(dev, flag)
{

	if(ll.flag & OPEN || LLADDR->dltcsr < 0) {
		u.u_error = EIO;
		return;
	}
	ll.flag =| (IND|EJECT|OPEN|NEEDC);
	ll.mlc = ll.mcc = 0;
	LLADDR->dlrcsr =| IENABLE | RQSEND;
	LLADDR->dltcsr =| MAINT;
}

llclose(dev, flag)
{
	lloutput(015);
	lloutput(FORM);
	if(ll.cc > 0)
	{
		ll.flag =| CLIP;	/* set close in progress */
		sleep(&ll.flag,LLPRI);	/* and wait for output to flush */
	}
	LLADDR->dlrcsr =& ~IENABLE & ~RQSEND;
	LLADDR->dltcsr =& ~MAINT;
	ll.flag = 0;
}

llwrite()
{
	register int c;

	while (( c=cpass())>=0) {
		llcanon(c);
	}
}

llcanon(c)
{
	register c1, c2;

	c1 = c;
	if(ll.flag&CAP) {
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
			llcanon(c2);
			ll.ccc--;
			c1 = '-';
		}
	}

	switch(c1) {

	case '\t':
		ll.ccc = (ll.ccc+8) & ~7;
		return;

	case FORM:
		lloutput(FORM);
		ll.mcc = ll.mlc = ll.mcc = 0;
		if (ll.flag&IND)
			ll.ccc = 8;
		return;
	case '\n':
		/* removed 09-26-77  /WAS
		if((ll.flag&EJECT) == 0 ||
		   ll.mcc!=0 || ll.mlc!=0) */ {
			ll.mcc = 0;
			ll.mlc++;
			if(ll.mlc >= EJLINE && ll.flag&EJECT)
			{
				lloutput(FORM);
				ll.mlc = 0;
			}
			else
				{
				lloutput(015);
				lloutput(012);
			}
		}

	case '\r':
		ll.ccc = 0;
		if(ll.flag&IND)
			ll.ccc = 8;
		return;

	case 010:
		if(ll.ccc > 0)
			ll.ccc--;
		return;

	case ' ':
		ll.ccc++;
		return;

	default:
		if(ll.ccc < ll.mcc) {
			lloutput('\r');
			ll.mcc = 0;
		}
		if(ll.ccc < MAXCOL) {
			while(ll.ccc > ll.mcc) {
				lloutput(' ');
				ll.mcc++;
			}
			lloutput(c1);
			ll.mcc++;
		}
		ll.ccc++;
	}
}

llstart()
{
	register int c;

	if ((LLADDR->dlrcsr&CLSEND) == 0) {
		LLADDR->dlrcsr =| DIENABLE;
		/* check again, in case it just showed up */
		if ((LLADDR->dlrcsr&CLSEND) == 0)
			return;
	}
	LLADDR->dlrcsr =& ~DIENABLE;
	if ((c = getc(&ll)) >= 0) {
		LLADDR->dltbuf = c;
	} else
		ll.flag =| NEEDC;	/* tell upper level we're waiting */
	if (ll.flag&CLIP && ll.cc == 0)
		wakeup(&ll.flag);
}

llint()
{
	register int k;

	k = LLADDR->dlrbuf;		/* clear RDONE */
	llstart();
	if (ll.cc == LLLWAT || ll.cc == 0)
		wakeup(&ll);
}

lloutput(c)
{
	if (ll.cc >= LLHWAT)
		sleep(&ll, LLPRI);
	putc(c, &ll);
	if (ll.flag&NEEDC) {
		/* if lower level needs a char, start it going */
		ll.flag =& ~NEEDC;
		spl4();
		llstart();
		spl0();
	}
}

