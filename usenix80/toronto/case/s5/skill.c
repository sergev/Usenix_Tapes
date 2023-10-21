#

/*
 *      skill - signal a process and all its children
 *                             written by Bob Greenberg
 */

#include "/usr/sys/param.h"
#include "/usr/sys/proc.h"

int *p__t, *p__te, p__sig;

skill(pid, sig)
{
	register int i, *n;
	struct proc procs[2*NPROC];

	if (gprocs(0) >= 2*NPROC) return(-1);
	i = gprocs(procs);
	p__t = procs;
	p__te = &procs[i];
	p__sig = sig;
	return(sk_2(pid));
}
/* NON-RBGUNIX version:
	register int i, *n, mem;
	struct proc procs[NPROC];
	struct { char name[8]; int  type; char  *value; } nl[2];

	if ((mem = open("/dev/mem",0)) < 0) return(-1);
	n = nl;
	*n++ = '_p'; *n++ = 'ro'; *n++ = 'c';
	for (i = 10; --i;) *n++ = 0;
	nlist("/unix",nl);
	if (nl[0].type == 0) return(-1);
	seek(mem, nl[0].value, 0);
	if (read(mem, procs, sizeof procs) < sizeof procs) return(-1);
	close(mem);
	p__t = procs;
*/

sk_2(pidd)
{
	register struct proc *p;
	register int n, pid;
	int n2;

	n = 0;
	pid = pidd;
	n2 = 1;
	for (p = p__t; p <= p__te; p++)
	{
		if (p->p_stat == 0) continue;
		if (p->p_ppid == pid) n =| sk_2(p->p_pid);
		if (p->p_pid == pid) n2 = 0;
	}
	if (n2 == 0 && kill(pid,p__sig) < 0) n2 = 1;
	return(n2 | n);
}
