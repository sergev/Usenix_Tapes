#
/*
 *	modification of DH-11 Driver
 *	for the DZ-11
 *
 */

#include "../hd/buf.h"
#include "../hd/param.h"
#include "../hd/proc.h"
#include "../hd/user.h"
#include "../hd/tty.h"

extern	dz_addr[];
#define	NDZ11	 24	/* number of lines */
#define	NDZBOARD	(NDZ11 /8)
#define	NDZMODEM 8	/* number of lines with modems */

struct	tty dz11[NDZ11];
char	dzringon;
int	dzcarr[NDZBOARD];		/* Stores old value of carrier byte.
			 * Used by dzring routine */

/* This structure is actually the first part of a tty structure.  It
 * gives new names to some elements as needed by lgetc/lputc buffering.
 */
struct lbufh {
	int		t_cc;	/* char count		*/
	int		t_mcc;	/* max char count	*/
	int		t_head;	/* offset: next char to be read	*/
	int		t_tail;	/* offset: next entry position	*/
	struct lbufh	*t_actf;	/* first buffer	*/
	struct lbufh	*t_actl;	/* last buffer	*/
};

/*
 * Hardware control bits
 * LPR - Line Parameter Register
 */
#define	BITS6	  010
#define	BITS7	  020
#define	BITS8	  030
#define	TWOSB	  040
#define	PENABLE	 0100
#define	OPAR	 0200
#define RCVRON 010000
#define SPEED	07400	/*	Baud	Code
				  50	  0
				  75	  1
				 110	  2
				 134.5	  3
				 150	  4
				 300	  5
				 600	  6
				1200	  7
	These codes differ	1800	  8
	from DH-11 and		2000	  9
	stty.c must be		2400	 10
	modified to report	3600	 11
	correct speeds.		4800	 12
				7200	 13
				9600	 14
			      reserved	 15
			*/
/*
 * RBUF - Receiver Buffer Register
 */
#define	RLINE	  03400
#define	RVALID	0100000
#define	RERROR	 050000
#define	PERROR	 010000
#define	FRERROR	 020000
/*
 * CSR - Control Status Register
 */
#define	MSE	    040
#define	IENABLE	 040100
#define	XINT	0100000

char	SSPEED[NDZ11]	{7,7,7,7,7,7,7,7,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13};

/* Table to map UNIX speed codes to DZ codes.  Some arbitrary things
 * have been done; 200 cannot be supported and results in a user error.
 * the comments per line:	   UNIX code	DZ code		*/

int	smap[16] {	07400,	/* HUP		RES.	*/
			 00000,	/* 50		50	*/
			 00400,	/* 75		75	*/
			 01000,	/* 110		110	*/
			 01400,	/* 134.5	134.5	*/
			 02000,	/* 150		150	*/
			 07400,	/* 200		RES.	*/
			 02400,	/* 300		300	*/
			 03000,	/* 600		600	*/
			 03400,	/* 1200		1200	*/
			 04000,	/* 1800		1800	*/
			 05000,	/* 2400		2400	*/
			 06000,	/* 4800		4800	*/
			 07000,	/* 9600		9600	*/
			 04400,	/* EXTa		2000	*/
			 05400,	/* EXTb		3600	*/
		};

struct dzwregs {
	int	dzcsr;
	int	dzlpr;
	char	dztcr;
	char	dzdtr;
	char	dztbuf;
	char	dzbrk;
};
struct dzrregs {
	int	dzcsr;
	int	dzrbuf;
	char	dztcr;
	char	dzdtr;
	char	dzring;
	char	dzcar;
};

#define	bit	tp->t_char	/*** 1<<dev for speed ***/

/*
 * Open a DZ11 line.
 */
dzopen(device, flag)
{
	register struct tty *tp;
	register	dev, wdz;
	extern		dzstart();

	dev = device;
	wdz = dev / 8;
	tp = &dz11[dev];

	if (dev >= NDZ11) {
		u.u_error = ENXIO;
		return;
	}

	tp->t_addr = &dzstart;
	tp->t_state =| WOPEN|SSTART;

	if ((tp->t_state&ISOPEN) == 0) {
		tp->t_erase = CERASE;
		tp->t_kill = CKILL;
                tp->t_speeds = SSPEED[dev] | (SSPEED[dev]<<8);
		tp->t_flags = ODDP|EVENP|ECHO;
		bit = 1<< (dev & 7);
		dzparam(dev);
		if(dzringon == 0) {	/* Scan not on */
			dzringon++;
			dzring();
		}
	}
	dz_addr[wdz]->dzdtr =| bit;		/* Turn on DTR */
	if (dev < NDZMODEM) {
		spl5();
		if ((dz_addr[wdz]->dzcar & bit) == 0)
			while ((tp->t_state&CARR_ON)==0)
				sleep(&tp->t_rawq, TTIPRI);
		spl0();
	}
	ttyopen(tp);
	tp->t_state =| CARR_ON;
}

/*
 * Close a DZ11 line.
 */
dzclose(device)
{
	register dev, wdz;
	register struct tty *tp;

	spl5();
	dev = device;
	wdz = dev / 8;
	tp = &dz11[dev];
	if (tp->t_flags&HUPCL) {
		dz_addr[wdz]->dzlpr = dev;	/* Turn off receiver */
		dz_addr[wdz]->dzdtr =& ~bit;	/* Turn off DTR	     */
	}
	wflushtty(tp);

	tp->t_state =& SSTART;
	spl0();
}

/*
 * Read from a DZ11 line.
 */
dzread(device)
{

	ttread(&dz11[device]);
}

/*
 * write on a DZ11 line
 */
dzwrite(device)
{
/* Contains a ttstart which in turn calls dzstart */
	ttwrite(&dz11[device]);
}

/*
 * stty/gtty for DZ11
 */
dzsgtty(dev, av)
int *av;
{
	register struct tty *tp;

	tp = &dz11[dev];

	if(av == 0 && (u.u_arg[0]&017) == 6) {	/*200 baud unsupported	*/
		u.u_error = EINVAL;
		return;
	}

	spl5();
	if (ttystty(tp, av))	/* stty returns 0, gtty 1 */
		return;

	dzparam(dev);
	spl0();
}

/*
 * Set parameters from open or stty into the DZ hardware
 * registers.
 */
dzparam(dev)
{
	register struct tty *tp;
	register int lpr, wdz;
	int	spd;
	int	r;		/* return code, must save	*/

	wdz = dev / 8;
	tp = &dz11[dev];
	spd = tp->t_speeds.lobyte&017;
	spl5();
	r = 0;
	if((dz_addr[wdz]->dzcsr & MSE) == 0)
		r = 1;
	dz_addr[wdz]->dzcsr = MSE | IENABLE;

	/*
	 * Hang up line?
	 */
	if (spd==0) {
		tp->t_flags =| HUPCL;
		dzclose(dev);
		return;
	}
	lpr = smap[spd] | dev | RCVRON;
	if (spd == 4)		/* 134.5 baud */
		lpr =| BITS6|PENABLE;
	else if (tp->t_flags&EVENP)
		if (tp->t_flags&ODDP)
			lpr =| BITS8;
		else lpr =| BITS7|PENABLE;
	else lpr =| BITS7|OPAR|PENABLE;
	if (spd == 3)		/* 110 baud */
		lpr =| TWOSB;
	dz_addr[wdz]->dzlpr = lpr;
	spl0();
	return(r);
}
/*
 * DZ11 receiver interrupt
 * Called as each character is received.
 */
dzrint(wdz)
{
	register struct tty *tp;
	register int c;
	register dev;

	while ((c = dz_addr[wdz]->dzrbuf) < 0) {	/* char. present */
		dev = ((c>>8) & 7) + (wdz << 3);
		tp = &dz11[dev];
		if((tp->t_state&ISOPEN)==0 || (c&RERROR)) {
			wakeup(tp);
			continue;
		}
		if (c&FRERROR)			/* break */
			if (tp->t_flags&RAW)
				c = 0;		/* null (for getty) */
			else
				c = 0177;	/* DEL (intr) */
		ttyinput(c, tp);
	}
}
/*
 * DZ11 transmitter interrupt.
 * Scanner found a line needing service.
 * Feed it the next character or, if none, turn off dztcr bit
 */
dzxint(wdz)
{
	register struct tty *tp;
	register dev, c;
	extern ttrstrt();

	while ((dev = dz_addr[wdz]->dzcsr) <  0) {	/* XINT bit test */
		dev = ((dev>>8) & 7) + (wdz << 3);
		tp = &dz11[dev];
		if ((c = getc(&tp->t_outq)) < 0) {	/* Queue is empty */
			dz_addr[wdz]->dztcr =& ~bit;
		}
		else {
			if(tp->t_flags&RAW)
				dz_addr[wdz]->dztbuf = c;
			else if(c > 0177) {
				tp->t_state =| TIMEOUT;
				timeout(ttrstrt, tp, (c&0177)+6);
				dz_addr[wdz]->dztcr =& ~bit;
			}
			else
				dz_addr[wdz]->dztbuf = (c&0177);
		}
		if(tp->t_outq.c_cc <= TTLOWAT ) {
			wakeup(&tp->t_outq);
		}
		dev =>> 4;	/* 2 microsecond delay for scanner */
	}
}

/*
 * Start (restart) transmission on the given DZ11 line.
 */
dzstart(atp)
struct tty *atp;
{
	register struct tty *tp;
	register sps, dev;

	sps = PS->integ;
	spl5();
	tp = atp;
	dev = (tp - &dz11[0]) >> 3;
	/*
	 * If it's currently delaying, return.
	 * Otherwise just be sure scan is on.
	 */
	if ((tp->t_state&TIMEOUT) == 0) {
		dz_addr[dev]->dztcr =| bit;
	}
	PS->integ = sps;
}

/* Periodically check the carrier status of modems to find if
 * anyone's called up lately.
 */
dzring()
{

	register	dev;
	register	c;
	register struct tty	*tp;
	int		unit;
	int		t;

	/* First, guarantee scan will continue	*/
	timeout(dzring, 0, DZ_SCAN);

	/* Test if any lines have changed status */
	unit = -1;
	for (dev = 0; dev < NDZMODEM; dev++) {
		if((dev>>3) != unit) {
			unit = dev >> 3;
			t = c = dz_addr[unit]->dzcar;
			c =^ dzcarr[unit];
			dzcarr[unit] = t;
		}
		if(c == 0)
			continue;
		if (c & 1) {	/* Found a change */
			tp = &dz11[dev];
			if ((dzcarr[unit] & bit) == 0) { /* Line Dropped */
				if ((tp->t_state&WOPEN)==0) {
					signal(tp->t_pgrp, SIGHUP);
					flushtty(tp);
				}
				tp->t_state =& ~CARR_ON;
			} else
				tp->t_state =| CARR_ON;
			wakeup(&tp->t_rawq);
		}
		c =>> 1;
	}
}
