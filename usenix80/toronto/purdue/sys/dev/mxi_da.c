#

/*
 *      mx imp DA11B line driver
 */

#include "mx.h"
#include "mxi.h"

/*
 * da11b hardware bits
 */
#define GO      01      /* start dr11b dma */
#define OMODE   02
#define ODIR	04	/* 0 = xmit, 1 = receive */
#define OINTR	010	/* interrupt other machine */
#define IE      0100    /* enable interrupts */
#define READY   0200
#define CYCLE	0400	/* force bus cycle to occur */
#define IMODE	01000	/* status of OMODE from other machine */
#define IDIR    02000   /* status of ODIR from other machine */
#define IINTR	04000	/* status of OINTR from other machine */
#define MAINT   010000  /* maint mode */
#define ATTN    020000  /* set briefly when IINTR or INIT in other machine */
#define NEX     040000  /* nonexistent memory */
#define ERROR   0100000 /* set by NEX or ATTN or ba overflow */


/*
 * da11b registers
 */
struct {
	int     wc;
	int     ba;
	int     st;
	int     db;
};

/*
 * per line data area
 */
struct mida {
	struct devdat d;        /* common prefix */
	char    e_spur;         /* spurrious ints */
	char    e_poweroff;     /* other machine power off */
	char    e_clr;          /* midaclr called */
	char    e_igrint;       /* ignored interrupt during recv */
	char    e_collide;      /* trans collided */
	char    e_retry;        /* retry timeouts */
	char    state;          /* driver state */
	char    flush;          /* flush has been recvd */
	pbn     ob;             /* current out buffer */
	pbn     ib;             /* current in buffer */
	char    rseq;           /* recv seq num */
	char    xseq;           /* xmit seq num */
	char    t_null;         /* null timer */
	char    sendnull;       /* null flag */
	char    t_sync;         /* resynch timer */
	char    t_retry;        /* retry timer */
	char    primary;        /* this side primary, other must be sec */
	struct head nullhead;   /* null buffer header */
} mida[NDA];

/*
 * state indexes
 */
#define IOFF    0               /* interrupt off */
#define IDLE    1
#define RECV    2
#define TRAN    3

/*
 * word mode csr->db flags
 */
#define W_TGO   0100000         /* transmitting */
#define W_FLUSH 040000          /* flush other side */
#define W_COUNT 07777           /* word count */



/*
 * entered at hardware level to perform "task" for da line "m"
 */
midatask(m,task,x)
{
	register struct mida *d;
	register int i,*csr;

	d = &mida[m];
	csr = d->d_csr;

	switch(task) {

	case TPACK:
		if(d->state != IDLE) break;
		midatran(m);
		break;

	case TTIMER:
		if(d->state != IOFF && d->t_sync++ > 20) {
			if(d->d_up) {
				midaclr(m);
				mxprf("MIDA%o timeout\n",m);
			} else {
				d->t_sync--;
			}
			break;
		}
		switch(d->state) {
		case IOFF:
			if(d->t_sync++ < 30) break;
			d->t_sync = 0;
			d->state = IDLE;
			csr->st = IE;
			break;

		case IDLE:
			if(d->t_null++ < (d->primary?10:11)) break;
			d->sendnull++;
			midatran(m);
			break;

		case RECV:  case TRAN:
			if(d->t_retry++ < 5) break;
			d->sendnull++;
			midatran(m);
			d->e_retry++;
			break;
		}
		break;

	case TINIT:
		csr = d->d_csr = x & 0177776;
		d->primary = x & 01;

	case TSTART:
		midaclr(m);
		d->state = IDLE;
		csr->st = IE;
		return(d);
	}
}

/*
 * da interrupt service
 */
midaint(m)
{
	register struct mida *d;
	register int *csr;
	register pbn bn;
	int oka5,i;

	oka5 = *ka5;
	d = &mida[m];
	csr = d->d_csr;
	if(!(csr->st&READY)) {
		d->e_spur++;
		goto out;
	}
ready:
	if(csr->st & ERROR) {
		for(i=0 ; i<100 ; i++);
		if(!(csr->st&ERROR)) goto ready;
		mxprf("MIDA%o err %o\n",m,csr->st);
		goto sync;
	}
	if(csr->db == -1 && (csr->st&07000) == 07000) {
		if(!d->e_poweroff)
			mxprf("MIDA%o poweroff\n",m);
		d->e_poweroff = 1;
		goto sync;
	}
	switch(d->state) {

	case IDLE:
		if(!(csr->db&W_TGO))
			goto out;
	recv:
		if(!d->ib && !(d->ib = mxgetfb())) {
			csr->db = 0;    /* out of recv buffers */
			csr->st = IE;
			csr->st = IE|OINTR;
			goto out;
		}
		i = csr->db&W_COUNT;
		if(i > ((PSIZE+1)>>1) || i <= 0) {
			mxprf("MIDA%o badcount\n",m);
			goto sync;
		}
		csr->wc = -i;
		csr->ba = (d->ib<<6)+PSTART;
		csr->db = 0;
		csr->st = GO|ODIR|IE|((d->ib>>6)&060);
		d->t_retry = 0;
		d->state = RECV;
		goto out;

	case RECV:
		if(csr->wc) {
			d->e_igrint++;
			goto out;
		}
		*ka5 = d->ib;
		if(b.h.h_seq != d->rseq++) {
			mxprf("MIDA%o seq\n",m);
			goto sync;
		}
		if(b.h.h_soh) {
			miqtask(d->ib);
			d->ib = 0;
		}
		d->d_up = 1;
		d->t_retry = d->t_sync = d->e_poweroff = 0;
		csr->db = 0;
		csr->st = IE;
		if(midatran(m))
			goto out;
		csr->st = IE|OINTR;
		goto out;

	case TRAN:
		csr->db = 0;
		csr->st = IE;
		if(csr->wc == -1) {
			if(!d->sendnull && d->ob) {
				mxputfb(d->ob);
				d->ob = 0;
			}
			d->sendnull = 0;
			d->xseq++;
			if(csr->db & W_TGO) {
				d->state = IDLE;
				goto recv;
			}
			midatran(m);
			goto out;
		}
		if(!(csr->db&W_TGO)) {  /* rec side out of bufs, try again */
			midatran(m);
			goto out;
		}
		d->e_collide++;
		if(d->primary) {
			if(csr->db&W_FLUSH) {
				midatran(m);
				goto out;
			}
		flush:
			csr->db = W_FLUSH|W_TGO;
			csr->st = IE|OINTR;
			goto out;
		} else {
			if(csr->db&W_FLUSH) {
				if(d->flush++ == 0) goto flush;
				goto out;
			}
			if(d->flush)
				goto recv;
			goto out;
		}
	default:
	sync:
		midaclr(m);
	}
out:
	*ka5 = oka5;
}

/*
 * clear and initialize da device
 */
midaclr(m)
{
	register struct mida *d;
	register int i,*ip;

	d = &mida[m];
	ip = &d->d_csr->db;
	for(i=0 ; i<4 ; i++)
		*ip-- = 0;
	d->d_up = d->d_xstate = d->d_rstate = 0;
	d->rseq = d->xseq = d->t_null = d->t_sync = d->t_retry = 0;
	d->sendnull = d->flush = 0;
	d->state = IOFF;
	d->e_clr++;
	while((i = mxgetb(&d->d_oq)))
		mirev(i);
	if(d->ob) {
		mirev(d->ob);
		d->ob = 0;
	}
}

/*
 * see if there is anything to transmit, if so start it up.
 */
midatran(m)
{
	register struct mida *d;
	register *csr;
	int newst,newdb;
	d = &mida[m];
	csr = d->d_csr;
	if(!d->sendnull && !d->ob && !d->d_oq) {
		d->d_xstate = 0;
		d->state = IDLE;
		return(0);
	}
	csr->st = IE;
	if(d->sendnull) {
		d->nullhead.h_seq = d->xseq;
		newst = 0;
		newdb = (sizeof d->nullhead +1)>>1;
		csr->ba = &d->nullhead;
	} else {
		if(!d->ob)
			d->ob = mxgetb(&d->d_oq);
		*ka5 = d->ob;
		b.h.h_soh = 1;
		b.h.h_seq = d->xseq;
		newst = (d->ob>>6) & 060;
		newdb = (&b.p.p_data[b.p.p_count] - &b.h + 1) >> 1;
		csr->ba = (d->ob<<6) + PSTART;
	}
	csr->wc = -(newdb+2);
	csr->db = newdb;        /* ensure count settled when TGO comes up */
	csr->db = newdb|W_TGO;
	csr->st = IE|OINTR|GO|newst;
	d->d_xstate++;
	d->state = TRAN;
	d->t_null = d->t_retry = 0;
	d->flush = 0;
	return(1);
}
