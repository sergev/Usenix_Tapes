#

/*
 *      mx - multiplex device
 */


#include "mx.h"
#include "../param.h"
#include "../proc.h"
#include "../conf.h"
#include "../user.h"

#define NMIN    20              /* number of mx minor devices */
#define NCON    32              /* number of connections */
#define NSOC    48              /* number of sockets */
#define FSOC    5               /* first socket not reserved for root servers */


/*
 * minor device table
 */
struct min {
	char    m_flag;
	struct con *m_con;      /* if nz, points to active con entry */
} mxmin[NMIN];                  /* mxmin[0] never used */

/* m_flag values */

#define m_open  1               /* device open */


/*
 * socket allocated flags
 */
char    mxsoc[NSOC];            /* nz=allocated.  mxsoc[0] unused */


/*
 * connection table
 */
struct con {
	char    c_fhost;        /* foreign host */
	char    c_fsoc;         /* foreign socket */
	char    c_lhost;        /* local host, =LHOST (for convenience) */
	char    c_lsoc;         /* local socket */
	pbn     c_ib;           /* input buffer */
	pbn     c_ob;           /* output buffer */
	int     c_flag;
	char    c_min;          /* if nz, associated open minor device */
} mxcon[NCON];

/* c_flag values */

#define c_state         017     /* connection state */
#define c_diss          01      /* disconnect sent */
#define c_disr          02      /* disc received */
#define c_cons          04      /* connect sent */
#define c_conr          010     /* conn received */
#define c_abort         020     /* connection aborted */
#define c_lsocall       040     /* local socket allocated */
#define c_readleft      0100    /* data left in c_ib after last read */
#define c_wnext         0200    /* waiting for "next" from foreign host */
#define c_wread         0400    /* read waiting (sleeping) */
#define c_wwrite        01000   /* write waiting (sleeping) */
#define c_wany          02000   /* waiting for any connect on c_lsoc */
#define c_flush         04000   /* flush c_ob when timer comes back */
#define c_eofs          010000  /* eof sent */
#define c_eofr          020000  /* eof received */
#define c_inuse         0100000 /* entry in use */



/* mxcstat() return values */

#define COPEN   1
#define CWAIT   2
#define CCLOSED 3

/* sleep priorities */

#define PRIC    4               /* connect */
#define PRIR    1               /* read */
#define PRIW    1               /* write */
#define PRIB    2               /* buffer wait */

/* stty functions */

#define SCON    1               /* connect */
#define SDIS    2               /* disconnect */
#define SWAIT   3               /* wait for connection */
#define SRESET  4               /* reset mx driver */
#define SPGRP   5               /* set process group */
#define SSIG    6               /* send signal */
#define SEOF    7               /* send end of file */




/*
 * local variables
 */
struct mx {
	char    nfree;          /* number of free buffers */
	char    lfree;          /* lowest number of free buffers */
	pbn     free;           /* free list */
	char    flush;          /* mxflush() running */
	char    rall;           /* mxrall() active */

/* error counters */

	char    e_rst;          /* host resets */
	char    e_outsoc;       /* out of sockets */
	char    e_outcon;       /* out of cons */
	char    e_ibnz;         /* c_ib nz on packet arrival ("impossible") */
} mx;



mxopen(dev)
{
	static initdone;
	register struct min *m;
	register i;
	extern mxrall();

	if(dev == NODEV) {      /* power fail */
		if(initdone && !mx.rall) {
			mistart();
			mx.rall++;
			timeout(mxrall,0,240);
		}
		return;
	}
	if(!initdone) {
		initdone++;
		mxreset();
	}
	i = dev.d_minor & 0377;
	if(i == 0 || i >= NMIN) goto bad;
	m = &mxmin[i];
	if(m->m_flag&m_open) goto bad;
	m->m_flag = m_open;
	return;
bad:
	u.u_error = ENXIO;
}

mxclose(dev)
{
	register i;
	i = dev.d_minor;
	spl1();
	mxdisc(i);              /* disconnect (clears m_con) */
	spl0();
	mxmin[i].m_flag = 0;    /* now closed */
}

mxsgtty(dev,v)
{
	register i;
	register struct con *c;
	extern praddr;

	if(v) goto bad;         /* no gtty */
	i = dev.d_minor;
	spl1();
	switch(u.u_arg[0]) {

	default:
	bad:
		u.u_error = ENXIO;
		break;

	case SCON:              /* connect */
		mxconn(i);
		break;

	case SDIS:               /* disconnect */
		mxdisc(i);
		break;

	case SWAIT:             /* wait for connection */
		mxwait(i);
		break;

	case SRESET:            /* reset mx driver */
		if(suser())
			mxreset();
		break;

	case SPGRP:             /* set process group */
		*ka5 = praddr;
		u.u_procp->p_ttyp = &mxmin[i];
		break;

	case SSIG:              /* send signal */
		if(!(c = mxmin[i].m_con) || mxcstat(c) != COPEN) goto bad;
		i = mxnewfb(c); /* format up a host control block */
		b.p.p_hostc = ph_sig;
		b.p.p_data[0] = u.u_arg[1];     /* signal number */
		b.p.p_count = 1;
		mxsend(i);
		break;

	case SEOF:              /* send end of file */
		if(!(c=mxmin[i].m_con) || mxcstat(c) != COPEN ||
		   (c->c_flag&c_eofs)) goto bad;
		c->c_flag =| c_eofs;
		mxsendob(c);
		mxsendc(c,ph_eof);
		break;
	}
	spl0();
}

mxread(dev)
{
	register n,i;
	register struct con *c;
	spl1();
	if(!(c = mxmin[dev.d_minor].m_con))     /* not connected */
		goto bad;
	while(!c->c_ib && mxcstat(c) == COPEN && !(c->c_flag&c_eofr)) {
		c->c_flag =| c_wread;           /* wait for data */
		sleep(&c->c_ib,PRIR);
	}
	if(!c->c_ib) {          /* if disconnected and drained */
		if(c->c_flag&c_abort) goto bad; /* abnormal disconnect */
		spl0();
		return;         /* no bytes read */
	}
	*ka5 = c->c_ib;         /* access buffer */
	n = min(u.u_count, b.p.p_count);        /* number of bytes for user */
	if(c->c_flag&c_readleft)
		i = b.h.h_check;
	else
		i = &b.p.p_data;
	bcopyout(i,u.u_base,n); /* move kernel to user space */
	u.u_base =+ n;
	u.u_count =- n;
	if(b.p.p_count =- n) {          /* bytes leftover */
		b.h.h_check = i+n;
		c->c_flag =| c_readleft;
	} else {
		c->c_flag =& ~c_readleft;
		mxputfb(c->c_ib);
		c->c_ib = 0;
		mxsendc(c,ph_next);     /* ready for next packet */
	}
	spl0();
	return;
bad:
	spl0();
	u.u_error = ENXIO;
}

mxwrite(dev)
{
	register n,i;
	register struct con *c;
	extern mxflush();

	spl1();
	if(!(c = mxmin[dev.d_minor].m_con) ||
	    mxcstat(c) != COPEN || (c->c_flag & c_eofs)) goto bad;
	while((i = mxcstat(c)) == COPEN && u.u_count > 0) {
		if(c->c_flag&c_wnext) { /* waiting for ph_next */
			c->c_flag =| c_wwrite;
			sleep(&c->c_ob, PRIW);
			continue;       /* still open ? */
		}
		if(!c->c_ob)
			c->c_ob = mxnewfb(c);
		else
			*ka5 = c->c_ob;
		n = min(u.u_count, PDSIZE-b.p.p_count);
		bcopyin(u.u_base,&b.p.p_data[b.p.p_count],n);
		u.u_base =+ n;
		u.u_count =- n;
		b.p.p_count =+ n;
		if(n > 4 || b.p.p_count == PDSIZE) {    /* send it now */
			mxsendob(c);
		} else {                        /* flush it later */
			c->c_flag =| c_flush;
			if(!mx.flush) {
				mx.flush++;
				timeout(mxflush,0,3);
			}
		}
	}
	if(i == CCLOSED) goto bad;
	spl0();
	return;
bad:
	spl0();
	u.u_error = ENXIO;
}

/*
 * Try to connect to a foreign host and socket (in u.u_arg[1] and [2] of the
 * stty call).  Allocate a unique local socket number on our end; since
 * the local socket number is not chosen by the user it's unlikely that
 * that the connect is in the con table already, but we look anyway.
 * Make an entry in the con table, associate this minor device with it,
 * and send the connect function to the foreign host.  mxrecv will
 * wake us up when the matching connect (or disconnect) function returns
 * from the foreign host.  mxretcon will deallocate the socket when the
 * connection is closed.
 */
mxconn(m)
{
	struct min *mp;
	register i,s;
	register struct con *c;
	mp = &mxmin[m];
	if(mp->m_con)           /* if already connected */
		goto bad;
	if(!u.u_arg[1])         /* foreign host=0 illegal */
		goto bad;
	for(i=FSOC ; i<NSOC ; i++)
		if(mxsoc[i] == 0) break;
	if(i == NSOC) {         /* no sockets left */
		mx.e_outsoc++;
		goto bad;
	}
	s = i;
	mxsoc[s]++;
	if(!(c = mxfindc(u.u_arg[1],u.u_arg[2],s)) &&
	   !(c = mxnewc(u.u_arg[1],u.u_arg[2],s)) ) {
		mxsoc[s] = 0;
		goto bad;       /* no con waiting and no room in con */
	}
	mp->m_con = c;  c->c_min = m;
	c->c_flag =| (c_lsocall|c_cons);
	mxsendc(c,ph_con);
	while((i=mxcstat(c)) == CWAIT) sleep(c,PRIC);
	if(i == CCLOSED) {
		mxdisc(m);
		goto bad;
	}
	return;
bad:
	u.u_error = ENXIO;
}

/*
 * Disconnect this minor device.  Disassociate the min and con structs so
 * that mxretcon can complete the disconnection.
 */
mxdisc(m)
{
	struct min *mp;
	struct con *c;
	mp = &mxmin[m];
	if((c = mp->m_con) == 0)        /* already disconnected */
		return;
	mxsendob(c);
	c->c_min = 0;  mp->m_con = 0;
	mxretcon(c);
}

/*
 * Wait for a connection from any foreign host to the user's local socket
 * number (u.u_arg[1] of the stty).  Local socket numbers below FSOC are
 * reserved for root server daemons.  The foreign connect may have already
 * arrived, so the con table is searched first.  If found, the connection
 * is immediately open; else a new con entry is filled in and the c_wany
 * flag set (mxrecv will awaken us when a matching connect is received).
 * The local socket number is allocated only during the waiting period.
 * After the connection completes (or is aborted), another wait on the
 * same local socket will be accepted.
 */
mxwait(m)
{
	struct min *mp;
	register struct con *c;
	register i,s;
	mp = &mxmin[m];
	if(c = mp->m_con)       /* if already connected */
		goto bad;
	s = u.u_arg[1] & 0377;  /* local socket */
	if(s >= NSOC || (s < FSOC && !suser()) || mxsoc[s])
		goto bad;
	for(c = &mxcon[0] ; c < &mxcon[NCON] ; c++) {
		if((c->c_flag&c_state) != c_conr) continue;
		if(c->c_lsoc != s) continue;
		c->c_min = m;   /* associate min and con */
		mp->m_con = c;
		c->c_flag =| c_cons;
		mxsendc(c,ph_con);
		return;
	}

	/* matching con entry not found, make a new one. */

	if(!(c = mxnewc(0,0,s))) goto bad;
	c->c_flag =| (c_wany|c_lsocall);
	mxsoc[s]++;
	c->c_min = m;
	mp->m_con = c;
	while(mxcstat(c) == CWAIT) sleep(c,PRIC);
	c->c_flag =& ~c_lsocall;
	mxsoc[s] = 0;           /* deallocate socket */
	return;
bad:
	u.u_error = ENXIO;
}

/*
 * Return a connection (con struct).  If a minor device is associated
 * with this con, return until recalled by mxdisc (mxclose).  If the
 * foreign host knows about this connection, send a disconnect;  dont
 * consider the connection closed until both hosts agree.  At this time
 * deallocate the local socket and clean up the con struct.
 *
 * returns a 1 if con deallocated; else 0
 */
mxretcon(c)
register struct con *c;
{
	if(c->c_min) return(0);
	if(c->c_flag&(c_conr|c_cons|c_disr)) {  /* other host involved */
		if(!(c->c_flag&c_diss)) {       /* we havent sent disc */
			c->c_flag =| c_diss;
			mxsendc(c,ph_dis);
		}
		if((c->c_flag&(c_diss|c_disr)) != (c_diss|c_disr))
			return(0);              /* both hosts not closed */
	}
	if(c->c_flag&c_lsocall)
		mxsoc[c->c_lsoc] = 0;
	if(c->c_ib)                             /* return any buffers */
		mxputfb(c->c_ib);
	if(c->c_ob)
		mxputfb(c->c_ob);
	c->c_flag = c->c_ib = c->c_ob = 0;
	return(1);
}

/*
 * flush any write buffers that were not sent because they were too tiny.
 * called by clock timeout at level 5.
 */
mxflush()
{
	register struct con *c;
	register oka5;
	oka5 = *ka5;
	for(c = &mxcon[0] ; c < &mxcon[NCON] ; c++) {
		if(!(c->c_flag&c_flush)) continue;
		c->c_flag =& ~c_flush;
		mxsendob(c);
	}
	*ka5 = oka5;
	mx.flush = 0;
}

/*
 * send the out buffer (c_ob) for connection c.
 */
mxsendob(c)
register struct con *c;
{
	if(!c->c_ob) return;
	mxsend(c->c_ob);
	c->c_ob = 0;
	c->c_flag =| c_wnext;
}

/*
 * Receive a buffer from the mx imp.  Called at level 1 from the mx imp
 * driver (can be thought of as an interrupt service routine).  The user
 * entrypoints to the mx device (mxread, mxwrite, etc.) do overly
 * conservative locking at spl1(), to prevent mxrecv() from running
 * while internal structures (such as con) are being altered.
 *
 * There are problems with this organization:  (1) If a REAL imp-like
 * hardware device existed and this code had to be adapted for it, the
 * spl1's would have to be spl4 or spl5s, and running all the mxread, etc.
 * code at such a high level for interlocking would be unwise.  (2) The
 * code can't issue any sleep's since it runs as an interrupt routine.
 * Since mxsendc() eventually calls sleep if it can't get a buffer, some
 * kludges had to be introduced into mxrecv() to avoid this.  If in the
 * future it was decided to implement end to end retransmission/
 * acknowledge, the current organization would be very awkward if not
 * impossible.
 *
 * A better but significantly slower scheme (used in the Illinois ARPA-NET
 * UNIX), would first pass the imp-host buffers (perhaps through a queue)
 * to a "dummy" UNIX process (always running in kernel mode) which is
 * awakened on each buffer arrival.  This code would then have no problem
 * if it had to sleep momentarily while waiting on a free buffer;  also
 * the current spl1 interlocks could then be removed.
 *
 *
 * Try to find the connection this packet belongs to, then
 * dispatch on the host-host or imp-host control code.
 * Most control codes fall though to "out" where the buffer is returned.
 */
mxrecv(bn)
pbn bn;
{
	register struct con *c;
	register i;
	c = mxfindc(b.p.p_shost,b.p.p_ssoc,b.p.p_dsoc);

	if(b.p.p_impc) {        /* imp control code */
		mxhostr(b.p.p_shost);
		goto out;

	} else if(b.p.p_hostc) {/* host control code */

		switch(b.p.p_hostc) {
	
		/*
		 * connect.  if exact match, wakeup the user "connect" call;
		 * else search for c_wany's and wakeup the user "wait for connect"
		 * call; else just make a new con entry.
		 */
		case ph_con:
			if(c) {
				c->c_flag =| c_conr;
				wakeup(c);
				break;
			}
			for(c = &mxcon[0] ; c < &mxcon[NCON] ; c++) {
				if(!(c->c_flag&c_wany)) continue;
				if(c->c_lsoc != b.p.p_dsoc) continue;
				c->c_fhost = b.p.p_shost;
				c->c_fsoc = b.p.p_ssoc;
				c->c_flag =& ~c_wany;
				c->c_flag =| (c_conr|c_cons);
				spl7();         /* kludge, comments above */
				mxputfb(bn);
				mxsendc(c,ph_con);
				spl1();
				wakeup(c);
				return;
			}
			if(c = mxnewc(b.p.p_shost,b.p.p_ssoc,b.p.p_dsoc)) {
				c->c_flag =| c_conr;
			}
			break;
	
		/*
		 * disconnect.  if no match do nothing.  else try to
		 * return the connection.  wakeup any user calls.
		 */
		case ph_dis:
			if(!c) break;
			c->c_flag =| c_disr;
			spl7();         /* kludge, comments above */
			mxputfb(bn);
			i = mxretcon(c);
			spl1();
			if(!i) {
				wakeup(c);
				wakeup(&c->c_ib);  wakeup(&c->c_ob);
			}
			return;
	
		/*
		 * ready for next data block;  wakeup write if blocked.
		 */
		case ph_next:
			if(!c) break;
			c->c_flag =& ~c_wnext;
			if(c->c_flag&c_wwrite) {
				c->c_flag =& ~c_wwrite;
				wakeup(&c->c_ob);
			}
			break;
	
		/*
		 * host reset
		 */
		case ph_rst:
			mxhostr(b.p.p_shost);
			break;
	
		/*
		 * signal the process group associated with this connection.
		 * signal number in p_data[0]
		 */
		case ph_sig:
			if(!c) break;
			signal(&mxmin[c->c_min],b.p.p_data[0]);
			break;

		/*
		 * end of file.  wakeup read if blocked.
		 */
		case ph_eof:
			if(!c) break;
			c->c_flag =| c_eofr;
			if(c->c_flag & c_wread) {
				c->c_flag =& ~c_wread;
				wakeup(&c->c_ib);
			}
			break;
		}
		goto out;

	} else {                /* host data */
		if(!c) goto out;
		if(c->c_ib) {
			mx.e_ibnz++;
			goto out;/* impossible */
		}
		c->c_ib = bn;
		if(c->c_flag & c_wread) {
			c->c_flag =& ~c_wread;
			wakeup(&c->c_ib);
		}
		return;
	}
out:
	mxputfb(bn);
}

/*
 * reset all connections with host number "host".  if "host" = 0, destroy
 * all connections.
 */
mxhostr(host)
{
	register struct con *c;
	for(c = &mxcon[0] ; c < &mxcon[NCON] ; c++) {
		if(!c->c_flag) continue;
		if(host && host != c->c_fhost) continue;
		c->c_flag =| (c_diss|c_disr|c_abort);
		if(!mxretcon(c)) {
			c->c_fsoc = 0;          /* dont match incoming packs*/
			wakeup(c);
			wakeup(&c->c_ob);  wakeup(&c->c_ib);
		}
		mx.e_rst++;
	}
}

/*
 * find and return a pointer to the con entry that matches the arguments
 */
mxfindc(fhost,fsoc,lsoc)
{
	register struct con *c;
	register ls,fs;
	ls = lsoc;  fs = fsoc;
	for(c = &mxcon[0] ; c < &mxcon[NCON] ; c++) {
		if(c->c_flag == 0) continue;
		if(ls != c->c_lsoc || fs != c->c_fsoc ||
		   fhost != c->c_fhost) continue;
		return(c);
	}
	return(0);      /* no match */
}

/*
 * make a new entry in the con table; returns 0 if no slots left.
 */
mxnewc(fhost,fsoc,lsoc)
{
	register struct con *c;
	for(c = &mxcon[0] ; c < &mxcon[NCON] ; c++) {
		if(c->c_flag != 0) continue;
		c->c_fhost = fhost;
		c->c_fsoc = fsoc;
		c->c_lhost = LHOST;
		c->c_lsoc = lsoc;
		c->c_flag = c_inuse;
		return(c);
	}
	mx.e_outcon++;
	return(0);
}

/*
 * connection status.  only "open" if connects sent and received and no
 * disconnects sent or received.
 */
mxcstat(c)
register struct con *c;
{
	register i;
	i = c->c_flag & c_state;
	if(i & (c_disr|c_diss)) return(CCLOSED);
	if(i == (c_conr|c_cons)) return(COPEN);
	return(CWAIT);
}

/*
 * send control command "op" on connection "c"
 */
mxsendc(c,op)
{
	pbn bn;
	bn = mxnewfb(c);
	b.p.p_hostc = op;
	mxsend(bn);
}

/*
 * return a new free buffer with the header setup for sending on connection
 * "c".  sleep on the free list if no buffers temporarily available.
 * buffers may be released from a higher interrupt level, so spl7() before
 * deciding to sleep.
 */
mxnewfb(c)
register struct con *c;
{
	register pbn bn;
	register sps;
	sps = PS->integ;
	spl7();
	while(!(bn = mxgetfb())) sleep(&mx.free,PRIB);
	PS->integ = sps;
	b.p.p_shost = c->c_lhost;
	b.p.p_ssoc = c->c_lsoc;
	b.p.p_dhost = c->c_fhost;
	b.p.p_dsoc = c->c_fsoc;
	b.p.p_count = 0;
	b.p.p_hostc = b.p.p_impc = 0;
	return(bn);
}

/*
 * copy bytes into the kernel
 */
bcopyin(src,dst,cnt)
register char *src,*dst;
register cnt;
{
	if(cnt == 0) return;
	if((src|dst|cnt) & 01)
		do{ *dst++ = fubyte(src++);}while(--cnt);
	else
		copyin(src,dst,cnt);
}

/*
 * copy bytes out of the kernel
 */
bcopyout(src,dst,cnt)
register char *src,*dst;
register cnt;
{
	if(cnt == 0) return;
	if((src|dst|cnt) & 01)
		do{ subyte(dst++,*src++);}while(--cnt);
	else
		copyout(src,dst,cnt);
}

/*
 * send buffer "bn" by enqueuing on the mx imp's task queue
 */
mxsend(bn)
pbn bn;
{
	miqtask(bn);
	b.l_flag = l_local;
}

/*
 * get a buffer from the free buffer queue
 */
mxgetfb()
{
	register sps,i;
	sps = PS->integ;
	spl7();
	if((i = mxgetb(&mx.free)) && --mx.nfree < mx.lfree)
		mx.lfree=mx.nfree;
	PS->integ = sps;
	return(i);
}

/*
 * return buffer to the free buffer queue.  if the queue used to be empty,
 * wakeup users who may have been waiting.
 */
mxputfb(bn)
pbn bn;
{
	register sps,ofree;
	sps = PS->integ;
	spl7();
	ofree = mx.free;
	mx.nfree++;
	mxputb(bn, &mx.free);
	PS->integ = sps;
	if(!ofree)
		wakeup(&mx.free);
}

/*
 * printf onto the error log
 */
mxprf(x1,x2,x3,x4,x5)
{
	extern errlg;
	errlg++;
	printf(x1,x2,x3,x4,x5);
	errlg = 0;
}

/*
 * get a buffer from the head of a circular queue.  "qh" is the address of
 * the queue head word, which points to the tail of the queue.  the link
 * word (l_link) of each buffer points to the next, with the link word of
 * the tail pointing back to the head.
 *
 * all this is complicated somewhat since the buffer addresses are
 * physical memory block numbers (the 22 bit address shifted right 6 bits).
 */
mxgetb(qh)
register pbn *qh;
{
	register pbn hbn,rbn;
	if(*qh == 0) return(0);         /* queue empty */
	*ka5 = *qh;
	rbn = b.l_next;                 /* return old head */
	*ka5 = rbn;
	hbn = b.l_next;                 /* new head */
	*ka5 = *qh;
	if(hbn == rbn)                  /* if this was last, now empty */
		*qh = 0;
	else
		b.l_next = hbn;
	*ka5 = rbn;                     /* leave addressable for caller */
	return(rbn);
}

/*
 * put a buffer on the tail of a circular queue
 */
mxputb(bn,qh)
register pbn bn,*qh;
{
	register pbn hbn;
	if(*qh == 0) {                  /* if was empty */
		*ka5 = bn;
		b.l_next = bn;          /* point to itself */
	} else {
		*ka5 = *qh;
		hbn = b.l_next;
		b.l_next = bn;
		*ka5 = bn;
		b.l_next = hbn;
	}
	*qh = bn;
}

/*
 * reset the mx device.  link up the buffer pool;  start up the mx imp;
 * startup mxrall() in 2 seconds to send host-host resets to all foreign
 * hosts.
 */
mxreset()
{
	static init;
	extern ntaddr,ntsiz,mxrall();
	int left,size,bn;

	if(!init) {
		init++;
		left = ntsiz;
		size = (sizeof (struct linebuf) + 63)/64;
		bn = ntaddr;
		while((left =- size) >= 0) {
			if((bn&0176000) == ((bn+size-1)&0176000))
				mxputfb(bn);
			bn =+ size;
		}
		mx.lfree = mx.nfree;
	}
	mistart();
	spl1();
	mx.rall++;
	timeout(mxrall,0,120);  /* wait for lines to come up */
	sleep(&mx,PRIC);
}

/*
 * reset all.  clear local connections and send host resets to foreign hosts.
 *
 * See comments above about sleeps at interrupt level at mxrecv().
 * This code should be fixed soon, (before adding more hosts than there
 * are buffers), so that the buffer pool is not flooded.
 */
mxrall()
{
	static char con[4];
	struct con *c;
	int host;
	int oka5;
	oka5 = *ka5;
	mxhostr(0);
	c = &con;               /* fake connection for mxsendc */
	c->c_lhost = LHOST;
	for(host = 1 ; host <= NHOST ; host++) {
		c->c_fhost = host;
		mxsendc(c,ph_rst);
	}
	mx.rall = 0;
	wakeup(&mx);
	*ka5 = oka5;
}
