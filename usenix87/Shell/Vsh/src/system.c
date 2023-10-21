/* @(#)system.c	4.1 (Berkeley) 12/21/80 */
#include	<signal.h>
#ifdef	VSH
#include	"hd.h"
#endif

system(s)
char *s;
{
	int status, pid, w;
	register int (*istat)(), (*qstat)();

#ifdef	VFORK
	if ((pid = vfork()) == 0) {
#else
	if ((pid = fork()) == 0) {
#endif
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		for (pid = 3; pid < 20; close(pid++));
#ifdef	VSH
		if (SHELL && *SHELL)
			execl(SHELL, "sh", "-c", s, 0);
#endif
		execl("/bin/sh", "sh", "-c", s, 0);
		_exit(127);
	}
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	while ((w = wait(&status)) != pid && w != -1)
		;
	if (w == -1)
		status = -1;
	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);
	return(status);
}
