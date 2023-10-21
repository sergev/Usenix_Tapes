/*
 *	holdscreen - run arbitrary screen saver.
 *
 *	Holdscreen runs the rest of its arguments as a subprocess, and
 *	waits for keyboard activity.  On the SUN console, mouse activity
 *	is also waited for.  When a key is pressed, the subprocesses
 *	are killed and the screen cleared.
 *
 *	Written by Clyde Hoover, UTexas Computation Center, July 1985
 *
 *	Usage: holdscreen program program-args
 *
 *	Compliation:	cc -o holdscreen -O holdscreen.c
 *	Libraries:	std
 *	Binary:		/usr/local/holdscreen
 *	Mode:		0555
 *	Ownership:	?/?
 */

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <sgtty.h>

char	*clear = "\014";	/* SUN console screen clear */
jmp_buf	ping;			/* Longjmp buffer */
struct sgttyb	oldmodes;	/* TTY mode buf */

/*
 *	holdscreen - screen saver exec
 */
main(argc,argv)
int	argc;
char	**argv;
{
	int	pgrp;		/* Subprocess process group */

	setbuf(stderr, (char *)NULL);	/* No error message buffering */
	pgrp = execargs (argc,argv);	/* Exec screen saver */
	waitforevent ();		/* Wait for human interaction */
	killoff (pgrp);			/* Kill off children */
					/* init (pid 1) will inherit them */
	cleanup ();			/* Reset TTY modes */
	(void) write (1, clear, 1);	/* Clear screen */
	exit (0);
}

/*
 *	catchsig - catch signals
 */
catchsig(sig)
int	sig;		/* Signal that got us here */
{
	(void) signal(sig, SIG_IGN);
	longjmp(ping,1);
}

/*
 *	cleanup - tidy up
 */
cleanup()
{
	(void) ioctl (0, TIOCSETP, &oldmodes);
}

/*
 *	execargs - fork off command line
 *
 *	Returns: process group number of the subprocess tree
 */
execargs(c,v)
int	c;
char	**v;
{
	int	pid;		/* process id */

	c--;
	v++;
	if (c <= 0)		/* No args */
		exit (0);
	pid = fork ();
	if (pid < 0)
		exit (1);
	if (pid == 0) {
		(void) setpgrp (0, getpid());
		(void) close (0);		/* Redirect stdin */
		(void) open ("/dev/null",0);
		(void) execv (v[0], v, 0);
		_exit (0);
	}
	return (pid);
}


/*
 *	killoff - kill subprocess tree.
 */
killoff (pg)
int	pg;		/* Subprocess process group */
{
	if (vfork () == 0) {
		if (setpgrp(0, pg) < 0)		/* Put self in childs pgrp */
			perror("\r\nsetpgrp");
		if (kill (0, SIGKILL) < 0)	/* Shoot us all in head */
			perror ("\r\nkillpg");
		_exit (0);
	}
	(void) wait(0);
}

/*
 *	waitforevent - wait for human-genereated event (hitting a key or
 *		or mouse button)
 */
waitforevent()
{
	int	(*catchsig)();	/* SIGCHLD catcher */
	int	imask;		/* Select mask */
	struct sgttyb	sx;	/* TTY RAW mode */
	int x;

	/*
	 * Set RAW if needed. Do this because I want select
	 * to return if any key on the keyboard is pressed, and
	 * I don't want to catch tty signals.
	 */
	(void) ioctl(0, TIOCGETP, &sx);
	oldmodes = sx;
	if ((sx.sg_flags & RAW) == 0) {
		sx.sg_flags |= RAW;		/* RAW on */
		sx.sg_flags &= ~ECHO;		/* ECHO off */
		(void) ioctl(0, TIOCSETP, &sx);
	}

	/*
	 * Arrange for SIGCHLD to be caught
	 */
	(void) signal(SIGCHLD, catchsig);
	if (x = setjmp(ping))
		goto restore;

	/*
	 * If on SUN console, turn mouse on (N/A)
	 *
	 *if (strcmp(ttyname(0),"/dev/console") == 0)
	 *	enable_mouse();
	 */

	/*
	 * Wait for stdin to become readable (tty input).
	 * This won't work quite the same with the mouse enabled.
	 */
	(void) ioctl (0, TIOCFLUSH, 0);
	imask = 1;
	(void) select(20,&imask,(int *)0,(int *)0,(struct timeval *)0);

restore:
	(void) signal(SIGCHLD, SIG_DFL);
}
