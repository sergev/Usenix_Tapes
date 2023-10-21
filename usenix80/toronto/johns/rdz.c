#
/*
 *	modification of DH-11 Driver
 *	for the DZ-11
 *
 *	7/27/78	WCL	Heavily modified to support VOTRAX ML-1
 *
 *	2/21/79 MHG	Modified for use as driver for research dz (rdz11)
 */

#include "../hd/buf.h"
#include "../hd/param.h"
#include "../hd/proc.h"
#include "../hd/user.h"
#include "../hd/tty.h"

extern	rdz_addr[];
#define NRDZ11	 8	/* number of lines */
#define	NRDZBORD	(NRDZ11 /8)
#define NRDZMODM 2	/* number of lines with modems */
#define	MLDEV	7	/* line number for VOTRAX ML-1.  Must
			 * be >= NDZMODEM	*/

struct	tty rdz11[NRDZ11];
char	rdzrngo;
int	rdzcarr[NRDZBORD];		/* Stores old value of carrier byte.
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

/* Some control constants for the ML	*/
#define	MLNULL	026
#define	MLSYNC	027
#define	MLFILL	120
#define	MLHIWAT	(10*512)	/* buffering limit	*/
#define	MLALARM	SIGHUP	/* signal sent on FIFO empty		*/
#define	FLUSH	0200	/* bit in t_state	*/
#define	MLFIFOE	0200	/* bit in t_speeds -- FIFO Empty	*/

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

char	RSPEED[NRDZ11]	{7,7,13,13,13,13,13,13};

/* Table to map UNIX speed codes to DZ codes.  Some arbitrary things
 * have been done; 200 cannot be supported and results in a user error.
 * the comments per line:	   UNIX code	DZ code		*/

int	rsmap[16] {	07400,	/* HUP		RES.	*/
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

struct rdzwregs {
	int	rdzcsr;
	int	rdzlpr;
	char	rdztcr;
	char	rdzdtr;
	char	rdztbuf;
	char	rdzbrk;
};
struct rdzrregs {
	int	rdzcsr;
	int	rdzrbuf;
	char	rdztcr;
	char	rdzdtr;
	char	rdzring;
	char	rdzcar;
};

#define	bit	tp->t_char	/*** 1<<dev for speed ***/

/*
 * Open a DZ11 line.
 */
rdzopen(device, flag)
{
	register struct tty *tp;
	register	dev, wdz;
	extern		rdzstart();

	dev = device;
	wdz = dev / 8;
	tp = &rdz11[dev];

	if (dev >= NRDZ11) {
		u.u_error = ENXIO;
		return;
	}

	if(dev == MLDEV) {
		if((tp->t_state&(WOPEN|ISOPEN)) || flag != 2) {
			u.u_error = ENXIO;
			return;
		}
		tp->t_mcc = MLHIWAT;
	}
	tp->t_addr = &rdzstart;
	tp->t_state =| WOPEN|SSTART;

	if ((tp->t_state&ISOPEN) == 0) {
		tp->t_erase = CERASE;
		tp->t_kill = CKILL;
                tp->t_speeds = RSPEED[dev] | (RSPEED[dev]<<8);
		tp->t_flags = ODDP|EVENP|ECHO;
		bit = 1<< (dev & 7);
		rdzparam(dev);
		if(rdzrngo == 0) {	/* Scan not on */
			rdzrngo++;
			rdzring();
		}
	}
	rdz_addr[wdz]->rdzdtr =| bit;		/* Turn on DTR */
	if (dev < NRDZMODM) {
		spl5();
		if ((rdz_addr[wdz]->rdzcar & bit) == 0)
			while ((tp->t_state&CARR_ON)==0)
				sleep(&tp->t_rawq, TTIPRI);
		spl0();
	}
	ttyopen(tp);
	tp->t_state =| CARR_ON;
	if(dev == MLDEV) {
		tp->t_pgrp = proc;
		mlflush();
	}
}

/*
 * Close a DZ11 line.
 */
rdzclose(device)
{
	register dev, wdz;
	register struct tty *tp;

	spl5();
	dev = device;
	wdz = dev / 8;
	tp = &rdz11[dev];
	if (tp->t_flags&HUPCL) {
		rdz_addr[wdz]->rdzlpr = dev;	/* Turn off receiver */
		rdz_addr[wdz]->rdzdtr =& ~bit;	/* Turn off DTR	     */
	}
	if(dev == MLDEV) {
		tp->t_pgrp = proc;
		mlflush();
	}
	else
		wflushtty(tp);

	tp->t_state =& SSTART;
	spl0();
}

/*
 * Read from a DZ11 line.
 */
rdzread(device)
{

	if(device == MLDEV)
		u.u_error = ENXIO;
	else
		ttread(&rdz11[device]);
}

/*
 * write on a DZ11 line
 */
rdzwrite(device)
{
	if(device == MLDEV)
		mlwrite(device);
	else
	/* Contains a ttstart which in turn calls dzstart */
		ttwrite(&rdz11[device]);
}

/*
 * stty/gtty for DZ11
 */
rdzsgtty(dev, av)
int *av;
{
	register struct tty *tp;

	tp = &rdz11[dev];

	if(av == 0 && (u.u_arg[0]&017) == 6) {	/*200 baud unsupported	*/
		u.u_error = EINVAL;
		return;
	}

	spl5();
	if(dev == MLDEV) {
		if(mlsgtty(tp, av))
			return;

	} else {
		if (ttystty(tp, av))	/* stty returns 0, gtty 1 */
			return;
	}

	rdzparam(dev);
	spl0();
}

/*
 * Set parameters from open or stty into the DZ hardware
 * registers.
 */
rdzparam(dev)
{
	register struct tty *tp;
	register int lpr, wdz;
	int	spd;
	int	r;		/* return code, must save	*/

	wdz = dev / 8;
	tp = &rdz11[dev];
	spd = tp->t_speeds.lobyte&017;
	spl5();
	r = 0;
	if((rdz_addr[wdz]->rdzcsr & MSE) == 0)
		r = 1;
	rdz_addr[wdz]->rdzcsr = MSE | IENABLE;

	/*
	 * Hang up line?
	 */
	if (spd==0) {
		tp->t_flags =| HUPCL;
		rdzclose(dev);
		return;
	}
	lpr = rsmap[spd] | dev | RCVRON;
	if (spd == 4)		/* 134.5 baud */
		lpr =| BITS6|PENABLE;
	else if (tp->t_flags&EVENP)
		if (tp->t_flags&ODDP)
			lpr =| BITS8;
		else lpr =| BITS7|PENABLE;
	else lpr =| BITS7|OPAR|PENABLE;
	if (spd == 3)		/* 110 baud */
		lpr =| TWOSB;
	rdz_addr[wdz]->rdzlpr = lpr;
	spl0();
	return(r);
}
/*
 * DZ11 receiver interrupt
 * Called as each character is received.
 */
rdzrint(wdz)
{
	register struct tty *tp;
	register int c;
	register dev;

	while ((c = rdz_addr[wdz]->rdzrbuf) < 0) {	/* char. present */
		dev = ((c>>8) & 7) + (wdz << 3);
		if(dev == MLDEV)
			continue;
		tp = &rdz11[dev];
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
rdzxint(wdz)
{
	register struct tty *tp;
	register dev, c;
	extern ttrstrt();

	while ((dev = rdz_addr[wdz]->rdzcsr) <  0) {	/* XINT bit test */
		dev = ((dev>>8) & 7) + (wdz << 3);
		if(dev == MLDEV) {
			mlxint();
			continue;
		}
		tp = &rdz11[dev];
		if ((c = getc(&tp->t_outq)) < 0) {	/* Queue is empty */
			rdz_addr[wdz]->rdztcr =& ~bit;
		}
		else {
			if(tp->t_flags&RAW)
				rdz_addr[wdz]->rdztbuf = c;
			else if(c > 0177) {
				tp->t_state =| TIMEOUT;
				timeout(ttrstrt, tp, (c&0177)+6);
				rdz_addr[wdz]->rdztcr =& ~bit;
			}
			else
				rdz_addr[wdz]->rdztbuf = (c&0177);
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
rdzstart(atp)
struct tty *atp;
{
	register struct tty *tp;
	register sps, dev;

	sps = PS->integ;
	spl5();
	tp = atp;
	dev = (tp - &rdz11[0]) >> 3;
	/*
	 * If it's currently delaying, return.
	 * Otherwise just be sure scan is on.
	 */
	if ((tp->t_state&TIMEOUT) == 0) {
		rdz_addr[dev]->rdztcr =| bit;
	}
	PS->integ = sps;
}

/* Periodically check the carrier status of modems to find if
 * anyone's called up lately.
 */
rdzring()
{

	register	dev;
	register	c;
	register struct tty	*tp;
	int		unit;
	int		t;

	/* First, guarantee scan will continue	*/
	timeout(rdzring, 0, DZ_SCAN);

	/* Check for ML empty			*/
	tp = &rdz11[MLDEV];
	if((rdz_addr[MLDEV/8]->rdzring&bit) == 0) {
		if(tp->t_speeds&MLFIFOE) {
			psignal(tp->t_pgrp, MLALARM);
			tp->t_speeds =& ~MLFIFOE;
		}
	}
	else
		tp->t_speeds =| MLFIFOE;

	/* Test if any lines have changed status */
	unit = -1;
	for (dev = 0; dev < NRDZMODM; dev++) {
		if((dev>>3) != unit) {
			unit = dev >> 3;
			t = c = rdz_addr[unit]->rdzcar;
			c =^ rdzcarr[unit];
			rdzcarr[unit] = t;
		}
		if(c == 0)
			continue;
		if (c & 1) {	/* Found a change */
			tp = &rdz11[dev];
			if ((rdzcarr[unit] & bit) == 0) { /* Line Dropped */
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

/* Special driver for the VOTRAX ML-1 multi-lingual voice system.
 *
 * This driver is intimately linked with the DZ driver, and will need
 * to be modified if used with any other serial device.
 * It differs from a normal character device in that system buffers
 * are grabbed for the data storage, up to a limit set by MLHIWAT.
 * Further, one bit (MLFIFOE) of the speed word in the stty/gtty
 * call has been allocated, and indicates the status of the FIFO
 * Empty line on the VOTRAX.  This line will usually be high if the
 * VOTRAX is currently speaking.  This is not true when a group
 * of phonemes follows the last null command, as these will then
 * be in the buffer, but will not be spoken.
 * This bit (MLFIFOE) also causes a signal (MLALARM) to be sent
 * to the process on the leading edge of FIFO Empty.
 *
 * Only one process may have the ML open at a time.
 * It is intended that the driver should be able to buffer large
 * amounts of data so that the translation program can calculate
 * while the votrax is speaking.
 *
 * Special routines for implementing monstrous character buffers are
 * included.  They are lputc(), lgetc(), getc(). lmt().  They are general-
 * purpose routines which use tty structures as headers for the queue.
 *
 * Additional Hardware note:  The bits seen in dzring & dzcar for
 * the FIFO control are negations of the actual signals.
 *
 * Remember that this is not a general Votrax driver, but a special
 * modification to the DZ driver.
 */

mlwrite(dev)
{

	register int	c;
	register struct tty	*tp;

	tp = &rdz11[dev];
	while((c = cpass()) >= 0) {
		spl5();
		while(lputc(tp, c) < 0) {
			rdzstart(tp);
			sleep(&tp->t_cc, TTOPRI);
		}
		spl0();
	}

	tp->t_speeds =| MLFIFOE;
	rdzstart(tp);
}

mlxint()
{

	register struct tty	*tp;
	register char		c;

	tp = &rdz11[MLDEV];

	/* If FIFO half full, give up for a while	*/
	if( (rdz_addr[0]->rdzcar&bit) == 0 && (rdz_addr[0]->rdzring&bit) != 0 ) {
		rdz_addr[0]->rdztcr =& ~bit;
		if((tp->t_state&ASLEEP) == 0) {
			tp->t_state =| ASLEEP;
			timeout(rdzstart, tp, MLFILL);
		}
		return;
	}
	tp->t_state =& ~ASLEEP;

	/* Flush the ML: check the FIFO empty lead, and output
	 * NULLs until the FIFO is empty.
	 */
	if(tp->t_state&FLUSH) {
		if((rdz_addr[0]->rdzring&bit) == 0) {
			tp->t_state =& ~FLUSH;
			wakeup(&tp->t_state);
			return;
		}
		else
			c = MLNULL;
	} else {
		if((c = lgetc(tp)) < 0) {
			rdz_addr[0]->rdztcr =& ~bit;
			wakeup(&tp->t_actf);
			return;
		}
	}

	rdz_addr[0]->rdztbuf = c;

}

mlsgtty(atp, av)
int	*av;
{

	register struct tty	*tp;
	register		*v;

	tp = atp;
	tp->t_pgrp = u.u_procp;
	if(v = av) {
		*v++ = tp->t_speeds;
		*v++ = 0;
		*v++ = 0;
		return(1);
	}
	mlflush();
	tp->t_speeds = u.u_arg[0]& ~MLFIFOE;
	return(0);
}

lputc(atp,c)
struct tty *atp;
char c;
{
	register struct tty *tp;
	register	sps;

	sps = PS->integ;
	spl7();

	tp = atp;
	/* if queue empty, initialize */
	if (tp->t_actf == 0) {
		tp->t_actl = tp->t_actf = getblk(NODEV, 0);
		tp->t_head = tp->t_tail = tp->t_cc = tp->t_actl->av_forw = 0;
	}
	/* if we are at the end of current buffer, allocate another */
	if(tp->t_tail == 512) {
		/* may we allocate? */
		while(tp->t_cc >= tp->t_mcc)
			return(-1);
		tp->t_actl = tp->t_actl->av_forw = getblk(NODEV, 0);
		tp->t_actl->av_forw = 0;
		tp->t_tail = 0;
	}
	tp->t_actl->b_addr[tp->t_tail++] = c;
	tp->t_cc++;
	PS->integ = sps;
	return(0);
}

lgetc(atp)
struct tty *atp;
{

	register int	c, bp;
	register struct tty *tp;
	int	sps;

	tp = atp;
	sps = PS->integ;
	spl7();

	if (tp->t_actf == 0) {
		c = -1;
		goto out;
	}

	if(tp->t_cc == 0) {
		printf("MISCOUNT: %o %o %d %d\n",tp->t_actf,tp->t_actl,tp->t_head,tp->t_tail);
		lmt(tp);
		c = -1;
		goto out;
	}
	c = ( tp->t_actf->b_addr[tp->t_head++] )&0377;
	tp->t_cc--;

	/* done with this buffer? */
	if(tp->t_head == 512 || tp->t_cc == 0) {
		bp = tp->t_actf;
		tp->t_actf = tp->t_actf->av_forw;
		if(tp->t_actf == 0 && tp->t_cc != 0)
			printf("ML SYNC\n");
		tp->t_head = 0;
		brelse(bp);
		wakeup(&tp->t_cc);
	}

out:
	PS->integ = sps;
	return(c);
}

lmt(atp)
{

	register struct tty	*tp;
	register struct buf	*bp, *nbp;
	int			sps;

	tp = atp;

	sps = PS->integ;
	spl7();

	bp = tp->t_actf;
	tp->t_actf = 0;
	tp->t_cc = 0;


	while(bp) {
		nbp = bp->av_forw;
		brelse(bp);
		bp = nbp;
	}
	PS->integ = sps;
}

/* Code to guarantee VOTRAX is in a predictable state.  When done,
 * votrax should be:
 *	silent
 *	FIFO empty
 *	in votrax mode
 */
mlflush()
{
	register struct tty *tp;

	tp = &rdz11[MLDEV];

	spl5();
	lmt(tp);

	/* this is necessary in case votrax is somehow in terminal
	 * mode; otherwise it might refuse to enter votrax mode.
	 */
	lputc(tp, '\n');
	lputc(tp, MLNULL);
	lputc(tp, MLNULL);
	rdzstart(tp);
	while(tp->t_actf)
		sleep(&tp->t_actf, TTOPRI);

	tp->t_state =| FLUSH;
	while(tp->t_state&FLUSH) {
		rdzstart(tp);
		sleep(&tp->t_state, TTOPRI);
	}

	spl0();
}

fdopen()
{

	register struct tty	*tp;

	tp = &rdz11[MLDEV];
	tp->t_state =| ISOPEN|SSTART;
	bit = 1<<MLDEV;
	tp->t_mcc = MLHIWAT;
}

fdclose()
{

	register struct tty	*tp;

	tp = &rdz11[MLDEV];
	tp->t_state =& SSTART;
}

fdwrite()
{

	register	c;
	register struct tty	*tp;

	tp = &rdz11[MLDEV];
	while((c = cpass()) >= 0)
		while(lputc(tp, c) < 0)
			sleep(&tp->t_cc, TTOPRI);
}

fdread()
{

	register int	c;
	register struct tty	*tp;

	tp = &rdz11[MLDEV];
	while( ((c = lgetc(tp)) >= 0) && (passc(c) >= 0) )
		;
}

fdintr(){}
