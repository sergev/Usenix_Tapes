/**			signals.c		**/

/** This set of routines traps various signals and informs the
    user of the error, leaving the program in a nice, graceful
    manner.

	(C) Copyright 1986 Dave Taylor
**/

#include "headers.h"
#include <signal.h>

extern int pipe_abort;		/* set to TRUE if receive SIGPIPE */

quit_signal()
{
	dprint0(2,"\n\n** Received SIGQUIT **\n\n\n\n");
	leave();
}

term_signal() 
{
	dprint0(2,"\n\n** Received SIGTERM **\n\n\n\n");
	leave();
}

ill_signal()
{
	dprint0(1,"\n\n** Received SIGILL **\n\n\n\n");
	PutLine0(LINES, 0, "\n\nIllegal Instruction signal!\n\n");
	emergency_exit();
}

fpe_signal()  
{
	dprint0(1,"\n\n** Received SIGFPE **\n\n\n\n");
	PutLine0(LINES, 0,"\n\nFloating Point Exception signal!\n\n");
	emergency_exit();
}

bus_signal()
{
	dprint0(1,"\n\n** Received SIGBUS **\n\n\n\n");
	PutLine0(LINES, 0,"\n\nBus Error signal!\n\n");
	emergency_exit();
}

segv_signal()
{
	dprint0(1,"\n\n** Received SIGSEGV **\n\n\n\n");
	PutLine0(LINES, 0,"\n\nSegment Violation signal!\n\n");
	emergency_exit();
}

alarm_signal()
{	
	/** silently process alarm signal for timeouts... **/

	int alarm_signal();

	signal(SIGALRM, alarm_signal);
}

pipe_signal()
{
	/** silently process pipe signal... **/

	int pipe_signal();

	dprint0(2,"*** received SIGPIPE ***\n\n");
	
	pipe_abort = TRUE;	/* internal signal ... wheeee!  */

	signal(SIGPIPE, pipe_signal);
}
