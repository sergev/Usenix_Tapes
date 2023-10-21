#
/*
 * dz.c - DZ11 asynchronous line multiplexer driver
 *
 *	last edit:  04-Jun-1980  D A Gwyn
 */

#include "/usr/sys/conf.h"
#include "/usr/sys/param.h"
#include "/usr/sys/proc.h"
#include "/usr/sys/tty.h"
#include "/usr/sys/user.h"


/*
 * Installation-dependent parameters
 */

#define	NDZMODS	1			/* # of M7814|M7819  modules */
int	dzaddr[NDZMODS] {		/* CSR Unibus addresses */
	0160110
};
char	dzeia[NDZMODS] {		/* 0 => M7814, 1 => M7819 */
	1
};

#define SAMPLE	60			/* line poll interval (ticks) */
#define	SSPEED	14			/* standard speed (see below) */

/* Speed table for stty.c - replaces DH11 table :
   baud	 code
     50	    0
     75	    1
    110	    2
    134.5   3
    150	    4
    300	    5
    600	    6
   1200	    7
   1800	    8
   2000	    9
   2400	   10
   3600	   11
   4800	   12
   7200	   13
   9600	   14
  19200	   15	*/


/*
 * Preamble
 */

#define	SPD110	2			/* speed 110 baud */
#define	SPD134	3			/* speed 134.5 baud */
#define	NLINES	8 * NDZMODS		/* # of terminal lines */

struct	tty	dz11[NLINES];		/* defined in "tty.h" */

/* device registers: */
struct	{
	int	dzcsr;			/* Control & Status Register */
	int	dzrbuf;			/* Receiver Buffer */
	int	dztcr;			/* Transmit Control Register */
	int	dzmsr;			/* Modem Status Register */
};
#define	dzlenb	dztcr.lobyte		/* line R/W enable bits */
#define	dzdtr	dztcr.hibyte		/* DTR line bits */
#define	dzri	dzmsr.lobyte		/* RI line bits */
#define	dzco	dzmsr.hibyte		/* CO line bits */
struct	{
	int	dzcsr;
	int	dzlpr;			/* Line Parameter Register */
	int	dztcr;
	int	dztdr;			/* Transmit Data Register */
};
#define	dztbuf	dztdr.lobyte		/* Transmit Buffer */
#define	dzbrk	dztdr.hibyte		/* Break line bits */

/* dzcsr bits: */
#define	MAINT	0000010			/* Maintenance */
#define	CLR	0000020			/* Clear */
#define	MSE	0000040			/* Master Scan Enable */
#define	RIE	0000100			/* Receiver Interrupt Enable */
#define	RDONE	0000200			/* Receiver Done */
#define	TLINE	0003400			/* Transmit Line A..C */
#define	SAE	0010000			/* Silo Alarm Enable */
#define	SA	0020000			/* Silo Alarm */
#define	TIE	0040000			/* Transmitter Int. Enable */
#define	TRDY	0100000			/* Transmitter Ready */

/* dzrbuf bits: */
#define	RBUF	0000377			/* Received Character */
#define	RXLINE	0003400			/* Receive Line A..C */
#define	PARERR	0010000			/* Parity Error */
#define	FRAMERR	0020000			/* Framing Error */
#define	OVRN	0040000			/* Overrun */
#define	DATAVAL	0100000			/* Data Valid */

/* dzlpr bits: */
#define	LINE	0000007			/* Line A..C */
#define	CHRLENG	0000030			/* Character Length */
#define	STPCODE	0000040			/* Stop Code */
#define	PARENAB	0000100			/* Parity */
#define	ODDPAR	0000200			/* Odd Parity */
#define	FREQ	0007400			/* Speed Select */
#define	RXON	0010000			/* Receiver On */

#define	BITS6	0000010			/* CHRLENG 6 bits */
#define	BITS7	0000020			/* CHRLENG 7 bits */
#define	BITS8	0000030			/* CHRLENG 8 bits */

char	dzline[NLINES];			/* line-select bits */
char	dzcarr[NDZMODS];		/* carrier_status memory */
char	dzpoll	0;			/* modem_polling_active flag */


/*
 * Open a DZ11 line
 */

dzopen( dev, flag )
int	dev, flag;
{
	extern	dzstart();
	register	minor, modlin;
	register struct tty	*tp;

	minor = dev.d_minor;

	if ( minor >= NLINES  ||  minor < 0 )  {
		u.u_error = ENXIO;		/* no such device */
		return;
	}

	if ( dzpoll == 0 )  {			/* first dzopen entry */
		dzpoll++;			/* never again */
		for ( modlin = 0;  modlin < NDZMODS;  modlin++ )  {
			dzaddr[modlin]->dzcsr = TIE | RIE | MSE;

			if ( dzeia[modlin] == 0 )	/* M7814 */
				for ( tp = &dz11[modlin<<3];
				      tp < &dz11[(modlin+1)<<3];  tp++ )
					tp->t_state = CARR_ON;
					/* carrier can never drop */
		}
		for ( modlin = 0;  modlin < NLINES;  modlin++ )
			/* precalculate line-select bits */
			dzline[modlin] = 1 << (modlin & 7);
		dzcheck();			/* start modem daemon */
	}

	/* Set up tty structure block for this line */

	tp = &dz11[minor];

	if ( (tp->t_state & ISOPEN) == 0 )  {
		tp->t_flags = ODDP | EVENP | ECHO;
		tp->t_addr = &dzstart;
		tp->t_erase = CERASE;
		tp->t_kill = CKILL;
		tp->t_state =| WOPEN | SSTART;
		tp->t_speeds = SSPEED | SSPEED<<8;
		tp->t_dev = dev;
		dzparam( dev );			/* set line params */
	}

	spl6();					/* lock out daemon */
	while ( (tp->t_state & CARR_ON) == 0 )
		sleep( &tp->t_rawq, TTIPRI );	/* wait for carrier */
	spl0();

	tp->t_state =& ~WOPEN;			/* no longer waiting */
	tp->t_state =| ISOPEN;			/* officially open */

	if ( u.u_procp->p_ttyp == 0 )
		u.u_procp->p_ttyp = tp;		/* attach to process */
}


/*
 * Modem line polling daemon - compatible with 20mA interfaces also
 */

dzcheck()
{
	char	co;
	register	minor, modul;
	register struct tty	*tp;

	for ( minor = 0;  minor < NLINES;  minor++ )  {

		if ( (minor & 7) == 0 )  {	/* fresh module */
			modul = minor>>3;
			/* answer RI by raising DTR, wait for carrier */
			dzaddr[modul]->dzdtr =| dzaddr[modul]->dzri;
			co = dzaddr[modul]->dzco ^ dzcarr[modul];
			if ( co == 0 )  {	/* no differences */
				minor =+ 7;	/* skip module */
				continue;
			}
			dzcarr[modul] =^ co;	/* new state */
		}

		if ( co & 1 )  {		/* line changed state */
			tp = &dz11[minor];
			tp->t_state =^ CARR_ON;	/* toggle */
			if ( (tp->t_state & CARR_ON) == 0 )  {
				/* lost carrier, so hang up line */
				dzaddr[modul]->dzdtr =& ~dzline[minor];
				if ( (tp->t_state & WOPEN) == 0 )  {
					signal( tp, SIGHUP );
					flushtty( tp );	/* zap queues */
				}
			}
			wakeup( &tp->t_rawq );	/* notify waiters */
		}

		co =>> 1;			/* next line */
	}
	timeout( dzcheck, 0, SAMPLE );		/* re-schedule */
}


/*
 * Close a DZ11 line
 */

dzclose( dev )
int	dev;
{
	register	addr, minor;
	register struct tty	*tp;

	minor = dev.d_minor;
	tp = &dz11[minor];

	wflushtty( tp );			/* flush output queue */

	if ( tp->t_flags & HUPCL )  {		/* hang up on close */
		addr = dzaddr[minor>>3];
		addr->dzlpr = minor & 7;	/* off RXON */
		addr->dzdtr =& ~dzline[minor];	/* off DTR */
	}

	tp->t_state =& ~ISOPEN;			/* officially closed */
}


/*
 * Read from a DZ11 line
 */

dzread( dev )
int	dev;
{
	ttread( &dz11[dev.d_minor] );
}


/*
 * Write to a DZ11 line
 */

dzwrite( dev )
int	dev;
{
	ttwrite( &dz11[dev.d_minor] );		/* calls dzstart */
}


/*
 * stty/gtty entry for DZ11
 */

dzsgtty( dev, av )
int	dev, *av;
{
	if ( ttystty( &dz11[dev.d_minor], av ) == 0)
		dzparam( dev );			/* stty */
}


/*
 * Set line parameters
 */

dzparam( dev )
int	dev;
{
	register	lpr, minor;
	register struct tty	*tp;

	minor = dev.d_minor;
	tp = &dz11[minor];

	/* DZ11 has no 0 speed, so there is no "hang up on 0" code */

	lpr = RXON | tp->t_speeds & FREQ | minor & LINE;

	if ( tp->t_speeds.lobyte == SPD134 )	/* 134.5 baud */
		lpr =| BITS6 | PARENAB;
	else if ( tp->t_flags & RAW  ||
		  (tp->t_flags & (ODDP|EVENP)) == (ODDP|EVENP) )
		lpr =| BITS8;
	else if ( tp->t_flags & EVENP )
		lpr =| BITS7 | PARENAB;
	else if ( tp->t_flags & ODDP )
		lpr =| BITS7 | PARENAB | ODDPAR;
	else
		lpr =| BITS7;

	if ( tp->t_speeds.lobyte == SPD110 )	/* 110 baud */
		lpr =| STPCODE;			/* 2 stop bits */

	dzaddr[minor>>3]->dzlpr = lpr;		/* pass to hardware */
}


/*
 * DZ11 receiver interrupt
 */

dzrint( dev )
int	dev;
{
	register	c, dzp;
	register struct tty	*tp;

	dzp = dzaddr[dev];

	/* empty silo of interrupting module */

	dev =<< 3;				/* module number * 8 */
	while ( (c = dzp->dzrbuf) < 0 )  {	/* data valid */
		tp = &dz11[dev + (c >> 8 & 7)];

		if ( (tp->t_state & ISOPEN) == 0 )
			continue;		/* spurious */

		if ( c & FRAMERR )
			if ( c & 0377 )
				c = '?';	/* erroneous data */
			else			/* break */
				if ( tp->t_flags & RAW )
					c = 0;	/* for getty */
				else
					c = CINTR;

		if ( c & PARERR )
			c = '?';		/* erroneous data */

		/* data overruns are ignored (c is valid) */

		ttyinput( c & 0377, tp );	/* put in queue */
	}
}


/*
 * DZ11 transmitter interrupt
 */
dzxint( dev )
int dev;
{
	extern ttrstrt();
	register struct tty	*tp;
	register	dzp, minor;
	int	c;

	dzp = dzaddr[dev];

	/* write to all ready lines */

	dev =<< 3;				/* module number * 8 */
	while ( (c = dzp->dzcsr) < 0 )  {	/* TRDY */
		minor = dev + (c >> 8 & 7);
		tp = &dz11[minor];

		if ( tp->t_state & SUSPEND  ||
		     (c = getc( &tp->t_outq )) < 0 )
			dzp->dzlenb =& ~dzline[minor];	/* disable */
		else  {
			c =& 0377;
			if (c <= 0177)		/* normal character */
				dzp->dztbuf = c;	/* send it */
			else  {
				tp->t_state =| TIMEOUT;
				timeout( ttrstrt, tp, c & 0177 );
				dzp->dzlenb =& ~dzline[minor];
			}
			if ( tp->t_outq.c_cc <= TTLOWAT )
				wakeup( &tp->t_outq );
		}

		minor =>> 4;		/* 1.9 usec delay for scanner */
	}
}


/*
 * Start transmission on a DZ11 line
 */
dzstart( tp )
struct tty	*tp;
{
	register	minor, sps;

	sps = PS->integ;			/* save status */
	spl6();					/* lock out timeout */

	/* if not delaying, enable line */

	if ( (tp->t_state & TIMEOUT) == 0 )  {
		minor = tp - &dz11[0];
		dzaddr[minor>>3]->dzlenb =| dzline[minor];
	}
	PS->integ = sps;			/* restore status */
}
