#
/*
 */

#include "../header/param.h"
#include "../header/systm.h"
#include "../header/user.h"
#include "../header/proc.h"
#include "../header/inode.h"
#include "../header/reg.h"

/*
 * Priority for tracing
 */
#define	IPCPRI	(-1)

/*
 * Structure to access an array of integers.
 */
struct
{
	int	inta[];
};

/*
 * Tracing variables.
 * Used to pass trace command from
 * parent to child being traced.
 * This data base cannot be
 * shared and is locked
 * per user.
 */
struct
{
	int	ip_lock;
	int	ip_req;
	int	ip_addr;
	int	ip_data;
} ipc;

/*
 * Send the specified signal to
 * all processes with 'tp' as its
 * controlling teletype.
 * Called by tty.c for quits and
 * interrupts.
 */
signal(tp, sig)
{
	register struct proc *p;

	for(p = &proc[0]; p < &proc[NPROC]; p++)
		if(p->p_ttyp == tp)
			psignal(p, sig);
}

/*
 * Send the specified signal to
 * the specified process.
 */
psignal(p, sig)
int *p;
{
	register *rp;

	if(sig >= NSIG)
		return;
	rp = p;
			/* Kill above all, but don't loose hangups either! */
	if (sig == SIGKIL || (rp->p_sig != SIGKIL && rp->p_sig != SIGHUP))
		rp->p_sig = sig;
	if(rp->p_stat > PUSER)
		rp->p_stat = PUSER;
	if(rp->p_stat == SWAIT)
		setrun(rp);
}

/*
 * Returns true if the current
 * process has a signal to process.
 * This is asked at least once
 * each time a process enters the
 * system.
 * A signal does not do anything
 * directly to a process; it sets
 * a flag that asks the process to
 * do something to itself.
 */
issig()
{
	register n;
	register struct proc *p;

	p = u.u_procp;
	if(n = p->p_sig) {
		if((u.u_signal[n]&1) == 0)
			return(n);
	}
	return(0);
}


/*
 * Perform the action specified by
 * the current signal.
 * The usual sequence is:
 *	if(issig())
 *		psig();
 */
psig()
{
	register n, p;
	register *rp;

	rp = u.u_procp;
	n = rp->p_sig;
	rp->p_sig = 0;
	if((p=u.u_signal[n]) != 0) {
		u.u_error = 0;
		if(n != SIGINS)
			u.u_signal[n] = 0;
		n = u.u_ar0[R6] - 4;
		grow(n);
		suword(n+2, u.u_ar0[RPS]);
		suword(n, u.u_ar0[R7]);
		u.u_ar0[R6] = n;
		u.u_ar0[RPS] =& ~TBIT;
		u.u_ar0[R7] = p;
		return;
	}
	switch(n) {

	case SIGQIT:
	case SIGINS:
	case SIGIOT:
	case SIGEMT:
	case SIGFPT:
	case SIGBUS:
	case SIGSEG:
	case SIGSYS:
		u.u_arg[0] = n;
	}
	u.u_arg[0] = (u.u_ar0[R0]<<8) | n;
	exit();
}


/*
 * grow the stack to include the SP
 * true return if successful.
 */

grow(sp)
char *sp;
{
	register a, si, i;

	if(sp >= -u.u_ssize*64)
		return(0);
	si = ldiv(-sp, 64) - u.u_ssize + SINCR;
	if(si <= 0)
		return(0);
	if(estabur(u.u_tsize, u.u_dsize, u.u_ssize+si, u.u_sep))
		return(0);
	expand(u.u_procp->p_size+si);
	a = u.u_procp->p_addr + u.u_procp->p_size;
	for(i=u.u_ssize; i; i--) {
		a--;
		copyseg(a-si, a);
	}
	for(i=si; i; i--)
		clearseg(--a);
	u.u_ssize =+ si;
	return(1);
}
