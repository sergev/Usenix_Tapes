#

/*
 *      mx imp DMC11 line driver
 */

#include "mx.h"
#include "mxi.h"

/*
 * DMC status bits, request codes.
 */
#define BUFR    4
#define	BUFX	0
#define	BASE	3
#define CTRL	1
#define	RDY	0200
#define	IE	0100
#define	RQ	040
#define MCLR	0300
#define	FATAL	01772
#define ORUN	04

/*#define TRACE   256   */

/*
 * DMC registers
 */
struct
{
	char	sin;
	char	smaint;
	char	sout;
	char	zz;
	int	sel4;
	int	sel6;
};

/*
 * per line data area.  initialized by mistart.
 */
struct  midm
{
	struct devdat d;        /* common prefix */
	pbn     ob;             /* current out buffer */
	pbn     ib;             /* current in buffer */
	char    rseq;           /* receive sequence expected */
	char    xseq;           /* xmit seq */
	char    xclock;         /* timer used to send idle packets */
	char    wait;           /* monitor busy waiting */
	char    abort;          /* dmc master cleared */
	char    reenter;        /* level of reentry */
	char    spurint;        /* weird spurrious interrupts */
	char    forceover;      /* force overruns for testing */
	int     orun;           /* number of overruns */
	int     base[128];      /* dmc scratchpad */
}midm[NDM];

#ifdef  TRACE
struct trace {
	char    tclock,trint,trseq,txseq,treenter;
	int     treg[4];
};
struct trace midmt[TRACE];
struct trace *midmtp;
#endif


/*
 * entered at hardware level to perform task "task" for dmc line "m".
 */
midmtask(m,task,x)
{
	register struct midm *d;
	static struct head nullhead;
	int i;
	register pbn bn;
	d = &midm[m];

	switch(task) {

	case TPACK:
		if(d->d_xstate) break;  /* if xmit already active */
		if(!(bn = mxgetb(&d->d_oq)))     /* nothing for us */
			break;
		d->d_xstate++;
		d->ob = bn;
		b.h.h_seq = d->xseq++;  /* plug next seq num into pack */
		b.h.h_soh = 1;          /* data, not null */
		midmin(m,BUFX,(bn<<6)+PSTART,
		  (&b.p.p_data[b.p.p_count] - &b.h) | ((bn<<4)&0140000));
		break;

	case TTIMER:
		if(d->d_xstate || d->xclock++ < 10)
			break;          /* if something recently sent */
		d->d_xstate++;
		midmin(m,BUFX,&nullhead,sizeof nullhead);
		break;

	case TINIT:
		d->d_csr = x;
	case TSTART:
		d->d_up = d->d_xstate = d->xseq = d->rseq = d->d_rstate =
			d->xclock = 0;
		d->d_csr->smaint = MCLR;
		for(i=0;i<1000;i++);
		while((bn = mxgetb(&d->d_oq)))
			mirev(bn);      /* flush oq */
		if(d->ob) {
			mirev(d->ob);
			d->ob = 0;
		}
		midmin(m,BASE,d->base,0);
		midmin(m,CTRL,0,0);
		d->d_csr->sout =| IE;
		d->abort = 1;           /* abort recursive calls */
		return(d);              /* give mitask our real address */
	}
}

/*
 * dmc hardware receive interrupt.  switch on type of dmc output message.
 */
midmrint(m)
{
	register struct midm *d;
	register char *ADDR;
	register pbn bn;
	int o2,o4,o6;
	unsigned oka5;

	oka5 = *ka5;            /* save mem mang reg */
#ifdef  TRACE
	midmtrace(1);
#endif
	d = &midm[m];
	ADDR = d->d_csr;
	o2 = ADDR->sout;  o4 = ADDR->sel4;  o6 = ADDR->sel6;
	if(!(o2&RDY)) {
		d->spurint++;
		return;
	}
	ADDR->sout =& ~RDY;
	switch(o2 & 7) {

	/*
	 * new buffer received
	 */
	case BUFR:
		d->d_up = 1;    /* now we are up */
		*ka5 = d->ib;
		if(b.h.h_soh) { /* if a data packet (non null) */
			if(b.h.h_seq != d->rseq++) {    /* bad seq num */
				mxprf("MIDMC%o seq %o %o\n",
				  m,b.h.h_seq,d->rseq-1);
				goto bad;
			}
			miqtask(d->ib);
			d->ib = mxgetfb();      /* next input buffer */
		}
	newread:
		d->d_rstate = 0;        /* recv idle */
		if((bn = d->ib) && !d->forceover) {     /* if buffer avail */
			d->d_rstate = 1;
			midmin(m,BUFR,(bn<<6)+PSTART,PSIZE | ((bn<<4)&0140000));
		}       /* else receive overrun will interrupt later */
		break;

	/*
	 * transmit buffer complete
	 */
	case BUFX:
		if(!d->d_xstate)    /* "impossible" */
			break;
		d->d_xstate = d->xclock = 0;  /* xmit idle, restart timer*/
		if(d->ob) mxputfb(d->ob);
		d->ob = 0;
		if(d->d_oq) midmtask(m,TPACK);      /* try sending more */
		break;

	/*
	 * control message (error condition)
	 */
	case CTRL:
		if(o6 == ORUN) {        /* overrun: dmc wants new recv buf */
			d->orun++;
			if(d->d_rstate) /* "impossible" */
				break;
			if(!d->ib)
				d->ib = mxgetfb();      /* try again */
			goto newread;
		}
		mxprf("MIDMC%o cntlo %o\n",m,o6);
		if(o6&FATAL) goto bad;
		break;

	default:
	bad:
		midmtask(m,TSTART);     /* reset this line */
		break;
	}
	*ka5 = oka5;
}

/*
 * Cause the dmc to input a command with arguments.  The dmc is supposed to
 * be a "full-duplex" device, but in fact the device register interface
 * is half-duplex.  If while the pdp is giving a command to the dmc, the
 * dmc decides to tell the pdp something, the pdp must drop everything and
 * do what the dmc wants.
 */
midmin(m,i,s4,s6)
{
	int c;
	register struct midm *d;
	register char *ADDR;
	d = &midm[m];
	ADDR = d->d_csr;
start:  ADDR->sin = i | RQ;
	c = 0;
wait:   while(ADDR->sin >= 0  &&  ADDR->sout >= 0)
		if(d->wait++, c++ > 500)goto start;
	if(ADDR->sout < 0) {
		if(d->reenter++ > 5) {
			printf("MIDMC%o reenter\n",m);
			halt();
			d->reenter--;
			return;
		}
		ADDR->sin = 0;
		d->abort = 0;
		midmrint(m);    /* we may be reentered */
		d->reenter--;
		if(d->abort) return;
		goto start;
	}
	ADDR->sel4 = s4;
	ADDR->sel6 = s6;
#ifdef  TRACE
	midmtrace(0);
#endif
	ADDR->sin =& ~RQ;
	while(ADDR->sin & RDY) d->wait++;
}

#ifdef  TRACE
midmtrace(rint)
{
	register struct trace *tp;
	register struct midm *d;
	register int *ADDR;
	int i;
	extern lbolt;

	d = &midm[0];
	ADDR = d->d_csr;
	if(!midmtp)
		midmtp = &midmt[0];
	if(d->orun > 10) return;
	if(tp = midmtp++, midmtp == &midmt[TRACE])
		midmtp = &midmt[0];
	tp->trint = rint;
	tp->tclock = lbolt;
	tp->trseq = d->rseq;  tp->txseq = d->xseq;
	tp->treenter = d->reenter;
	for(i = 0; i<4 ; i++)
		tp->treg[i] = *ADDR++;
}
#endif
