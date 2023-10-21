#
/*
 */

/*
 * General line printer driver.
 *
 * This driver is capable of driving most printers that can be
 * interfaced to the computer through a dl11 communications interface.
 * This driver was adapted from the Unix lp11 driver
 * and has been tested with a Tally printer, Printronix printer/plotter,
 * and a HyType II printer (homemade intelligent interface).
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
 * Conversion from Tally to Printronix by W. Shannon  11/16/77
 * Rewritten and modified for multiple printers by W. Shannon  01/15/78
 * Revised for increased functionality, sgtty added - W. Shannon  01/15/79
 *
 */

#include "../param.h"
#include "../conf.h"
#include "../user.h"

#define NLX	2		/* number of printers */

/*
 * control bits in dl11 status registers
 */

#define RQSEND	 04
#define CLSEND	 020000
#define IENABLE  0100
#define DIENABLE 040
#define MAINT   04
#define	DONE	0200

/*
 * Driver control flags
 */

#define	LXPRI	10
#define	LXLWAT	50
#define	LXHWAT	100
#define	EJLINE	60
#define	LENGTH	66		/* length of paper in lines */
#define	WIDTH	132		/* width of paper in characters */
#define	INDENT	8		/* number of chars to indent, if IND set */

/*
 *	User specified flags
 */
#define CAP	01		/* printer can't handle lower case */
#define	EJECT	02		/* printer knows about form feeds */
#define IND	010		/* indent all lines 8 spaces */
#define	BACKSP	020		/* device can backspace */
#define PRX	040		/* device is a Printronix printer/plotter */

/*
 *	Internal states
 */
#define OPEN	01		/* device is open */
#define CLIP	02		/* close in progress */
#define PLOTM	04		/* plot mode flag */
#define ELONGM	010		/* elongated mode flag */
#define	LOST	020		/* don't know what line we're on */

/*
 *	Special characters
 */
#define	FORM	014
#define PLOT	05		/* enter plot mode character */
#define ELONG	07		/* print elongated characters */

struct {			/* dl11 device registers */
	int dlrcsr;
	int dlrbuf;
	int dltcsr;
	int dltbuf;
};

struct	lx {
	int	l_cc;
	int	l_cf;
	int	l_cl;
	int	*l_addr;
	char	l_flags;
	char	l_state;
	char	l_mcc;		/* max chars actually printed */
	char	l_ccc;		/* current char count, saved white space, >= mcc */
	char	l_mlc;		/* max lines actually printed */
	char	l_clc;		/* current line count, saved white space, >= mlc */
	char	l_width;
	char	l_length;
	char	l_indent;
	char	l_pad;
} lx[NLX];

int iflags[NLX] {
	PRX | EJECT,		/* Printronix */
	EJECT			/* Tally */
};

int *lxaddr[NLX] {
	0175630,		/* Printronix */
	0175620			/* Tally */
};

lxopen(dev, flag)
{
	register struct lx *rlx;
	register *addr, d;

	if ((d = dev.d_minor) >= NLX || (rlx = &lx[d])->l_state & OPEN) {
		u.u_error = EIO;
		return;
	}
	rlx->l_flags = iflags[dev.d_minor];	/* set up initial flags */
	rlx->l_mlc = rlx->l_mcc = rlx->l_clc = rlx->l_ccc = 0;
	rlx->l_length = LENGTH;
	rlx->l_width = WIDTH;
	rlx->l_indent = INDENT;
	rlx->l_state = OPEN;
	addr = rlx->l_addr = lxaddr[dev.d_minor];
	addr->dlrcsr =| IENABLE | RQSEND | DIENABLE;
	addr->dltcsr =| MAINT;
	lxoutput('\r', rlx);
}

lxclose(dev, flag)
{
	register struct lx *rlx;
	register *addr;

	rlx = &lx[dev.d_minor];
	addr = rlx->l_addr;
	lxcanon(FORM, rlx);		/* make sure we're at top of page */
	lxoutput(FORM, rlx);		/* and give us a blank page */
	if (rlx->l_cc > 0) {
		rlx->l_state =| CLIP;	/* set close in progress */
		sleep(&rlx->l_state,LXPRI);  /* and wait for output to flush */
	}
	addr->dlrcsr =& ~(IENABLE | RQSEND | DIENABLE);
	addr->dltcsr =& ~MAINT;
	rlx->l_state = 0;
}

lxwrite(dev)
{
	register int c;

	while (( c=cpass())>=0) {
		lxcanon(c, &lx[dev.d_minor]);
	}
}

lxcanon(c, alx)
struct lx *alx;
{
	register c1, c2;
	register struct lx *rlx;

	rlx = alx;
	c1 = c;
	if (rlx->l_state&PLOTM) {
		if (c1 == '\n')  
			rlx->l_state =& ~PLOTM;
		lxoutput(c1, rlx);
	} else {
		/* removed 11-16-77   /WAS
		if(rlx->l_flags&CAP) {
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
				lxcanon(c2, rlx);
				rlx->l_ccc--;
				c1 = '-';
			}
		}
		*/
	
		switch(c1) {
	
		case '\t':
			rlx->l_ccc = (rlx->l_ccc+8) & ~7;
			return;
	
		case FORM:
			rlx->l_clc = rlx->l_length - 1;
		case '\n':
			if (rlx->l_mcc != 0) {		/* at left margin ? */
				lxoutput('\r', rlx);	/* put us there */
				lxoutput('\n', rlx);	/* physically */
				rlx->l_mcc = 0;		/* and logically */
				rlx->l_mlc++;
				if (rlx->l_state & ELONGM) rlx->l_mlc++;
			}
			rlx->l_clc++;
			if (rlx->l_state&ELONGM) rlx->l_clc++;
			rlx->l_state =& ~(PLOTM | ELONGM);
			if (rlx->l_clc >= rlx->l_length) {
				/* past end of page */
				if ((rlx->l_mlc || rlx->l_state&LOST)  &&
				    /* page was not blank (or we're lost) */
				    (rlx->l_clc > rlx->l_mlc)) {
				    /* and we spaced over page boundary */
					if (rlx->l_flags & EJECT) {
						/* printer knows about FF */
						/* bring it to start of page */
						lxoutput(FORM, rlx);
					} else {
						/* must simulate FF with LF's */
						while (rlx->l_mlc++ < rlx->l_clc)
							lxoutput('\n', rlx);
					}
					rlx->l_state =& ~LOST;
				}
				rlx->l_mlc = rlx->l_clc = 0;
			}
			/* note fall through here */

		case '\r':
			rlx->l_ccc = 0;
			if(rlx->l_flags&IND)
				rlx->l_ccc = rlx->l_indent;
			return;
	
		case 010:
			if(rlx->l_ccc > 0) {
				rlx->l_ccc--;
				if ((rlx->l_flags&BACKSP) &&
				    /* and last char output was printable */
				    rlx->l_ccc == rlx->l_mcc - 1) {
					rlx->l_mcc--;
					lxoutput(010, rlx);
				}
			}
			return;
	
		case ' ':
			rlx->l_ccc++;
			return;

		case PLOT:
			if (rlx->l_flags&PRX) {
				rlx->l_state =| PLOTM | LOST;
			}
			goto print;

		case ELONG:
			if (rlx->l_flags & PRX) {
				rlx->l_state =| ELONGM;
				c1 = 010;
			}
			goto print;
	
		default:
		print:
			if(rlx->l_ccc < rlx->l_mcc) {
				lxoutput('\r', rlx);
				rlx->l_mcc = 0;
			}
			while (rlx->l_clc > rlx->l_mlc) {
				lxoutput('\n', rlx);
				rlx->l_mlc++;
			}
			if(rlx->l_ccc < (rlx->l_width & 0377)) {
				while(rlx->l_ccc > rlx->l_mcc) {
					lxoutput(' ', rlx);
					rlx->l_mcc++;
				}
				lxoutput(c1, rlx);
				if (c1 >= ' ') rlx->l_mcc++;
			}
			if (c1 >= ' ') rlx->l_ccc++;
		}
	}
}

lxstart(alx)
struct lx *alx;
{
	register int c;
	register struct lx *rlx;
	register *addr;

	rlx = alx;
	addr = rlx->l_addr;
	addr->dlrcsr =& ~(IENABLE | DIENABLE);
	if ((addr->dltcsr & DONE) &&
		(addr->dlrcsr & CLSEND) &&
		(c = getc(rlx)) >= 0)
		addr->dltbuf = c;
	if (rlx->l_state&CLIP && rlx->l_cc == 0)
		wakeup(&rlx->l_state);
	addr->dlrcsr =| IENABLE | DIENABLE;
}

lxint(dev)
{
	register int k, *addr;
	register struct lx *rlx;

	rlx = &lx[dev.d_minor];
	addr = rlx->l_addr;
	k = addr->dlrbuf;		/* clear RDONE */
	lxstart(rlx);
	if (rlx->l_cc == LXLWAT || rlx->l_cc == 0)
		wakeup(rlx);
}

lxoutput(c, alx)
struct lx *alx;
{
	register m1, m2;
	register struct lx *rlx;
	int c2;

	rlx = alx;
	if (rlx->l_cc >= LXHWAT)
		sleep(rlx, LXPRI);
	if (rlx->l_state&PLOTM) {
		m1 = 1;
		m2 = 1<<5;
		c2 = 0100;
		while (m2) {
			if (c&m1) c2 =| m2;
			m1 =<< 1;
			m2 =>> 1;
		}
		putc(c2, rlx);
	} else
		putc(c, rlx);
	spl4();
	lxstart(rlx);
	spl0();
}

lxsgtty(dev, av)
int *av;
{
	register int *v;
	register struct lx *rlx;

	rlx = &lx[dev.d_minor];
	if (v = av) {
		*v++ = rlx->l_flags;
		v->lobyte = rlx->l_length;
		v->hibyte = rlx->l_width;
		v++;
		v->lobyte = rlx->l_indent;
		v->hibyte = rlx->l_pad;
		return(1);
	}
	v = u.u_arg;
	rlx->l_flags = *v++;
	rlx->l_length = v->lobyte;
	rlx->l_width = v->hibyte;
	v++;
	rlx->l_indent = v->lobyte;
	rlx->l_pad = v->hibyte;
	return(0);
}
