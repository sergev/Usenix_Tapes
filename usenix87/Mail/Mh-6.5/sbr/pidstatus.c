/* pidstatus.c - report child's status */

#include "../h/mh.h"
#include <signal.h>
#include <stdio.h>


#ifndef	BSD42
static char *sigs[] = {
    NULL,
    "Hangup",
    "Interrupt",
    "Quit",
    "Illegal instruction",
    "Trace/BPT trap",
    "IOT trap",
    "EMT trap",
    "Floating exception",
    "Killed",
    "Bus error",
    "Segmentation fault",
    "Bad system call",
    "Broken pipe",
    "Alarm clock",
    "Terminated",
#ifdef	SIGURG
    "Urgent I/O condition",
#else
    NULL,
#endif
    "Stopped (signal)",
    "Stopped",
    "Continued",
    "Child exited",
    "Stopped (tty input)",
    "Stopped (tty output)",
    "Tty input interrupt",
    "Cputime limit exceeded",
    "Filesize limit exceeded",
    NULL
};
#else
extern  char *sys_siglist[];
#endif	BSD42

/*  */

int	pidstatus (status, fp, cp)
register int   status;
register FILE *fp;
register char *cp;
{
    int     signum;

    if ((status & 0xff00) == 0xff00)
	return status;

    switch (signum = status & 0x007f) {
	case OK: 
	    if (signum = ((status & 0xff00) >> 8)) {
		if (cp)
		    fprintf (fp, "%s: ", cp);
		fprintf (fp, "Exit %d\n", signum);
	    }
	    break;

	case SIGINT: 
	    break;

	default: 
	    if (cp)
		fprintf (fp, "%s: ", cp);
#ifndef	BSD42
	    if (signum >= sizeof sigs || sigs[signum] == NULL)
		fprintf (fp, "Signal %d", signum);
	    else
		fprintf (fp, "%s", sigs[signum]);
#else	BSD42
	    if (signum >= NSIG)
		fprintf (fp, "Signal %d", signum);
	    else
		fprintf (fp, "%s", sys_siglist[signum]);
#endif	BSD42
	    fprintf (fp, "%s\n", status & 0x80 ? " (core dumped)" : "");
	    break;
    }

    return status;
}
