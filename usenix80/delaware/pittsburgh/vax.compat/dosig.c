#
/*	Handle signal trapping from version 6 or version 7 compatability
 *	mode programs.
 *	Art Wetzel	November 1979
 */
#include <stdio.h>
#include <signal.h>
#include "defs.h"
unsigned int  sigvals[NSIG+1];
/* actual catch point for all signals */
sigcatch(signum) int signum; {
	unsigned short *pcptr;
	extern getregs();
	/* hurry up and get the registers before they are destroyed */
	getregs();
	/* figure out the pc */
	pcptr = (unsigned short *)((char *)&pcptr + 20);
	pc = (unsigned short *) *pcptr;
	/* get the psl with condition codes */
	/* requires UNIX-32V patch to not clear out condition codes */
	psl = 0x83c00000 | (*(pcptr - 6) & 017);
	/* actually do the thing */
	dosig(signum, pc);
	/* go back to compatability mode and the signal routine there */
	compat();
}
/* routine to set up pdp11 space for return from a signal */
dosig(signum, from) {
	unsigned short *sp;
	/* where is the stack */
	sp = (unsigned short *)regs[6];
	/* stack up psw condition codes so rti works */
	*(--sp) = psl & 017;
	/* stack pc */
	*(--sp) = (unsigned short)(int)pc;
	/* reset stack pointer */
	regs[6] = (unsigned short)(int)sp;
	/* reset pc to signal catching routine */
	pc = (unsigned short *)sigvals[signum];
}
