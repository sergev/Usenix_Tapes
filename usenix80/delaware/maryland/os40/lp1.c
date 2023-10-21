#
/*
 */

/*
 * LP-11 Line printer driver
 */

#include "../h/param.h"
#include "../h/conf.h"
#include "../h/user.h"

/* #define LPADDR  0177514
 * Changed to use DC11E to drive a PRINTRONIX thru a QSI interface
 * by R.L.Kirby 17-Jan-79.
 * l.s must be changed by altering both
 * DL11E trap vectors to lpou instead of klin and klou.
 * c.c can now include an lpgstty routine.
 * Avoids opening and closing hangups when printer is disconnected.
 */
#define	LPADDR	0175610

#define	IENABLE	0100
#define	DONE	0200
/* Clear-to-send status line true if QSI buffer not 7/8ths full */
#define DSRDY	020000

#define	LPPRI	10
#define LPLWAT	10
#define	LPHWAT	100
#define	EJLINE	88
#define	MAXCOL	132

#define RAW	040		/* Raw mode as in tty.h */
/* #define CAP	01 */		/* Define if printer does not have 96-chars*/
/* #define EJECT 02 */		/* Define for end of page actions */
#define OPEN	04
/* #define IND	010 */		/* Define to indent all output */

#define FORM	014

struct {
	int lprsr;
	int lprbuf;
	int lpsr;
	int lpbuf;
};

struct  {
	int	cc;
	int	cf;
	int	cl;
	int	flag;
	int	mcc;
	int	ccc;
#ifdef EJECT
	int	mlc;
#endif EJECT
} lp11;

lpopen(dev, flag)
{

	/* Changed to test DATA SET READY in DL11E
	if(lp11.flag & OPEN || LPADDR->lpsr < 0) {
	*/
	if(lp11.flag & OPEN) {
		u.u_error = EIO;
		return;
	}
	lp11.flag =| OPEN;	/* Mark as open before sleeping */
	spl5();
	LPADDR->lpsr =| IENABLE;
	LPADDR->lprsr =| 042;	/* Enable receiver change int */
	while((LPADDR->lprsr & DSRDY) == 0 || lp11.cc != 0)
		sleep(&lp11, LPPRI);
	lp11.flag = (
#ifdef CAP
		CAP|
#endif CAP
#ifdef IND
		IND|
#endif IND
#ifdef EJECT
		EJECT|
#endif EJECT
		OPEN);
	lpcanon(FORM);
}

lpclose(dev, flag)
{
	lp11.flag =& ~OPEN;	/* Remove open flag before sleeping */
	/* Flush the buffer, just in case */
	spl5();
	while (lp11.cc != 0) {
		lpstart();
		sleep(&lp11, LPPRI);
	}
	spl0();
	lp11.flag = 0;
}

lpwrite()
{
	register int c;

	while ((c=cpass())>=0)
		lpcanon(c);
}

lpcanon(c)
{
	register c1;

	c1 = c;
	if (lp11.flag & RAW) {
		lpoutput(c1);
		return;
	}
#ifdef CAP
	if(lp11.flag&CAP) {
		if(c1>='a' && c1<='z')
			c1 =+ 'A'-'a'; else
		switch(c1) {

		case '{':
			c1 = '(';
			goto esc;

		case '}':
			c1 = ')';
			goto esc;

		case '`':
			c1 = '\'';
			goto esc;

		case '|':
			c1 = '!';
			goto esc;

		case '~':
			c1 = '^';

		esc:
			lpcanon(c1);
			lp11.ccc--;
			c1 = '_'; /* Use underline instead of dash */
		}
	}
#endif CAP

	switch(c1) {

	case '\t':
		lp11.ccc = (lp11.ccc+8) & ~7;
		return;

	case FORM:
	case '\n':
#ifdef EJECT
		if((lp11.flag&EJECT) == 0 ||
		   lp11.mcc!=0 || lp11.mlc!=0) {
#endif EJECT
			lp11.mcc = 0;
#ifdef EJECT
			lp11.mlc++;
			if(lp11.mlc >= EJLINE && lp11.flag&EJECT)
				c1 = FORM;
#endif EJECT
			lpoutput(c1);
#ifdef EJECT
			if(c1 == FORM)
				lp11.mlc = 0;
		}
#endif EJECT

	case '\r':
		lp11.ccc = 0;
#ifdef IND
		if(lp11.flag&IND)
			lp11.ccc = 8;
#endif IND
		return;

	case 010:	/* Backspace */
		if(lp11.ccc > 0)
			lp11.ccc--;
		return;

	case ' ':
		lp11.ccc++;
		return;

	case 005:	/* Plot mode */
	case 006:	/* 8 Lines per inch */
	case 016:	/* Shift out */
	case 017:	/* Shift in */
	case 0210:	/* Double-Height */
		lpoutput(c1);
		return;
	default:
		if(lp11.ccc < lp11.mcc) {
			lpoutput('\r');
			lp11.mcc = 0;
		}
		if(lp11.ccc < MAXCOL) {
			while(lp11.ccc > lp11.mcc) {
				/* No action space character */
				lpoutput(' ');
				lp11.mcc++;
			}
			lpoutput(c1);
			lp11.mcc++;
		}
		lp11.ccc++;
	}
}

lpstart()
{
	register int c;

	if (LPADDR->lprsr & DSRDY)
	while (LPADDR->lpsr&DONE && (c = getc(&lp11)) >= 0)
			LPADDR->lpbuf = c;
}

lpint()
{
	register int c;

	lpstart();
	if (lp11.cc == LPLWAT || lp11.cc == 0)
		wakeup(&lp11);
}

lpoutput(c)
{
	while(lp11.cc >= LPHWAT) sleep(&lp11, LPPRI);
	putc(c, &lp11);
	spl5();
	lpstart();
	spl0();
}
lpsgtty(dev, av)
	int *av;
{
	if(av) {
		spl5();
		while(lp11.cc != 0) sleep(&lp11, LPPRI);
		if(u.u_arg[2]&RAW) lp11.flag =| RAW;
		else lp11.flag =& ~RAW;
	}
}
