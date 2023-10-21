/* @(#)popen.c	4.5 (Berkeley) 7/6/84
 * Modified from 4.2, 2.9 BSD popen by Stephen Uitti, PUCC, Nov '85
 * Doesn't invoke shell, saving time, becoming more secure.
 * Calls breakargs to break line into command line arguments,
 * delimited by white space, but ignores globing and quoting
 * characters.  For use where you don't need shell meta character
 * expansion or filename globbing.
 */
#ifdef BSD2_9
#include <sys/types.h>
#endif
#include <stdio.h>
#include <signal.h>

#define	tst(a,b)	(*mode == 'r'? (b) : (a))
#define	RDR	0
#define	WTR	1
#define SPC '\040'			/* ascii space */

static	int popen_pid[20];
extern char **environ;
char **breakargs();

/* no-shell popen */
FILE *
nshpopen(cmd, mode)
char *cmd;
char *mode;
{
	int p[2];
	int myside, hisside, pid;
	char **elist;				/* the execv list */

	elist = breakargs(cmd);
	if (pipe(p) < 0)
		return (NULL);
	myside = tst(p[WTR], p[RDR]);
	hisside = tst(p[RDR], p[WTR]);
	if ((pid = vfork()) == 0) {
		/* myside and hisside reverse roles in child */
		close(myside);
		if (hisside != tst(0, 1)) {	/* 0, 1 => stdin, stdout */
			dup2(hisside, tst(0, 1));
			close(hisside);
		}
		execve(elist[0], elist, environ);
		perror("execv");
		_exit(1);			/* bombed execv, child dies */
	}
	free(elist);				/* discard the broken args */
	if (pid == -1) {
		close(myside);
		close(hisside);
		return (NULL);
	}
	popen_pid[myside] = pid;
	close(hisside);
	return (fdopen(myside, mode));
}

/* close for nshpopen */
int *
nshpclose(ptr)
FILE *ptr;
{
#ifdef	BSD4_2
	int child, pid, status, omask;

	child = popen_pid[fileno(ptr)];
	fclose(ptr);
#define	mask(s)	(1 << ((s)-1))
	omask = sigblock(mask(SIGINT)|mask(SIGQUIT)|mask(SIGHUP));
	while ((pid = wait(&status)) != child && pid != -1)
		/* empty */;
	(void) sigsetmask(omask);
	return (int *)(pid == -1 ? -1 : status);
#else		/* 2.9 BSD, at least */
	register f, r, (*hstat)(), (*istat)(), (*qstat)();
	int status;

	f = fileno(ptr);
	fclose(ptr);
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	hstat = signal(SIGHUP, SIG_IGN);
	while((r = wait(&status)) != popen_pid[f] && r != -1)
		;
	if(r == -1)
		status = -1;
	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);
	signal(SIGHUP, hstat);
	return (int *)(status);
#endif	BSD4_2
}
