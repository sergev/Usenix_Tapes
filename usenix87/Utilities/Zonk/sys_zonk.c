/*
 *  Zonk system call - apply a signal to every process owned by a user.
 *  Typically used as zonk(uid,SIGKILL) to kill student runaway jobs.
 *  A count of the affected processes is returned.
 *  If passed signal zero, no signal is sent; only the count is returned.
 */

zonk()
{
	struct a {
		int	uid;
		int	sig;
	} *uap = (struct a *)u.u_ap;
	register int sig = uap->sig;
	register int count, uid = uap->uid;
	register struct proc *p;

	if (uid != u.u_uid && !suser())
		return;
	if (uid == 0 && sig || (unsigned)sig >= NSIG) {
		u.u_error = EINVAL;
		return;
	}
	for (count = 0, p = proc; p < procNPROC; p++)
		if (p->p_uid == uid) {
			count++;
			if (sig)
				psignal(p, sig);
		}
	u.u_r.r_val1 = count;
}
