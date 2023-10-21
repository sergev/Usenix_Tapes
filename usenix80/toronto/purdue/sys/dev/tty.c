#
/*
 */

/*
 * general TTY subroutines
 */
#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../tty.h"
#include "../proc.h"
#include "../inode.h"
#include "../file.h"
#include "../reg.h"
#include "../conf.h"
#include "../errlog.h"
# ifdef XSTTY
#include "../buf.h"
# endif

/*
 * Input mapping table-- if an entry is non-zero, when the
 * corresponding character is typed preceded by "\" the escape
 * sequence is replaced by the table value.  Mostly used for
 * upper-case only terminals.
 */
char	maptab[]
{
	000,000,000,000,004,000,000,000,
	010,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	030,000,000,000,000,000,000,000,
	000,'|',000,000,000,000,000,'`',
	'{','}',000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,'~',000,
	000,'A','B','C','D','E','F','G',
	'H','I','J','K','L','M','N','O',
	'P','Q','R','S','T','U','V','W',
	'X','Y','Z',000,000,000,000,000,
};

# ifdef APL

/*
 * APL overstrike table - if the sequence <char1><APL_BS><char2> is seen
 * on output (ttyouput), the cursor is positioned over <char1> and the
 * appropriate overstrike is output.
 * WARNING: this structure does not compile correctly with the
 * V6 C-compiler with -O option. --ghg 07/29/78.
 */

struct  apltab  {
	char    chr1;
	char    chr2;
} apltab[32]      {
	'T',')',    /* NOR */
	'T','{',    /* NAND */
	'L','=',    /* Domino */
	'O','P',    /* Log */
	'O','M',    /* reverse */
	'O','*',    /* rotate */
	'H','M',    /* grad up */
	'G','M',    /* grad dn */
	'L','K',    /* ' quad */
	'B','J',    /* execute */
	'N','J',    /* format */
	'O','?',    /* transpose */
	'C','J',    /* lamp */
	'B','N',    /* I-beam */
	'*','/',    /* reduce */
	'*','?',    /* scan */
	'*','U',    /* start */
	'*','Y',    /* stop */
	'L','\\',   /* read */
	'L','|',    /* write */
	'L','Y',    /* open */
	'L','U',    /* close */
	'L','O',    /* query */
	'G','T',    /* lock del */
	0

/*      octal 030 to 037 not assigned yet */
	};
# endif

/*
 * The actual structure of a clist block manipulated by
 * getc and putc (mch.s)
 */
struct cblock {
	struct cblock *c_next;
	char info[6];
};

/* The character lists-- space for 6*NCLIST characters */
struct cblock cfree[NCLIST];
/* List head for unused character blocks. */
struct cblock *cfreelist;

/*
 * structure of device registers for KL, DL, and DC
 * interfaces-- more particularly, those for which the
 * SSTART bit is off and can be treated by general routines
 * (that is, not DH).
 */
struct {
	int ttrcsr;
	int ttrbuf;
	int tttcsr;
	int tttbuf;
};

/*
 * The routine implementing the gtty system call.
 * Just call lower level routine and pass back values.
 */
gtty()
{
	int v[3];
	register *up, *vp;

	u.u_arg[4] = 0; /* allows pty to detect empty() call */
	vp = v;
	sgtty(vp);
	if (u.u_error)
		return;
	up = u.u_arg[0];
	suword(up, *vp++);
	suword(++up, *vp++);
	suword(++up, *vp++);
}

/*
 * The routine implementing the stty system call.
 * Read in values and call lower level.
 */
stty()
{
	register int *up;

	up = u.u_arg[0];
	u.u_arg[0] = fuword(up);
	u.u_arg[1] = fuword(++up);
	u.u_arg[2] = fuword(++up);
	sgtty(0);
}

/*
 * Stuff common to stty and gtty.
 * Check legality and switch out to individual
 * device routine.
 * v  is 0 for stty; the parameters are taken from u.u_arg[].
 * c  is non-zero for gtty and is the place in which the device
 * routines place their information.
 */
sgtty(v)
int *v;
{
	register struct file *fp;
	register struct inode *ip;

	if ((fp = getf(u.u_ar0[R0])) == NULL)
		return;
	ip = fp->f_inode;
	if ((ip->i_mode&IFMT) != IFCHR) {
		u.u_error = ENOTTY;
		return;
	}
	(*cdevsw[ip->i_addr[0].d_major].d_sgtty)(ip->i_addr[0], v);
}

/*
 * Wait for output to drain, then flush input waiting.
 */
wflushtty(atp)
struct tty *atp;
{
	register struct tty *tp;

	tp = atp;
	spl5();
nohold:
	if(tp->t_state&HOLD) {
		tp->t_state =& ~(HOLD|TIMEOUT);
		ttrstrt(tp);
	}
# ifdef XSTTY
	while(tp->t_bp && tp->t_bp->b_wcount) {
		if (tp->t_state & HOLD)
			goto nohold;
		sleep(tp->t_bp, TTOPRI);
	}
# endif
	while (tp->t_outq.c_cc) {
		if(tp->t_state & HOLD)
			goto nohold;	/* don't let suspend hang close */
		tp->t_state =| ASLEEP;
		sleep(&tp->t_outq, TTOPRI);
	}
	flushtty(tp);
	spl0();
}

/*
 * Initialize clist by freeing all character blocks, then count
 * number of character devices. (Once-only routine)
 */
cinit()
{
	register int ccp;
	register struct cblock *cp;
	register struct cdevsw *cdp;
#ifdef XBUF
	extern claddr;

	*ka5 = claddr;
#endif

	ccp = cfree;
	for (cp=(ccp+07)&~07; cp <= &cfree[NCLIST-1]; cp++) {
		cp->c_next = cfreelist;
		cfreelist = cp;
	}
	ccp = 0;
	for(cdp = cdevsw; cdp->d_open; cdp++)
		ccp++;
	nchrdev = ccp;
}

/*
 * flush all TTY queues
 */
flushtty(atp)
struct tty *atp;
{
	register struct tty *tp;
	register int sps;

	tp = atp;
	while (getc(&tp->t_canq) >= 0);
	while (getc(&tp->t_outq) >= 0);
	wakeup(&tp->t_rawq);
	wakeup(&tp->t_outq);
	sps = PS->integ;
	spl5();
	while (getc(&tp->t_rawq) >= 0);
	tp->t_delct = 0;
	tp->t_state =& ~(TIMEOUT|HOLD);
	ttstart(tp);	/* to clear HOLD */
	PS->integ = sps;
}

/*
 * transfer raw input list to canonical list,
 * doing erase-kill processing and handling escapes.
 * It waits until a full line has been typed in cooked mode,
 * or until any character has been typed in raw mode.
 */
canon(atp)
struct tty *atp;
{
	register char *bp;
	char *bp1;
	register struct tty *tp;
	register int c;

	tp = atp;
	spl5();
	while (tp->t_delct==0) {
#ifdef XSTTY
		if(tp->t_speeds&BLITZ)
			return(0);
#endif
/*		if ((tp->t_state&CARR_ON)==0)
			return(0); */
		sleep(&tp->t_rawq, TTIPRI);
	}
	spl0();
loop:
	bp = &canonb[2];
	while ((c=getc(&tp->t_rawq)) >= 0) {
		if (c==0377) {
			tp->t_delct--;
			/* so may receive more than one char in raw mode */
			if( tp->t_flags&RAW )
				continue;
			break;
		}
		if ((tp->t_flags&RAW)==0) {
			if (bp[-1]!='\\') {
				if (c==tp->t_erase) {
					if (bp > &canonb[2])
						bp--;
					continue;
				}
				if (c==tp->t_kill)
					goto loop;
				if (c==CEOT)
					continue;
			} else
			if (maptab[c] && (maptab[c]==c || (tp->t_flags&LCASE))) {
				if (bp[-2] != '\\')
					c = maptab[c];
				bp--;
			}
		}
		*bp++ = c;
		if (bp>=canonb+CANBSIZ)
			break;
	}
	bp1 = bp;
	bp = &canonb[2];
	c = &tp->t_canq;
	while (bp<bp1)
		putc(*bp++, c);
	return(1);
}

/*
 * Place a character on raw TTY input queue, putting in delimiters
 * and waking up top half as needed.
 * Also echo if required.
 * The arguments are the character and the appropriate
 * tty structure.
 */
ttyinput(ac, atp)
struct tty *atp;
{
	extern int cblkct;	/* number of busy blocks in clist */
	register int t_flags, c;
	register struct tty *tp;

	tp = atp;
	c = ac;
	t_flags = tp->t_flags;
	if (((c =& 0177) == '\r'  || c == 037  ) && t_flags&CRMOD)
		c = '\n';
	if ((t_flags&RAW)==0 && (c==CQUIT || c==CINTR)) {
		signal(tp, c==CINTR? SIGINT:SIGQIT);
		if (tp->t_bp && tp->t_bp->b_wcount) {

		/*
		 * abort DMA (large buffer) output by suspending
		 * output by setting HOLD and setting buffer
		 * error bit and restarting transmission.
		 */

			tp->t_state =| (TIMEOUT|HOLD);
			ttstart(tp);
			tp->t_bp->b_flags =| B_ERROR;
		}
		flushtty(tp);
		return;
	}
	if((t_flags&RAW)==0 &&  (c==CHOLD || c==CACK)) {
		if(tp->t_state&HOLD) {
			tp->t_state =& ~HOLD;
			ttrstrt(tp);
		} else {
			tp->t_state =| (TIMEOUT|HOLD);
			ttstart(tp);	/* abort DMA transfer if any */
		}
		return;
	}
	if (c == '\n')
		tp->t_col = 0; /* so tabs expand in half dpx ok */
#ifdef XSTTY
	if(c==021 || (tp->t_speeds&SUSNL) == 0)
		if(tp->t_state&HOLD) {
			tp->t_state =& ~HOLD;
			ttrstrt(tp);
		}
/* temp kludge to make enad file xfer work. if nl3 & 021 received */
/* then 021 is tossed out and output suspend is cleared */
	if(c==021 && (tp->t_speeds&SUSNL))
		return;
/* ......................................................*/
#endif
	if (tp->t_rawq.c_cc>=TTYHOG) {
		flushtty(tp);
		return;
	}
	if (t_flags&LCASE && c>='A' && c<='Z')
		c =+ 'a'-'A';
	if(cblkct >= NCLIST-20) {
		flushtty(tp);
/*
		printf("ttyi clist overrun dev = %d/%d\n",
		tp->t_dev.d_major, tp->t_dev.d_minor);
*/
		return;
	}
	putc(c, &tp->t_rawq);
	if (t_flags&RAW || c=='\n' || c==004 || c==021) {
		wakeup(&tp->t_rawq);
		if (putc(0377, &tp->t_rawq)==0)
			tp->t_delct++;
	}
	if(( c == tp->t_kill) && ((tp->t_flags&RAW) == 0)) {
		ttyoutput('x',tp);
		ttyoutput('x',tp);
		ttyoutput('x',tp);
		ttyoutput('\n',tp);
		ttstart(tp);
		return;
	}
	if (t_flags&ECHO) {
		if(c == tp->t_erase) 
			switch((tp->t_flags>>10)&03) {
			case 3:	ttyoutput('\010',tp);
				ttyoutput('\040',tp);
				c = '\010';
				break;
			case 2:	;
			case 1:	;
			case 0:	;
			}
		ttyoutput(c, tp);
		ttstart(tp);
	}
}

/*
 * put character on TTY output queue, adding delays,
 * expanding tabs, and handling the CR/NL bit.
 * It is called both from the top half for output, and from
 * interrupt level for echoing.
 * The arguments are the character and the tty structure.
 */
ttyoutput(ac, tp)
struct tty *tp;
{
	register int c;
	register struct tty *rtp;
	register char *colp;
	int ctype;
	extern int cblkct;	/* count of cblocks in use (putc/m45.s) */

	rtp = tp;
	c = ac;
	/*
	 * If clist almost full - blast this guy  -ghg 10/14/77
	 */
	if(cblkct >= NCLIST-20) {
		flushtty(rtp);
/*
		printf("ttyo clist overrun dev = %d/%d\n", 
		  rtp->t_dev.d_major, rtp->t_dev.d_minor);
*/
	}
	/*
	 * Turn tabs to spaces as required
	 */
	if ((c&0177)=='\t' && rtp->t_flags&XTABS) {
		do
			ttyoutput(' ', rtp);
		while (rtp->t_col&07);
		return;
	}
	/*
	 * for upper-case-only terminals,
	 * generate escapes.
	 */
	if (rtp->t_flags&LCASE) {
		c =& 0177;
		colp = "({)}!|^~'`";
		while(*colp++)
			if(c == *colp++) {
				ttyoutput('\\', rtp);
				c = colp[-2];
				break;
			}
		if ('a'<=c && c<='z')		
			c =+ 'A' - 'a';		
	}
/*
 *	display control characters if desired
 */
	if(rtp->t_flags<0){
		c =& 0177;
		ctype = partab[c];
		if((ctype&077) == 1 && c != 007 ) {
			ttyoutput('^',rtp);
			c =| 0100;
		}
	}
#ifdef XSTTY
#ifdef APL
	/*
	 * This code handles the weirdities of Tony's APL, and requires
	 * PDH'S specially modified Lear Siegler ADM-3a CRT's.
	 * --ghg 07/29/78.
	 */

	if (rtp->t_speeds & APLMOD) {
		if (c == CAPL_BS) {
			rtp->t_state =| APLSTRK;
			putc(CBS, &rtp->t_outq);
			return;
		}
		if (rtp->t_state & APLSTRK) {
			rtp->t_state =& ~APLSTRK;
			putc(017, &rtp->t_outq);        /* control-O */
			putc(aplstrike(rtp->t_achr, c), &rtp->t_outq);
			putc(016, &rtp->t_outq);        /* control-N */
			rtp->t_achr = 0;
			return;
		}
		rtp->t_achr = c;        /* save for next time around */
	}
#endif
#endif

	/*
	 * turn <nl> to <cr><lf> if desired.
	 */
	if ((c&0177)=='\n' && rtp->t_flags&CRMOD)
		ttyoutput('\r', rtp);
	if((rtp->t_flags&RAW)==0) {
		c =& 0177;
	} else {
		putc(c, &rtp->t_outq);
#ifdef PUMP.
		if(rtp->t_speeds&PUMP)
			return;
#endif
		rtp->t_col++;
		if(c == '\n')
			rtp->t_col = 0;
		return;
	}
	if (putc(c, &rtp->t_outq))
		return;
	/*
	 * Calculate delays.
	 * The numbers here represent clock ticks
	 * and are not necessarily optimal for all terminals.
	 * The delays are indicated by characters above 0200,
	 * thus (unfortunately) restricting the transmission
	 * path to 7 bits.
	 */
	colp = &rtp->t_col;
	ctype = partab[c];
	c = 0;
	switch (ctype&077) {

	/* ordinary */
	case 0:
		(*colp)++;

	/* non-printing */
	case 1:
		break;

	/* backspace */
	case 2:
		if (*colp)
			(*colp)--;
		break;

	/* newline */
	case 3:
		ctype = (rtp->t_flags >> 8) & 03;
		if(ctype == 1) { /* k4080d */
			putc(0, &rtp->t_outq);
		} else
		if(ctype == 2) { /* vt05 */
			c = 6;
		} else
		if(ctype == 3)	/* suspend output, CDC1700 */
			c = 0176;
		*colp = 0;
		break;

	/* tab */
	case 4:
		ctype = (rtp->t_flags >> 10) & 03;
		if(ctype == 1) { /* tty 37 */
			c = 1 - (*colp | ~07);
			if(c < 5)
				c = 0;
		}
		*colp =| 07;
		(*colp)++;
		break;

	/* vertical motion */
	case 5:
		if(rtp->t_flags & VTDELAY) { /* tty 37 */
			c = 0177;
			break;
		}
		if(((rtp->t_flags >> 8) & 03) == 1) { /* k4080d */
			putc(0,&rtp->t_outq);
			putc(0,&rtp->t_outq);
			break;
		}
		break;

	/* carriage return */
	case 6:
		ctype = (rtp->t_flags >> 12) & 03;
		if(ctype == 1) { /* tn 300 */
			c = 5;
		} else
		if(ctype == 2) { /* ti 700 */
			c = 10;
		}
		*colp = 0;
	}
	if(c)
		putc(c|0200, &rtp->t_outq);
}

# ifdef XSTTY
# ifdef APL

/*
 * Takes 2 characters and returns appropriate character (from apltab)
 * to make the APL overstrike character appear on the screen of the
 * specially modified Lear-Siegler ADM-3a's.
 * The character returned is just the index into the structure
 * (starting at 1).  This probably won't compile correctly
 * under V6 C.
 */

aplstrike(c1, c2)
char c1, c2;
{
	register struct apltab *ap;
	register char rc1, rc2;
	char *temp;

	rc1 = c1; rc2 = c2;
	for (ap = apltab; ap->chr1; ap++) {
		if (rc1 == ap->chr1 && rc2 == ap->chr2) {
			temp = ap - apltab;
			return(temp + ' ' + 1);
		}
		if (rc2 == ap->chr1 && rc1 == ap->chr2) {
			temp = ap - apltab;
			return(temp + ' ' + 1);
		}
	}
	return(007);    /* for now */
}

# endif
# endif
/*
 * Restart typewriter output following a delay
 * timeout.
 * The name of the routine is passed to the timeout
 * subroutine and it is called during a clock interrupt.
 */
ttrstrt(atp)
{
	register struct tty *tp;

	tp = atp;
	if(tp->t_state&HOLD) return;
	tp->t_state =& ~TIMEOUT;
	ttstart(tp);
}

/*
 * Start output on the typewriter. It is used from the top half
 * after some characters have been put on the output queue,
 * from the interrupt routine to transmit the next
 * character, and after a timeout has finished.
 * If the SSTART bit is off for the tty the work is done here,
 * using the protocol of the single-line interfaces (KL, DL, DC);
 * otherwise the address word of the tty structure is
 * taken to be the name of the device-dependent startup routine.
 */
ttstart(atp)
struct tty *atp;
{
	register int *addr, c;
	register struct tty *tp;
	struct { int (*func)(); };

	tp = atp;
	if (addr = tp->t_addr) {
		if (tp->t_state&SSTART) {
			(*addr.func)(tp);
			return;
		}
		if ((addr->tttcsr&DONE)==0 || tp->t_state&TIMEOUT)
			return;
		if ((c=getc(&tp->t_outq)) >= 0) {
			if(tp->t_flags&RAW) {
				addr->tttbuf = c;
				return;
			}
			if (c<=0177)
				addr->tttbuf = c | (partab[c]&0200);
			else {
				if((c&0377) == 0376)  /*suspend output  */
					tp->t_state =| (HOLD|TIMEOUT);
				else {
					timeout(ttrstrt, tp, c&0177);
					tp->t_state =| TIMEOUT;
				}
			}
		}
	}
}

/*
 * Called from device's read routine after it has
 * calculated the tty-structure given as argument.
 * The pc is backed up for the duration of this call.
 * In case of a caught interrupt, an RTI will re-execute.
 */
ttread(atp)
struct tty *atp;
{
	register struct tty *tp;
	register char c;

	tp = atp;
#ifdef XSTTY
	if(tp->t_speeds&BLITZ) 
		return;
#endif
/*	if ((tp->t_state&CARR_ON)==0)
		return; */
	c = 0;
	if (tp->t_canq.c_cc || canon(tp))
#ifdef XSTTY
		while (tp->t_canq.c_cc && passc((c=getc(&tp->t_canq)))>=0)
			if(tp->t_speeds & SPY) {
				sputchar(c);
				c =  0;
			}
		if(c)
			if(tp->t_speeds & SPY) {
				sputchar(c);
			}
#endif
#ifndef XSTTY
		while (tp->t_canq.c_cc && passc((getc(&tp->t_canq)))>=0);
#endif
}

/*
 * Called from the device's write routine after it has
 * calculated the tty-structure given as argument.
 */
ttwrite(atp)
struct tty *atp;
{
	register struct tty *tp;
	register int c;
	int pri;
# ifdef XSTTY
	extern dhstart();
# endif

	tp = atp;
	pri = TTOPRI;
#ifdef XSTTY
	if(tp->t_speeds&BLITZ) {
		u.u_count = 0; /* fake successfull write */
		return;
	}
#ifdef PUMP.
	if(tp->t_speeds&PUMP) {
		tp->t_col = 0;
		pri = PUMPRI;
	}
#endif
#endif
	/*
	 * Code to force characters on another tty's input
	 * clist.  If file is seeked to max pos, then chars
	 * go onto rawq via ttyinput. Super user only.
	 * --ghg 03/12/78.
	 */
	if (u.u_uid == 0 && u.u_offset[0] == -1 && u.u_offset[1] == -1) {
		spl5();
		while ((c = cpass()) >= 0) {
			while(tp->t_rawq.c_cc > TTYHOG-50)
				sleep(&lbolt, TTIPRI+1);
			ttyinput(c,tp);
		}
		spl0();
		return;
	}

/*	if ((tp->t_state&CARR_ON)==0)
		return; */
# ifdef XSTTY
	if (tp->t_addr == dhstart && u.u_count >= DTHRESH && 
	   (u.u_count&01) == 0 && (u.u_base&01) == 0) {
		switch(tp->t_speeds & (SPEED|SPY|APLMOD)) {
		case B9600:
		case B19200:
		case B38400:
			switch(tp->t_flags & ~(HUPCL|TAB3|ECHO|EVENP|ODDP)) {
			case RAW:
			case CRMOD:
			case CRMOD+RAW:
			case XTABS+CRMOD:
			case XTABS+CRMOD+RAW:
				while(tp->t_state & DWRIT) {
					tp->t_state =| DWANT;
					sleep(&tp->t_state, TTOPRI+1);
				}
				tp->t_state =| DWRIT;
				ttwrt(tp,pri); /* this must be interlocked */
				if (tp->t_state & DWANT)
					wakeup(&tp->t_state);
				tp->t_state =& ~(DWANT|DWRIT);
				return;
			}
		}
	}
# endif
	while ((c=cpass())>=0) {
		spl5();
		while (tp->t_outq.c_cc > TTHIWAT) {
			ttstart(tp);
			tp->t_state =| ASLEEP;
			sleep(&tp->t_outq, pri);
		}
		spl0();
		ttyoutput(c, tp);
#ifdef XSTTY
		if (tp->t_speeds & SPY) {
			sputchar(c);
		}
#endif
	}
	spl5();
	ttstart(tp);
# ifdef XSTTY
	/*
	 * If last write was a short one that did not go through
	 * large buffer pool, then give the buffer back as this
	 * guy is either about finished or is using short writes.
	 */

	if ((tp->t_state&DWRIT) == 0) {
		while(tp->t_bp && (tp->t_bp->b_flags & B_BUSY))
			sleep(tp->t_bp, pri);
		if (tp->t_bp && (tp->t_state&DWRIT) == 0) {  
			brelse(tp->t_bp); /* buffer may have vanished */
			tp->t_bp = 0;
		}
	}
# endif
	spl0();
}

# ifdef XSTTY
/*
 * ttwrt - sets up disk buffers for DMA output for fast dev's
 * Must be called with u.u_count even.
 * --ghg 02/18/78.
 */


ttwrt(tp,pri)
struct tty *tp;
{
	register char *cp1, *cp2;
	register a;
	int wc;
	struct buf *bp;
	char bfr[256];	/* hope stack doesn't blow!! */
	int xtabs;
	char *colp;
	char *ba;

	xtabs = 0;
	if (pri >= 0)
		pri = DPRI;
	colp = &tp->t_col;
	if ((bp=tp->t_bp) == NULL) {
		bp = getblk(NODEV2);	/* snarf disk buffer */
		bp->b_flags =& ~(B_BUSY|B_ERROR);
		bp->b_wcount = 0;
		bp->b_resid = 0;
		tp->t_bp = bp;
	}
	spl5();
	while(bp->b_flags & B_BUSY) {
		sleep(bp, pri);
	}
	spl0();
	bp->b_flags =& ~B_ERROR;
# ifdef XBUF
	*ka5 = bp->b_par;
	ba = &b;
# endif
# ifndef XBUF
	ba = bp->b_addr;
# endif
	cp2 = ba;
	while(u.u_count) {
		if ((tp->t_flags & ~(HUPCL|TAB3|ECHO|ODDP|EVENP)) == RAW) {
			a = min(u.u_count,512); /* only 1 copy needed */
			if (copyin(u.u_base, ba, a)) {
				u.u_error = EFAULT;
				return;
			}
			u.u_base =+ a;
			u.u_count =- a;
			wc = a;
			goto out;
		}
		a = min(u.u_count, 256);
		if(copyin(u.u_base, &bfr, a)) {
			u.u_error = EFAULT;
			return;
		}
		u.u_base =+ a;
		u.u_count =- a;
		cp1 = &bfr;
		switch(tp->t_flags & ~(HUPCL|TAB3|ECHO|ODDP|EVENP)) {

		case CRMOD+XTABS:
		case CRMOD+XTABS+RAW:
			xtabs++;
		case CRMOD:
		case CRMOD+RAW:
			while(a--) {
				switch(*cp2++ = *cp1++) {
				case '\n':
					*colp = 0;
					*--cp2 = '\r';
					if(++cp2 == (ba + 512)) {
						wc = 512;
						ptbuf(bp,tp,&wc,pri);
						cp2 = ba;
					}
					*cp2++ = '\n';
					break;

				case '\t':
					if(!xtabs)
						break;
					cp2--;
					do {
						*cp2++ = ' ';
						(*colp)++;
						if(cp2 == (ba + 512)) {
							wc = 512;
							ptbuf(bp,tp,&wc,pri);
							cp2 = ba;
						}
					} while(*colp&07);
					break;

				default:
					if (cp2[-1] >= ' ')
						(*colp)++;
				}
				if(cp2 == (ba + 512)) {
					wc = 512;
					ptbuf(bp,tp,&wc,pri);
					cp2 = ba;
				}
			}
			wc = cp2 - ba;
			break;

		default:
			printf("bad call to ttwrt: 0%o\n", tp->t_flags);
			u.u_error = ENXIO;
			return;
		}
out:
		if (wc == 512) {
			ptbuf(bp,tp,&wc,pri);
			cp2 = ba;
		}
	}
	if (wc)
		ptbuf(bp,tp,&wc,pri);
}

ptbuf(abp,atp,wc,pri)
struct buf *abp;
struct tty *atp;
int *wc;
{
	register struct buf *bp;
	register struct tty *tp;

	bp = abp;
	tp = atp;
	if(bp->b_flags&B_ERROR)
		goto out;
	spl5();
	bp->b_resid = bp->b_addr;
	while (tp->t_outq.c_cc || (tp->t_state&BUSY)) {
		tp->t_state =| ASLEEP;
		sleep(&tp->t_outq, pri);
	}
	bp->b_flags =| B_BUSY;
	bp->b_wcount = *wc;
	while(bp->b_flags & B_BUSY) {
		ttstart(tp);
		sleep(bp, pri);
	}
	spl0();
out:
	*wc = 0;
	bp->b_wcount = 0;
	bp->b_resid = 0;
# ifdef XBUF
	*ka5 = bp->b_par;
# endif
}

# endif
/*
 * Common code for gtty and stty functions on typewriters.
 * If v is non-zero then gtty is being done and information is
 * passed back therein;
 * if it is zero stty is being done and the input information is in the
 * u_arg array.
 */
ttystty(atp, av)
int *atp, *av;
{
	register  *tp, *v;
#ifdef PUMP.
	extern pumpck();
#endif

	tp = atp;
	if(v = av) {
#ifdef XSTTY
		*v++ = tp->t_speeds& ~SPY;
#endif
#ifndef XSTTY
		*v++ = tp->t_speeds;
#endif
		v->lobyte = tp->t_erase;
		v->hibyte = tp->t_kill;
		v[1] = tp->t_flags;
		/* Temporary rand change for empty */
		u.u_arg[4] = (tp->t_canq.c_cc==0 &&
			tp->t_delct==0);
		return(1);
	}
	v = u.u_arg;
#ifdef XSTTY
	if(*v & SPY) {
		if(u.u_uid || *pupd)
			return(1);
		tp->t_speeds =^ SPY;
		return(1);
	}
	if (tp->t_speeds & BLITZ)
		return(1);	/* only blitz once */
	if(u.u_uid)
		*v =& ~BLITZ;	 /* make blitz su only */
	*v =| tp->t_speeds&(SPY|BLITZ);	/* preserve spy and blitz bits */
	tp->t_speeds = *v;
/* Don't allow users to set following bits (su only) */
	if(u.u_uid)
		tp->t_speeds =& ~(PUMP|TOEOB);
	if(*v++ & BLITZ) {
		flushtty(tp);
		nulltty();	/* reassign tty files to null dev */
	}
	else
		wflushtty(tp);
#ifdef PUMP.
	spl6();
	if ((tp->t_speeds&TOEOB) && (tp->t_state&TIMACT) == 0) {
		tp->t_state =| TIMACT;
		tp->t_col = 0;
		timeout(&pumpck, tp, 60);
	}
	spl0();
#endif
#endif
#ifndef XSTTY
	wflushtty(tp);
	tp->t_speeds = *v++;
#endif
	tp->t_erase = v->lobyte;
	tp->t_kill = v->hibyte;
	tp->t_flags = v[1];
	return(0);
}

#ifdef PUMP.
/*
 * Routine to send PUMP end of block (rubout) incase the user mode
 * PUMP process gets tied up waiting for disk I/O to complete for
 * over 2 seconds.  This keeps the line from going out of sync.
 * Warning: This code uses t_col for a scratch counter.  If  this
 * port is running in PUMP mode, then nobody else should use t_col.
 * --ghg 10/09/77
 */

pumpck(atp)
{
	register struct tty *tp;

	tp = atp;
	if((tp->t_speeds&TOEOB) == 0) {
		tp->t_state =& ~TIMACT;
		return;
	}
	timeout(&pumpck, tp, 60);
	if(tp->t_outq.c_cc)
		goto out;
	if(++tp->t_col == 2) {
		putc(0377, &tp->t_outq);
		ttstart(tp);
		goto out;
	}
	return;

out:	tp->t_col = 0;
}
#endif
#ifdef XSTTY

/*
 * Track down all file descripters which point to this tty
 * which is being blitzed and reassign to /dev/null. Then
 * close this tty file.  u.u_ar0[R0] is user file desc.
 * --ghg 02/11/78.
 */

nulltty()
{
	register struct file *fp;
	register char *ip;
	register struct inode *ipp;
	struct file *fpp;
	struct file filex;
	struct file *fq;
	int count;
	extern schar();

	if ((fpp = getf(u.u_ar0[R0])) == NULL)
		return;
	ip = fpp->f_inode;
	if (ip->i_count == 1)	/* no reassign needed */
		return;
	u.u_dirp = "/dev/null";
	u.u_error = 0;
	if((ipp = namei(&schar,1)) == NULL)
		return;
	prele(ipp);
loop:
	plock2(ip);
	sleep(&lbolt,-1);	/* wait for everyone to wakeup and */
	sleep(&lbolt,-1);	/* and abort I/O in progress */
	if (ip->i_flag & IWANT) {
		prele(ip);	/* let other close's finish up */
		sleep(&lbolt,-1); /* let others in wanting this inode */
		goto loop;
	}
	prele(ip);
	for(fp = &file[0]; fp <  &file[NFILE]; fp++)
		if(ip == fp->f_inode)
			fp->f_inode = ipp;
	count = ip->i_count + ipp->i_count - 1;
	if(count > 127) {
		printf("TTY BLITZ botch up - call system staff\n");
		sleep(1,-2);
	}
	ipp->i_count = count;
	ip->i_count = 1;
	/*
	 * All this messing around with fake file entries is needed
	 * since routine closei()/fio.c has been eliminated from
	 * PWB unix, and a call to closef() must be used instead.
	 */
	fq = &filex;
	fq->f_inode = ip;
	fq->f_count = 1;
	fq->f_flag = fpp->f_flag;
	closef(fq);
}
#endif
