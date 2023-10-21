#
#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../proc.h"
#include "../buf.h"
#include "../inode.h"
#include "../inst.h"

#define	UMODE	0170000
#define	SCHMAG	8/10

/*
 * clock is called straight from
 * the real time clock interrupt.
 *
 * Functions:
 *	reprime clock
 *	copy *switches to display
 *	implement callouts
 *	maintain user/system times
 *	maintain date
 *	Maintain inode last reference time.
 *	profile
 *	tout wakeup (sys sleep)
 *	lightning bolt wakeup (every 4 sec)
 *	alarm clock signals
 *	jab the scheduler
 */

/* MONITOR controls conditional compilation which causes the clock
 * routine to keep a few statistics about system performance.
 * See param.h
 *	Mike Tilson, Nov. 1975
 *	Bill Shannon, 06/01/78
 */

extern waitloc();
struct buf swbuf;


clock(dev, sp, r1, nps, r0, pc, ps)
int (*pc)();
{
	register struct callo *p1, *p2;
	register struct proc *pp;
	struct inode *ip;
	int a;
#ifdef	MONITOR
	int index;	/* index for bumping swpstat,cpustat (see inst.h)*/
	int i;		/* temporary cpu state 1/0 = busy/idle */
#endif

	/*
	 * restart clock
	 */

	*lks = 0115;

	/*
	 * display register
	 */

	display();

/*

   System Instrumentation - maintains comprehensive statistics on demand.
   Duration of trace can cumulatively amount to 414 days (2147483654 ticks)
   Utilities available to start,initialize,stop & analyze traced data.

   	- Sujit R. Kumar, Jan 1979, CWRU.

*/

#ifdef	MONITOR
	if (I.inst_go) {
		index = i = 1;	/* Set cpu bit position (busy)*/
		I.c_ticks++;
		if (runrun)	/* cpu sched has higher priority process */
			I.c_rnrun++;
		if (runout)	/* No ready process waiting to be swapped in */
			I.swp_rnout++;
		else
			if (runin)	/* Proc. waiting for core */
				I.swp_rnin++;
		if ((ps&UMODE) == UMODE)	/* Cpu was executing in user-mode */
			I.c_user++;
		if (pc==waitloc) {
			I.c_idle++;
			index = i = 0;	/* Clear cpu bit (idle) */
		}
		if ((ps&0340)!=0)  /* Previous priority>0 */
				I.c_sysint++;
		if (swbuf.b_flags&B_BUSY) {
			I.swp_stat[index]++;
			if (swbuf.b_flags&B_WANTED)
				I.swp_stat[(index+2)]++;
		}
		if (I.inst_conf&RK) {	/* RKs configured in the system */
			if (I.rk_busy)
				index =| RK;	/* Set RK bit position (busy)*/
			I.rk_stat[(i|(I.rk_busy<<1))]++; /* Bump rk-cpu stats */
		}

		I.c_stat[index]++;	/* Bump cpu-devices overlap stats. */
	}
#endif

#ifdef	PROF
	/*
	 * call system profiler
	 * to do PC trace
	 */

	sprof(pc, ps);
#endif

	/*
	 * callouts
	 * if none, just return
	 * else update first non-zero time
	 */

	if(callout[0].c_func == 0)
		goto out;
	p2 = &callout[0];
	while(p2->c_time<=0 && p2->c_func!=0)
		p2++;
	p2->c_time--;

	/*
	 * if ps is high, just return
	 */

	if((ps&0340) != 0)
		goto out;

	/*
	 * callout
	 */

	spl5();
	if(callout[0].c_time <= 0) {
		p1 = &callout[0];
		while(p1->c_func != 0 && p1->c_time <= 0) {
			(*p1->c_func)(p1->c_arg);
			p1++;
		}
		p2 = &callout[0];
		while(p2->c_func = p1->c_func) {
			p2->c_time = p1->c_time;
			p2->c_arg = p1->c_arg;
			p1++;
			p2++;
		}
	}

	/*
	 * lightning bolt time-out
	 * and time of day
	 */

out:
	if((ps&UMODE) == UMODE) {
		u.u_utime++;
		if(u.u_prof[3])
			incupc(pc, u.u_prof);
	} else
	{
		u.u_stime++;
	}
	pp = u.u_procp;
	if(++pp->p_cpu == 0)
		pp->p_cpu--;
	if(++lbolt >= HZ) {
		if((ps&0340) != 0)
			return;
		lbolt =- HZ;
		/*
		 * On clock overflow, update high order time
		 * and zap last inode reference times.
		 */
		if(++time[1]==0){
			for(ip=inode;ip!= &inode[NINODE];ip++)
				ip->i_lrt=0;
			++time[0];
		}
		spl1();
		if(time[1]==tout[1] && time[0]==tout[0])
			wakeup(tout);
		if((time[1]&03) == 0) {
			runrun++;
			wakeup(&lbolt);
		}
		for(pp = &proc[0]; pp < &proc[NPROC]; pp++)
		if (pp->p_stat && pp->p_stat < SZOMB) {
			if(pp->p_time != 127)
				pp->p_time++;
			if (pp->p_clktim)
				if (--pp->p_clktim == 0)
					psignal(pp, SIGCLK);
			a = (pp->p_cpu & 0377)*SCHMAG + pp->p_nice;
			if (a < 0);
				a = 0;
			if (a > 255)
				a = 255;
			pp->p_cpu = a;
			if(pp->p_pri > PUSER)
				setpri(pp);
		}
		if(runin!=0) {
			runin = 0;
			wakeup(&runin);
		}
		if((ps&UMODE) == UMODE) {
			u.u_ar0 = &r0;
			if(issig())
				psig();
			setpri(u.u_procp);
		}
	}
}

/*
 * timeout is called to arrange that
 * fun(arg) is called in tim/HZ seconds.
 * An entry is sorted into the callout
 * structure. The time in each structure
 * entry is the number of HZ's more
 * than the previous entry.
 * In this way, decrementing the
 * first entry has the effect of
 * updating all entries.
 *
 * The panic is there because there is nothing
 * intelligent to be done if an entry won't fit.
 */
timeout(fun, arg, tim)
{
	register struct callo *p1, *p2;
	register t;
	int s;

	t = tim;
	s = PS->integ;
	p1 = &callout[0];
	spl7();
	while(p1->c_func != 0 && p1->c_time <= t) {
		t =- p1->c_time;
		p1++;
	}
	if (p1 >= &callout[NCALL-1])
		panic("Timeout table overflow");
	p1->c_time =- t;
	p2 = p1;
	while(p2->c_func != 0)
		p2++;
	while(p2 >= p1) {
		(p2+1)->c_time = p2->c_time;
		(p2+1)->c_func = p2->c_func;
		(p2+1)->c_arg = p2->c_arg;
		p2--;
	}
	p1->c_time = t;
	p1->c_func = fun;
	p1->c_arg = arg;
	PS->integ = s;
}
