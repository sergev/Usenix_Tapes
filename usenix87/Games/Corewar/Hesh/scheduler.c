/*
** scheduler.c	- routines dealing with the running of the jobs
**
**	[cw by Peter Costantinidis, Jr. @ University of California at Davis]
**
static	char	rcsid[] = "$Header: scheduler.c,v 7.0 85/12/28 14:36:26 ccohesh Dist $";
**
*/

#include <stdio.h>
#include <setjmp.h>
#include "cw.h"


static	int	_pc;			/* current pc */
static	jmp_buf	haltit;

extern	void	halt(), run();
extern	memword	*getoperand();

/*
** scheduler()	- run the processes
**		- very simple algorithm to determine what job is scheduled next:
**			jobs are scheduled by thier order in the process table
**		- each job gets to execute one instruction during its scheduled
**		  period
*/
int	scheduler (pcs)
reg	process	*pcs;
{
	reg	int	flag;

	if (ON(VISUAL))
		return(vscheduler(pcs));
	for (cycle=0; jobsleft > 1 && cycle < max_cyc; cycle++)
	{
		reg	process	*j;

		if (ON(PASSNUM) && (!(cycle % PASSFREQ) || ON(PINST)))
			printf("Pass #%04d\t", cycle);
		if (!ON(PINST) && ON(PASSNUM) &&
		    (!(cycle % PASSFREQ) || ON(PINST)))
			putchar('\r'), fflush(stdout);
		for (flag=0, j=pcs; j < (pcs+jobcnt); j++, flag++)
		{
			if (ON(PINST) && ON(PASSNUM) && flag &&
			    (!(cycle % PASSFREQ) || ON(PINST)))
				printf("\t\t");
			if (!HALTED(j))
				run(j);
		}
	}
	if (cycle >= max_cyc)
		return(ECYCLE);
	return(EWIN);
}

/*
** vscheduler()	- the scheduler used for visual display mode
*/
int	vscheduler (pcs)
reg	process	*pcs;
{
	for (cycle=0; jobsleft > 1 && cycle < max_cyc; cycle++)
	{
		reg	process	*j;

		for (j=pcs; j < (pcs+jobcnt); j++)
			if (!HALTED(j))
				run(j);
		vupdate(pcs);
		if (ON(STATUS) && !(cycle % PASSFREQ) && ON(PASSNUM))
			vstatus();
	}
	if (cycle >= max_cyc)
		return(ECYCLE);
	return(EWIN);
}

/*
** run()	- execute one instruction of the given job
*/
static	void	run (j)
reg	process	*j;
{
	reg	memword	*w;
	reg	int	status;

	/* fetch pc (incremented at end of function or where appropriate) */
	PC = j->pc;
	/* fetch instruction */
	w = &(memory[j->pc]);
	/* remember this for halting processes */
	if (status = SETJMP(haltit))
	{
		halt(j, status);
		return;
	}
	if (ON(PINST))
	{
		printf("PC: %4d\t", PC);
		if (printit(stdout, w))
			fprintf(stderr, "%s: printit() error\n", argv0);
	}
	switch (w->op)
	{
		auto	int	tmp;

#define	A	w->moda, w->arga
#define	B	w->modb, w->argb
/*
** when modifying memory locations through arithmetic operations
** make sure the opcode gets reset
*/
#define	COREA	tmp=getaddr(w->moda, w->arga),memory[tmp].op=DAT,memory[tmp].val
#define	COREB	tmp=getaddr(w->modb, w->argb),memory[tmp].op=DAT,memory[tmp].val

		when MOV:/* move A into location B */
			movmem(j->plstmem=(&(memory[getaddr(B)])),
				getoperand(A));
		when ADD:/* add A to contents of B, results in B */
			COREB = sum((int) (getoperand(A))->val,(int) (getoperand(B))->val);
		when MUL:
		{
			reg	int	temp = (int) (getoperand(B))->val;

			temp *= (int) (getoperand(A))->val;
			COREB = sum(temp, 0);
		}
		when DIV:
		{
			reg	int	temp = (int) (getoperand(B))->val;

			temp /= (int) (getoperand(A))->val;
			COREB = sum(temp, 0);
		}
		when SUB:/* subtract A from contents of B, results in B */
			COREB = sum((int) -(getoperand(A))->val,(int) (getoperand(B))->val);
		when JMP:/* jump to B */
			j->pc = getaddr(B);
			return;
		when JMZ:/* if A == 0 then jump to B, otherwise continue */
			if (getoperand(A)->val == 0)
			{
				j->pc = getaddr(B);
				return;
			}
		when DJZ:/* --A; if A == 0 then jump to B */
			COREA = sum((int) getoperand(A)->val, -1);
			if (COREA == 0)
			{
				j->pc = getaddr(B);
				return;
			}
		when CMP:/* compare A with B; if != then skip next inst. */
			if (getoperand(A)->val != getoperand(B)->val)
			{
				j->pc = sum(j->pc, 2);
				return;
			}
		when RND:/* into A a random number between 0 and MAX_MEM */
			COREB = sum((int) rand(), 0);
		otherwise:
			halt(j, EILLINST);
#undef	A
#undef	B
#undef	COREA
#undef	COREB
	}
	if (!HALTED(j))
		j->pc = sum(j->pc, 1);		/* increment pc */
}

/*
** getoperand()	- return the operand
**		- since a pointer to static data is returned, care is
**		  taken to insure that this static data isn't corrupted
**		  due to two `simultaneous' calls.
*/
static	memword	*getoperand (m, a)
reg	uns	m, a;
{
	static	memword	mw1, mw2, *mw;

	mw = (mw == &mw1) ? &mw2 : &mw1;
	switch (m)
	{
		case IMM:
			mw->op = 0;
			mw->val = a;
			return(mw);
		case REL:
			return(&(memory[sum(PC, (int) a)]));
		case IND:
			return(&(memory[getaddr(m,a)]));
		default:
			LONGJMP(haltit, EBADMODE);
	}
	/*NOTREACHED*/
}

/*
** getaddr()	- return the address
*/
static	int	getaddr (m, a)
reg	uns	m, a;
{
	switch (m)
	{
		case REL:
			return(sum((int) PC, (int) a));
		case IND:
		{
			reg	int	tmp;

			tmp = sum(PC, (int) a);
			return(sum(tmp, (int) memory[tmp].val));
		}
		case IMM:
		default:
			LONGJMP(haltit, EBADMODE);
	}
	/*NOTREACHED*/
}

/*
** halt()	- halt the specified process
*/
static	void	halt (j, reason)
reg	process	*j;
reg	int	reason;
{
#ifdef	DEBUG
	fprintf(stderr, "%s: process \"%s\" halted\n", argv0, j->pname);
#endif
	j->pdied = reason;
	j->picnt = cycle;
	jobsleft--;
	if (ON(VISUAL) && ON(STATUS))
		vstajob(j);
}
