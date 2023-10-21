507a
	spl0();
.
506a
	spl6();
.
463a
	PS->integ = sps;
.
460c
			timeout(ttrstrt, tp, (c&0177) + DLDELAY);
.
457c
		if (tp->t_flags&RAW)
			addr->tttbuf = c;
		else if (c<=0177)
.
455a

	sps = PS->integ;
	spl5();
.
454c
	if ((addr->tttcsr&DONE)==0 || tp->t_state&(TIMEOUT|SUSPEND))
.
444a
	int sps;
.
347,349c
	 * The delays are indicated by characters above 0200.
	 * In raw mode there are no delays and the
	 * transmission path is 8 bits wide.
.
341,342c
	putc(c, &rtp->t_outq);
.
311a
	}
	c =& 0177;
.
310c
	if ( rtp->t_flags&RAW )  {
		putc(c, &rtp->t_outq);
.
307,308c
	 * In raw mode dump the char unchanged.
.
305c
	c = ac;
.
284a
		if (c=='\b' && c==tp->t_erase && (t_flags&RAW)==0) {
			ttyoutput(' ',tp);
			ttyoutput('\b',tp);
		}
		if (c==tp->t_kill && (t_flags&RAW)==0)
			ttyoutput('\n',tp);
.
280c
		if ((t_flags&RAW)==0 && putc(0377, &tp->t_rawq)==0)
.
278c
	if (t_flags&RAW || (c=='\n' || c==CEOT)) {
/* if EOT during output stall, force resume before EOT processing. */
		if (c==CEOT && (tp->t_state&SUSPEND))
			ttyinput (CSTART, tp);
.
271,274c
	else
		tp->t_state =& ~SUSPEND;

	if (tp->t_rawq.c_cc>TTYHOG) {
		ttyoutput( 0007, tp );	/* ring bell */
		return;			/* discard new char */
	}
.
264,269c
	if ((t_flags&RAW)==0) {
		c =& 0177;
		if (c == CSTOP) {
			tp->t_state =| SUSPEND;		/* suspend output */
			return;
		} else if (c == CSTART) {
			tp->t_state =& ~SUSPEND;	/* resume output */
			ttstart (tp);
			return;
		}
		if (c==CQUIT || c==CINTR) {
			signal(tp, c==CINTR? SIGINT:SIGQIT);
			flushtty(tp);
			return;
		}
		if (c=='\r' && t_flags&CRMOD)
			c = '\n';
.
262c
	c = ac&0377;
.
229,233c
			} else {
				mc = maptab[c];
				if (c==tp->t_erase || c==tp->t_kill)
					mc = c;
				if (mc && (mc==c || (tp->t_flags&LCASE))) {
					if (bp[-2] != '\\')
						c = mc;
					bp--;
				}
.
218a
			if (c==0377) {
				tp->t_delct--;
				break;
			}
.
214,217d
205c
	while ((tp->t_flags&RAW)==0 && tp->t_delct==0
	    || (tp->t_flags&RAW)!=0 && tp->t_rawq.c_cc==0)  {
.
201a
	int mc;
.
183a
	tp->t_char = 0;			/* DH delay */
	tp->t_state =& ~SUSPEND;
.
110c
 * v  is non-zero for gtty and is the place in which the device
.
62c
 * (that is, not DH or DZ).
.
36a
	000,000,000,000,000,000,000,000,
.
34d
30c
	000,'|',000,000,000,000,000,'`',
.
26c
	000,000,000,000,CEOT,00,000,000,
.
17a
#define	DLDELAY	4	/* Extra delay for DL's (double buff) */

.
4,7d
2a
 * tty.c - general TTY subroutines
 *	modified 05-Jun-1980 by D A Gwyn:
 *	1) added some of Ken's mods (not process groups);
 *	2) added scope rubout and stall support;
 *	3) don't flush input queue on overflow;
 *	4) fix ttwrite bug causing occasional char loss on DL.
.
w
q
