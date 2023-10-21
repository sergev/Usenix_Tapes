#

/*
 *      mx imp (packet switch)
 */

#define INIT    INIT            /* cause mxi.h to initialize structs */

#include "mx.h"
#include "mxi.h"


struct mi {
	pbn     taskq;          /* packets waiting for mitask */
	char    active;         /* mitask active */
	char    init;           /* first init performed */
} mi;


/*
 * Called indirectly by miqtask(packetaddress) via a software interrupt at
 * level 1.  miqtask is called when the local host sends a packet, and
 * when packets are received from external device lines.  By looking at
 * the destination host address, mitask determines whether
 * to forward the packet out to another device, or pass it to the
 * local host.
 */
mitask()
{
	int oka5;
	pbn bn;
	register host;
	register struct ld *ld;
	register struct devdat *d;

	oka5 = *ka5;
	while(spl7(),(bn = mxgetb(&mi.taskq))) {
		spl1();
		if(b.l_flag&l_call) {   /* call crc task */
			(*b.l_task)(bn);
			continue;
		}
		if((host = b.p.p_dhost) <= 0 || host > NHOST ||
		   mihostline[host] == -2) {
			mirev(bn);      /* bad host number */
			continue;
		}
		if(mihostline[host] == -1 ||
			(host == LHOST && !(b.l_flag&l_local))) {
			mxrecv(bn);     /* this pack for our host */
			continue;
		}
		ld = &milinedev[mihostline[host]];
		(*ld->ld_spl)();        /* lockout this device */
		d = ld->ld_dat;
		if(!d->d_up) {          /* if this line is dead */
			mirev(bn);
			continue;
		}
		mxputb(bn,&d->d_oq);    /* place packet on dev out q */
		if(d->d_xstate)         /* if already active */
			continue;
		(*ld->ld_task)(ld->ld_min,TPACK);       /* start device */
	}
	*ka5 = oka5;
	PIR->integ = 0;                 /* clear soft int req */
	mi.active = 0;
}

/*
 * start or restart the mx imp.  call the device reset routines.
 */
mistart()
{
	register struct ld *ld;
	register pbn bn;
	extern miclock();
	spl7();
	while((bn = mxgetb(&mi.taskq))) /* flush old task queue */
		mxputfb(bn);
	for(ld = &milinedev[0] ; ld->ld_min >= 0 ; ld++) {
		/* (*ld->ld_spl)();     */
		ld->ld_dat = (*ld->ld_task)(ld->ld_min,TINIT,ld->ld_csr);
	}
	if(mi.init) return;
	mi.init++;
	timeout(miclock,0,TTIMERTICKS);
}

/*
 * call each device's timer task every TTIMERTICKS ticks
 */
miclock()
{
	register struct ld *ld;
	register oka5;
	oka5 = *ka5;
	for(ld = &milinedev[0] ; ld->ld_min >= 0 ; ld++) {
		(*ld->ld_spl)();
		(*ld->ld_task)(ld->ld_min,TTIMER);
	}
	timeout(miclock,0,TTIMERTICKS);
	*ka5 = oka5;
	spl5();                 /* really should be in clock.c */
}

/*
 * reverse this packet; send it back to the source host with the dead
 * host status bit set.  if this was already a dead packet, discard it.
 */
mirev(bn)
pbn bn;
{
	register i;
	*ka5 = bn;
	if(b.p.p_impc){    /* if already dead */
		mxputfb(bn);
		return;
	}
	b.p.p_impc = pi_dead;
	i = b.p.p_dhost;
	b.p.p_dhost = b.p.p_shost;
	b.p.p_shost = i;
	i = b.p.p_dsoc;
	b.p.p_dsoc = b.p.p_ssoc;
	b.p.p_ssoc = i;
	miqtask(bn);
}

/*
 * queue up a packet on mi.taskq, start up mitask if he isnt already.
 */
miqtask(bn)
{
	int sps;
	sps = PS->integ;
	spl7();
	mxputb(bn,&mi.taskq);
	b.l_flag = 0;
	if(!mi.active) {
		mi.active++;
		PIR->integ =| TASKREQ;
	}
	PS->integ = sps;
}
