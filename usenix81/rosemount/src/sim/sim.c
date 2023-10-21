#include "sim.h"

/*
 * Discrete event simulation package
 */




/*
 * Simulation data
 */

struct proc {
	char *next ;
	char *procblk ;
	int signal ;
	double runtime ;
	} mainproc ;

double simtime ;
struct proc *pproc ;
struct proc *curproc = { &mainproc } ;
struct proc *nproc ;

extern struct proc *nextproc() ;




/*
 * siminit - initialize system for simulation
 */

siminit()
{
	simtime = 0.0 ;

	pproc = nproc = 0 ;
	curproc = &mainproc ;
	curproc->next = 0 ;
	curproc->procblk = 0 ;
	curproc->signal = 0 ;
	curproc->runtime = 0.0 ;

	} /* siminit */





/*
 * sfork - fork into two processes.  Returns twice, returns with 0 for
 *		   the child process, returns with process pointer for parent.
 */

struct proc *sfork()
{
	register struct proc *tproc ;

	if ((tproc = malloc(sizeof(*tproc))) <= 0) goto oom ;

	tproc->next = nproc ;
	tproc->signal = 0 ;
	tproc->runtime = simtime ;

	nproc = tproc ;
	curproc->next = tproc ;
	if ((tproc->procblk = save()) == 0) return (0) ;
	else if (tproc->procblk != -1) return (tproc) ;

	oom :
		printf("simulation: Out of memory.	simtime= %f\n",simtime) ;
		abort() ;

	} /* sfork */





/*
 * sexit - terminate a process
 */

sexit()
{
	if (curproc == &mainproc) {
		printf("simulation/sexit:  Can't 'sexit' main process.	simtime= %f\n",
			simtime) ;
		return ;
		}

	if (pproc) pproc->next = nproc ;

	free (curproc) ;
	curproc = pproc ;

	restore (nextproc()->procblk) ;

	} /* sexit */




/*
 * delay - wait for simulation time to increase by amount.
 */

delay(time)
int time ;
{
	double x ;

	x = time ;
	delayf(x) ;

	} /* delay */



delayf(time)
double time ;
{
	if (time < 0.0) {
		printf("simulation/delay:  Can't go back in time.	simtime= %f\n",
			simtime) ;
		}

	curproc->runtime = simtime + time ;

	switchproc() ;

	} /* delay */





/*
 * ssignal - start all processes waiting for signal 'n'.
 */

ssignal(n)
int n ;
{
	register struct proc *tproc ;

	for (tproc = &mainproc; tproc; tproc = tproc->next) {
		if (tproc->runtime < 0.0 &&
			tproc->signal == n) tproc->runtime = simtime ;
		}

	} /* ssignal */





/*
 * swait - wait for signal 'n'.
 */

swait(n)
int n ;
{
	curproc->signal = n ;
	curproc->runtime = -1.0 ;

	switchproc() ;
	} /* swait */





/*
 * swaited - returns 0 if no process is waiting in signal 'n', 1 otherwise.
 */

swaited(n)
int n ;
{
	register struct proc *tproc ;

	for (tproc = &mainproc; tproc; tproc = tproc->next ) {
		if (tproc->runtime < 0.0 &&
			tproc->signal == n) return (1) ;
		}

	return (0) ;

	} /* swaited */





/*
 * snumber - returns number of processes waiting on signal 'n'
 */

snumber(n)
int n ;
{
	register int i ;
	register struct proc *tproc ;

	i = 0 ;
	for (tproc = &mainproc; tproc; tproc = tproc->next) {
		if (tproc->runtime < 0.0 &&
			tproc->signal == n) i++ ;
		}

	return (i) ;

	}






/*
 * qinit - initialize wait queue.
 */

qinit(s)
struct squeue *s ;
{
	s->count = 0 ;
	s->first = s->last = 0 ;

	} /* qinit */





/*
 * qsignal - start next process in squeue queue.
 */

qsignal(s)
struct squeue *s ;
{
	register struct proc *tproc ;

	if (s->count < 0 || s->first == 0) s->count++ ;
	else {
		tproc = s->first ;
		tproc->runtime = simtime ;
		if ( (s->first = s->first->next) == 0) s->last = 0 ;
		curproc->next = tproc ;
		tproc->next = nproc ;
		nproc = tproc ;
		}

	} /* qsignal */





/*
 * qwait - wait on squeue queue.
 */

qwait(s)
struct squeue *s ;
{
	register struct proc *tproc ;

	if (s->count > 0) s->count-- ;
	else {
		tproc = curproc ;
		if (curproc = nproc) nproc = nproc->next ;
		else curproc = &mainproc ;

		tproc->next = 0 ;
		if (s->last == 0) s->first = s->last = tproc ;
		else s->last = s->last->next = tproc ;

		switchproc() ;
		}

	} /* qwait */





/*
 * qwaited - returns 1 if there is a process waiting on queue, 0 otherwise.
 */

qwaited(s)
struct squeue *s ;
{
	return ( (s->first != 0) ? 1 : 0) ;

	} /* qwaited */




/*
 * switchproc - search for lowest runtime process and start it.
 */

switchproc()
{
	if ((curproc->procblk = save()) == 0) return ;
	else if (curproc->procblk == -1) {
		printf("simulation: Out of memory.	simtime= %f\n",
			simtime) ;
		abort() ;
		}
	else {
		restore (nextproc()->procblk) ;
		}

	} /* switchproc */





/*
 * nextproc - scan process table and return pointer to next process to run.
 */

struct proc *nextproc()
{
	double mintime ;
	struct proc *p, *c, *x ;

	c = 0 ;
	x = curproc ;

	mintime = 1e30 ;

	do {
		pproc = curproc ;
		curproc = curproc->next ;
		if (curproc == 0) curproc = &mainproc ;
		if (curproc->runtime >= simtime &&
			curproc->runtime < mintime ) {
			mintime = curproc->runtime ;
			p = pproc ;
			c = curproc ;
			if (mintime == simtime) break ;
			}
		} while (curproc != x) ;

	if (c == 0) {
		printf("simulation:  Deadlock condition.	simtime= %f\n",simtime) ;
		exit() ;
		}

	simtime = mintime ;
	curproc = c ;
	pproc = p ;
	nproc = curproc->next ;

	return (curproc) ;

	} /* nextproc */
