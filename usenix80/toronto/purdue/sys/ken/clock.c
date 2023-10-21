#
#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../proc.h"
#include "../inode.h"
#ifdef STATS
#include "../stats.h"
#include "../buf.h"
#endif

#define	UMODE	0170000
int	schmag	10;
int	schmin	1;	/* min value for schmag (when schproc procs runable) */
int	schmax	10;	/* max value for schmag (when 0 procs runable) */
int	schproc	20;	/* schmag goes to schmin as runable procs goes to scproc */
int	sclschd	25*HZ;	/* number of CPU ticks which to set LSCHD */
int	runable;	/* current number of runnable proc's */
int	slice	20;	/* experimental */

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
 *	profile
 *	tout wakeup (sys sleep)
 *	lightning bolt wakeup (every 4 sec)
 *	alarm clock signals
 *	jab the scheduler
 */

#ifdef STATS
	extern waitloc();	/* CPU idle location */
	struct buf swbuf;
#endif
clock(dev, sp, r1, nps, r0, pc, ps)
#ifdef STATS
	int (*pc)();
#endif
{
	register struct callo *p1, *p2;
	register struct proc *pp;
	static int cycle;
	struct inode *ip;
	char *totime;
# ifdef XBUF
	int savka5;
# endif
#ifdef STATS
	int i;
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
#ifdef STATS
	if (RP04ADDR->integ & 01)
		stats.mon_cur[RP04_CNT]++;	/* RP04 transfer active */
	if ((swbuf.b_flags&B_BUSY) == B_BUSY)
		stats.mon_cur[SWAP_CNT]++;
	stats.ken_pc = -1;
#endif
# ifdef XBUF
	savka5 = *ka5;
	*ka5 = praddr;
# endif
	pp = u.u_procp;
	if((ps&UMODE) == UMODE) {
#ifdef STATS
		stats.mon_cur[USER_CNT]++;
#endif
		u.u_utime++;
		if(u.u_prof[3])
			incupc(pc, u.u_prof);
	} else {
		u.u_stime++;
#ifdef STATS
		if (pc == waitloc) 
			stats.mon_cur[IDLE_CNT]++;
		else {
			if (ps&0340)
				stats.mon_cur[SYSI_CNT]++;
			else
				stats.mon_cur[SYS_CNT]++;
			stats.ken_pc = pc;
		}
#endif
	}
	totime = u.u_utime + u.u_stime;
	if (totime >= sclschd && pp->p_nice > -5)
		pp->p_flag =| LSCHD;  /* set unfavorable scheduling */
	if (u.u_timlim && totime == u.u_timlim)
		psignal(pp, SIGTIM);
	if (cycle%slice == 0)
		if(++pp->p_cpu == 0)
			pp->p_cpu--;
	if(++lbolt >= HZ) {
		if((ps&0340) != 0) {
# ifdef XBUF
			*ka5 = savka5;
# endif
			return;
		}
#ifdef STATS
		for(i=0; i<NMON; i++) {
			stats.mon_cnt[i] = (stats.mon_cnt[i]+stats.mon_cur[i]) >>1;
			stats.mon_cur[i] = 0;
		}
#endif
		lbolt =- HZ;
	/*
	 * On clock overflow, update high order time
	 * and zap last inode reference times
	 * -ghg 10/05/77 (from Toronto)
	 */
		if(++time[1]==0) {
			for(ip=inode; ip != &inode[NINODE]; ip++)
				ip->i_lrt = 0;
			++time[0];
		}

		spl1();
		if(time[1]==tout[1] && time[0]==tout[0])
			wakeup(tout);
		if((time[1]&03) == 0) {
			runrun++;
			wakeup(&lbolt);
		}
		runable = 0;
		for(pp = &proc[0]; pp < &proc[NPROC]; pp++)
		if (pp->p_stat) {
			if(pp->p_time != 127)
				pp->p_time++;
			if((pp->p_cpu & 0377) > schmag)
				pp->p_cpu =- schmag; else
				pp->p_cpu = 0;
			if (pp->p_nice < -5)
				pp->p_flag =& ~LSCHD;
			if(pp->p_clktim)
				if(--pp->p_clktim == 0)
					psignal(pp, SIGCLK);
			if(pp->p_pri >= PUSER) {
				setpri(pp);
				runable++;
			}
		}
		if (runable > schproc)
			runable = schproc;
		schmag = schmax - ((schmax-schmin)*runable)/schproc;
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
# ifdef XBUF
	*ka5 = savka5;
# endif
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
