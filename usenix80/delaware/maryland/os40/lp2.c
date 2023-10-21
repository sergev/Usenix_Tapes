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
 * Changed by R.L.Kirby 9-Apr-79 to support multiple line printers.
 */
#define	LPADDR	0175610
/* Number of line printers supported */
#define NLP	2

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

struct lpstruct {
	int	cc;
	int	cf;
	int	cl;
	int	flag;
	int	mcc;
	int	ccc;
#ifdef EJECT
	int	mlc;
#endif EJECT
} lp11[NLP];

lpopen(dev, flag)
{
	register addr;
	register struct lpstruct *lp;

	if(dev.d_minor >= NLP) {
		u.u_error = ENXIO;
		return;
	}
	addr = LPADDR + 8 * dev.d_minor;
	lp = &lp11[dev.d_minor];
	/* Changed to test DATA SET READY in DL11E
	if(lp->flag & OPEN || addr->lpsr < 0) {
	*/
	if(lp->flag & OPEN) {
		u.u_error = EIO;
		return;
	}
	lp->flag =| OPEN; /* Mark as open before sleeping */
	spl5();
	addr->lpsr =| IENABLE;
	addr->lprsr =| 042;	/* Enable receiver change int */
	while((addr->lprsr & DSRDY) == 0 || lp->cc != 0)
		sleep(lp, LPPRI);
	lp->flag = (
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
	lpcanon(dev, FORM);
}

lpclose(dev, flag)
{
	register struct lpstruct *lp;

	lp = &lp11[dev.d_minor];
	/* Remove open flag before sleeping */
	lp->flag =& ~OPEN;
	/* Flush the buffer, just in case */
	spl5();
	while (lp->cc != 0) {
		lpstart(dev);
		sleep(lp, LPPRI);
	}
	spl0();
	lp->flag = 0;
}

lpwrite(dev)
int dev;
{
	register int c;

	while ((c=cpass())>=0)
		lpcanon(dev, c);
}

lpcanon(dev, c)
{
	register c1;
	register struct lpstruct *lp;

	lp = &lp11[dev.d_minor];
	c1 = c;
	if (lp->flag & RAW) {
		lpoutput(dev, c1);
		return;
	}
#ifdef CAP
	if(lp->flag&CAP) {
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
			lpcanon(dev, c1);
			lp->ccc--;
			c1 = '_'; /* Use underline instead of dash */
		}
	}
#endif CAP

	switch(c1) {

	case '\t':
		lp->ccc = (lp->ccc+8) & ~7;
		return;

	case FORM:
	case '\n':
#ifdef EJECT
		if((lp->flag&EJECT) == 0 ||
		   lp->mcc!=0 || lp->mlc!=0) {
#endif EJECT
			lp->mcc = 0;
#ifdef EJECT
			lp->mlc++;
			if(lp->mlc >= EJLINE && lp->flag&EJECT)
				c1 = FORM;
#endif EJECT
			lpoutput(dev, c1);
#ifdef EJECT
			if(c1 == FORM)
				lp->mlc = 0;
		}
#endif EJECT

	case '\r':
		lp->ccc = 0;
#ifdef IND
		if(lp->flag&IND)
			lp->ccc = 8;
#endif IND
		return;

	case 010:	/* Backspace */
		if(lp->ccc > 0)
			lp->ccc--;
		return;

	case ' ':
		lp->ccc++;
		return;

	case 005:	/* Plot mode */
	case 006:	/* 8 Lines per inch */
	case 016:	/* Shift out */
	case 017:	/* Shift in */
	case 0210:	/* Double-Height */
		lpoutput(dev, c1);
		return;
	default:
		if(lp->ccc < lp->mcc) {
			lpoutput(dev, '\r');
			lp->mcc = 0;
		}
		if(lp->ccc < MAXCOL) {
			while(lp->ccc > lp->mcc) {
				/* No action space character */
				lpoutput(dev, ' ');
				lp->mcc++;
			}
			lpoutput(dev, c1);
			lp->mcc++;
		}
		lp->ccc++;
	}
}

lpstart(dev)
int dev;
{
	register int c;
	register addr;
	register struct lpstruct *lp;

	addr = LPADDR + 8 * dev.d_minor;
	lp = &lp11[dev.d_minor];

	if (addr->lprsr & DSRDY)
	while (addr->lpsr&DONE && (c = getc(&lp->cc)) >= 0)
			addr->lpbuf = c;
}

lpint(dev)
int dev;
{
	register int c;
	register struct lpstruct *lp;

	lp = &lp11[dev.d_minor];
	lpstart(dev);
	if (lp->cc == LPLWAT || lp->cc == 0)
		wakeup(lp);
}

lpoutput(dev, c)
{
	register struct lpstruct *lp;

	lp = &lp11[dev.d_minor];
	while(lp->cc >= LPHWAT)
		sleep(lp, LPPRI);
	putc(c, lp);
	spl5();
	lpstart(dev);
	spl0();
}

lpsgtty(dev, av)
	int *av;
{
	register struct lpstruct *lp;

	lp = &lp11[dev.d_minor];
	if(av) {
		spl5();
		while(lp->cc != 0)
			sleep(lp, LPPRI);
		if(u.u_arg[2]&RAW) lp->flag =| RAW;
		else lp->flag =& ~RAW;
	}
}
