/*
 *	uw - UNIX windows program for the Macintosh (VAX end)
 *
 * Copyright 1985 by John D. Bruner.  All rights reserved.  Permission to
 * copy this program is given provided that the copy is not sold and that
 * this copyright notice is included.
 *
 * Compile: cc -o uw -O uw.c
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/uio.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include "uw.h"

#define	MAXENV	128			/* maximum environment size */

#define	W_IN	0
#define	W_OUT	1

#define	NFDS	20			/* max number of file descriptors */

#define	XON	0021			/* ASCII XON */
#define	XOFF	0023			/* ASCII XOFF */
#define	RUB	0177			/* ASCII RUBOUT */

#define	META	0200			/* "meta" bit for whatever it's worth */

#define	RCV_OK	(-1)			/* see recvcmd() */
#define	RCV_META (-2)			/* see recvcmd() */

typedef int fildes_t;			/* file descriptor data type */
typedef int nwin_t;			/* window index data type */

struct window {
	fildes_t	w_fd;
	char		w_tty[32];
};

struct window window[NWINDOW];		/* window data structures */
struct window *fdmap[NFDS];		/* mapping from fd's to windows */
struct window *curwin[2];		/* current input and output windows */

char portal[] = "/tmp/uwXXXXXX";	/* UNIX-domain network address */


/* The following are added to the environment of all child processes */

char env_uwin[64] = "UWIN=";
char *env[] = { 
	env_uwin,
	"TERM=adm31",
	"TERMCAP=adm31:cr=^M:do=^J:nl=^J:al=\\EE:am:le=^H:bs:ce=\\ET:cm=\\E=%+ %+ :cl=^Z:cd=\\EY:co#80:dc=\\EW:dl=\\ER:ei=\\Er:ho=^^:im=\\Eq:li#24:mi:nd=^L:up=^K:MT:km:",
	NULL
};

int ctlch[] = { -1, IAC, XON, XOFF, -1, -1, -1, -1 };	/* CTL char mapping */

struct selmask {
	int	sm_rd;
	int	sm_wt;
	int	sm_ex;
} selmask[2];

extern char *strncpy(), *strncat();
extern char *mktemp();
extern char *getenv();
extern done(), cwait(), onalarm();
extern int errno;

main(argc, argv)
char **argv;
{
	register fildes_t fd, sd;
	register struct window *w;
	struct sockaddr sa;

	/*
	 * Make sure we don't accidentally try to run this inside itself.
	 */
	if (getenv("UWIN")) {
		fprintf(stderr, "%s is already running\n", *argv);
		exit(1);
	}

	/*
	 * Close all file descriptors except 0, 1, and 2.
	 */

	for (fd=3; fd < NFDS; fd++)
		(void)close(fd);

	/*
	 * Mark all windows closed.
	 */

	for (w=window; w < window+NWINDOW; w++)
		w->w_fd = -1;


	/*
	 * Create a UNIX-domain network address, and put its name into
	 * the environment so that descendents can contact us with new
	 * window requests.
	 */

	(void)strncat(env_uwin, mktemp(portal), sizeof env_uwin - 1);
	setenv(env);

	if ((sd=socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}
	sa.sa_family = AF_UNIX;
	(void)strncpy(sa.sa_data, portal, sizeof sa.sa_data-1);
	sa.sa_data[sizeof sa.sa_data-1] = '\0';
	if (bind(sd, &sa, sizeof sa.sa_family + strlen(sa.sa_data)) < 0) {
		perror("bind");
		exit(1);
	}
	if (fcntl(sd, F_SETFL, FNDELAY))
		perror("fcntl(sd, F_SETFL, FNDELAY)");


	/*
	 * Ignore interrupts, quits, and terminal stops.  Clean up and exit
	 * if a hangup or termination is received.  Also catch changes in
	 * child status (so that we can wait for them).  Set up the terminal
	 * modes.
	 */

	(void)signal(SIGHUP, done);
	(void)signal(SIGINT, SIG_IGN);
	(void)signal(SIGQUIT, SIG_IGN);
	(void)signal(SIGTERM, done);
	(void)signal(SIGTSTP, SIG_IGN);
	(void)signal(SIGCHLD, cwait);

	tmode(1);


	/*
	 * Tell the Macintosh to initialize.  Initialize the current input
	 * and output windows to NULL.
	 */

	xmitcmd(CB_FN_MAINT|CB_MF_ENTRY);
	curwin[W_IN] = curwin[W_OUT] = NULL;

	
	/*
	 * Initialize the "select" masks, create window 1 (to start
	 * things off) and wait for input.  When input is available,
	 * process it.
	 */

	selmask[0].sm_rd = (1<<0)|(1<<sd);
	selmask[0].sm_wt = 0;
	selmask[0].sm_ex = selmask[0].sm_rd | 2;
	if (newwind(window) == 0)
		xmitcmd(CB_FN_NEWW|1);

	while (1) {
		selmask[1] = selmask[0];
		if (select(NFDS, &selmask[1].sm_rd, &selmask[1].sm_wt,
		    &selmask[1].sm_ex, (struct timeval *)0) < 0) {
			if (errno == EINTR)
				continue;
			perror("select");
			done(1);	/* for now -- fix this! */
		}
		for (fd=0; fd < NFDS; selmask[1].sm_rd >>= 1, fd++) {
			if (selmask[1].sm_rd&(1<<0)) {
				if (fd < 2)
					mrecv();
				else if (fd == sd)
					netrecv(sd);
				else
					mxmit(fd);
			}
		}
	}
}

mrecv()
{
	register int len;
	register char *cp, *cq;
	auto int nready;
	char ibuf[512], obuf[512];
	static int seen_iac, seen_meta;

	/*
	 * The received bytestream is examined.  Non-command bytes are
	 * written to the file descriptor corresponding to the current
	 * "input" window (relative to the Macintosh -- the window the
	 * user types input to).
	 */

	if (ioctl(0, (int)FIONREAD, (char *)&nready) < 0) {
		perror("FIONREAD");
		return;
	}

	cq = obuf;
	while (nready > 0) {
		if (nready > sizeof ibuf)
			len = read(0, ibuf, sizeof ibuf);
		else
			len = read(0, ibuf, nready);
		if (len <= 0) {
			perror("read");
			return;
		}
		for (cp=ibuf; cp < ibuf+len; cp++) {
			if (seen_iac) {
				if ((*cp&CB_DIR) == CB_DIR_MTOH) {
					if (cq > obuf) {
						(void)write(curwin[W_IN]->w_fd,
							    obuf, cq-obuf);
						cq = obuf;
					}
					switch (*cq = recvcmd(*cp)) {
					case RCV_OK:
						break;
					case RCV_META:
						seen_meta = 1;
						break;
					default:
						if (seen_meta) {
							seen_meta = 0;
							*cq |= META;
						}
						if (curwin[W_IN])
							cq++;
						break;
					}
				}
				seen_iac = 0;
			} else if (*cp == IAC)
				seen_iac++;
			else if (curwin[W_IN]) {
				if (seen_meta) {
					seen_meta = 0;
					*cq++ = *cp|META;
				} else
					*cq++ = *cp;
				if (cq >= obuf+sizeof obuf) {
					(void)write(curwin[W_IN]->w_fd,
						    obuf, cq-obuf);
					cq = obuf;
				}
			}
		}
		nready -= len;
	}
	if (cq > obuf)
		(void)write(curwin[W_IN]->w_fd, obuf, cq-obuf);
}

recvcmd(cmd)
char cmd;
{
	register int nwin, fn;
	register struct window *w;

	/*
	 * Perform the function the Mac is asking for.  There are three
	 * broad categories of these functions: RCV_META, which tells
	 * the caller that the next character is a "meta" character;
	 * an ASCII data character, which is passed back to the caller
	 * for proper handling; and RCV_OK, which means that this routine
	 * has done everything which was required to process the command.
	 */

	fn = cmd&CB_FN;
	switch (fn) {
	case CB_FN_NEWW:
	case CB_FN_KILLW:
	case CB_FN_ISELW:
		nwin = cmd&CB_WINDOW;
		if (!nwin)
			break;
		w = &window[nwin-1];

		switch (fn) {
		case CB_FN_NEWW:
			if (w->w_fd < 0 && newwind(w) < 0)
				xmitcmd(CB_FN_KILLW|nwin);
			break;

		case CB_FN_KILLW:
			killwind(w, 0);
			break;

		case CB_FN_ISELW:
			if (w->w_fd >= 0)
				curwin[W_IN] = w;
			break;
		}
		break;

	case CB_FN_META:
		return(RCV_META);

	case CB_FN_CTLCH:
		return(ctlch[cmd&CB_CC]);

	case CB_FN_MAINT:
		if ((cmd&CB_MF) == CB_MF_EXIT)
			done(0);
		/*NOTREACHED*/
	}
	return(RCV_OK);
}

xmitcmd(cmd)
char cmd;
{
	static char cmdbuf[2] = { IAC, '\0' };

	/*
	 * Transmit the command "cmd" to the Macintosh.  The byte is ORed
	 * with the host-to-Mac direction indicator.
	 */

	cmdbuf[1] = cmd|CB_DIR_HTOM;
	(void)write(1, cmdbuf, sizeof cmdbuf);
}

netrecv(sd)
register fildes_t sd;
{
	register struct window *w;
	register int cnt;
	struct msghdr msg;
	auto int fd;
	struct iovec iov;
	struct stat st1, st2;
	static int unity = 1;
	char buf[256];


	/*
	 * main() calls this routine when there is a message waiting on
	 * the UNIX-domain socket.  The message's access rights are
	 * expected to contain the file descriptor for the "master" side
	 * of a pseudo-tty.  The message contains the name of the pty.
	 * The sender is expected to start up a process on the slave side
	 * of the pty.  This allows the host end to create windows which
	 * run something other than the shell.
	 */

	fd = -1;

	iov.iov_base = (caddr_t)buf;
	iov.iov_len = sizeof buf - 1;

	msg.msg_name = (caddr_t)0;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_accrights = (caddr_t)&fd;
	msg.msg_accrightslen = sizeof fd;

	if ((cnt=recvmsg(sd, &msg, 0)) < 0)
		perror("recvmsg");

	if (msg.msg_accrightslen > 0 && fd >= 0) {
		/*
		 * We can't trust the process which connected to us, so
		 * we verify that it really passed us a pseudo-tty's
		 * file descriptor by checking the device name and its
		 * inode number.  [Of course, if someone else wants to
		 * hand us a terminal session running under their uid....]
		 */
		buf[cnt] = 0;
		if (strncmp(buf, "/dev/ptyp", sizeof "/dev/ptyp" - 1) ||
		    fstat(fd, &st1) < 0 || stat(buf, &st2) < 0 ||
		    st1.st_dev != st2.st_dev || st1.st_ino != st2.st_ino) {
			(void)close(fd);
			return;
		}
		/*
		 * OK, we believe the sender.  We allocate a window and
		 * tell the Macintosh to create that window on its end.
		 */
		buf[5] = 't';		/* switch to "/dev/ttyp?" */
		for (w=window; w < window+NWINDOW; w++) {
			if (w->w_fd < 0) {
				w->w_fd = fd;
				fdmap[fd] = w;
				selmask[0].sm_rd |= (1<<fd);
				(void)strncpy(w->w_tty, buf, sizeof w->w_tty-1);
				xmitcmd(CB_FN_NEWW|(w-window+1));
				break;
			}
		}
		/*
		 * If we have no free windows, then we close the file
		 * descriptor (which will terminate the slave process).
		 */
		if (w == window+NWINDOW)
			(void)close(fd);
		/*
		 * Set non-blocking I/O on the master side.
		 */
		(void)ioctl(fd, FIONBIO, &unity);
	}
}

mxmit(fd)
register fildes_t fd;
{
	register char *cp, *cq, i;
	register int len;
	char ibuf[16], obuf[16];

	/*
	 * Copy input from file "fd" to the Macintosh.  Be sure to convert
	 * any embedded IAC characters.
	 *
	 * Note that the input/output buffers should NOT be very large.
	 * It is undesirable to perform large reads and effectively
	 * "lock out" all other file descriptors.  The chosen size
	 * should preserve a reasonable amount of efficiency.
	 */

	if (fdmap[fd]) {
		curwin[W_OUT] = fdmap[fd];
		xmitcmd(CB_FN_OSELW|(fdmap[fd]-window+1));
		cq = obuf;
		if ((len = read(fd, ibuf, sizeof ibuf)) < 0 &&
		    errno != EWOULDBLOCK) {
			killwind(fdmap[fd], 1);
			return;
		}
		for (cp=ibuf; cp < ibuf+len; cp++) {
			if (*cp&META) {
				if (cq > obuf) {
					(void)write(1, obuf, cq-obuf);
					cq = obuf;
				}
				xmitcmd(CB_FN_META);
				*cp &= ~META;
			}
			i = -1;
			if (*cp == RUB || *cp < ' ') {
				i = sizeof ctlch;
				while (i >= 0 && ctlch[i] != *cp)
					i--;
			}
			if (i >= 0) {
				if (cq > obuf) {
					(void)write(1, obuf, cq-obuf);
					cq = obuf;
				}
				xmitcmd(CB_FN_CTLCH|i);
			} else {
				*cq++ = *cp;
				if (cq >= obuf+sizeof obuf) {
					(void)write(1, obuf, cq-obuf);
					cq = obuf;
				}
			}
		}
	} else
		(void)read(fd, ibuf, sizeof ibuf);
	if (cq > obuf)
		(void)write(1, obuf, cq-obuf);
}

killwind(w, notify)
register struct window *w;
{
	register int mask;

	/*
	 * Kill window "w".  Notify the Macintosh that it is gone if
	 * "notify" is nonzero.
	 */

	(void)close(w->w_fd);
	mask = ~(1<<w->w_fd);
	fdmap[w->w_fd] = NULL;
	w->w_fd = -1;
	if (curwin[W_IN] == w)
		curwin[W_IN] = NULL;
	if (curwin[W_OUT] == w)
		curwin[W_OUT] = NULL;
	selmask[0].sm_rd &= mask;
	selmask[0].sm_wt &= mask;
	selmask[0].sm_ex &= mask;
	if (notify)
		xmitcmd(CB_FN_KILLW|(w-window+1));
}

newwind(w)
register struct window *w;
{
	register char *cp;
	register fildes_t fd;
	register int pid;
	register char *shell;
	char pty[32], ptysufx[2];
	static char ptyidx[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	static int unity = 1;
	extern char *getenv();

	/*
	 * Create a new window using the specified component of "window".
	 * This routine isn't very smart at finding pseudo-ttys.
	 */

	ptysufx[1] = '\0';
	for (cp=ptyidx; *cp; cp++) {
		ptysufx[0] = *cp;
		(void)strncpy(pty, "/dev/ptyp", sizeof pty-1);
		(void)strncat(pty, ptysufx, sizeof pty-1);
		if ((fd = open(pty, 2)) >= 0)
			break;
	}
	if (fd < 0)
		return(-1);
	(void)ioctl(fd, FIONBIO, &unity);	/* set non-blocking I/O */
	fdmap[fd] = w;
	w->w_fd = fd;
	selmask[0].sm_rd |= (1<<fd);
	(void)strncpy(w->w_tty, "/dev/ttyp", sizeof w->w_tty-1);
	(void)strncat(w->w_tty, ptysufx, sizeof w->w_tty-1);

	while ((pid=fork()) < 0)
		sleep(5);
	if (!pid) {
		(void)signal(SIGHUP, SIG_DFL);
		(void)signal(SIGINT, SIG_DFL);
		(void)signal(SIGQUIT, SIG_DFL);
		(void)signal(SIGTERM, SIG_DFL);
		(void)signal(SIGTSTP, SIG_IGN);
		(void)signal(SIGCHLD, SIG_DFL);
		(void)ioctl(open("/dev/tty", 2), (int)TIOCNOTTY, (char *)0);
		(void)setuid(getuid());		/* shouldn't need this */
		if ((fd=open(w->w_tty, 2)) < 0)
			_exit(1);
		if (!(shell = getenv("SHELL")))
			shell = "/bin/sh";
		(void)dup2(fd, 0);
		(void)dup2(0, 1);
		(void)dup2(0, 2);
		for (fd=3; fd < NFDS; fd++)
			(void)close(fd);
		tmode(0);	/* HACK! */
		execl(shell, shell, (char *)0);
		_exit(1);
	}

	return(0);
}

tmode(f)
{
	static struct sgttyb ostty, nstty;
	static struct tchars otchars, ntchars;
	static struct ltchars oltchars, nltchars;
	static int olmode, nlmode;
	static saved;

	/*
	 * This routine either saves the current terminal modes and then
	 * sets up the terminal line or resets the terminal modes (depending
	 * upon the value of "f").  The terminal line is used in "cbreak"
	 * mode with all special characters except XON/XOFF disabled.  The
	 * hated (by me) LDECCTQ mode is required for the Macintosh to
	 * handle flow control properly.
	 */

	if (f == 1) {
		if (ioctl(0, (int)TIOCGETP, (char *)&ostty) < 0) {
			perror("ioctl((int)TIOCGETP)");
			done(1);
		}
		if (ioctl(0, (int)TIOCGETC, (char *)&otchars) < 0) {
			perror("ioctl((int)TIOCGETC)");
			done(1);
		}
		if (ioctl(0, (int)TIOCGLTC, (char *)&oltchars) < 0) {
			perror("ioctl((int)TIOCGLTC)");
			done(1);
		}
		if (ioctl(0, (int)TIOCLGET, (char *)&olmode) < 0) {
			perror("ioctl((int)TIOCLGET)");
			done(1);
		}
		nstty = ostty;
		nstty.sg_erase = nstty.sg_kill = -1;
		nstty.sg_flags |= CBREAK;
		nstty.sg_flags &= ~(RAW|CRMOD|ECHO|LCASE|XTABS);
		ntchars.t_intrc = ntchars.t_quitc = -1;
		ntchars.t_eofc = ntchars.t_brkc = -1;
		ntchars.t_startc = XON;
		ntchars.t_stopc = XOFF;
		nltchars.t_suspc = nltchars.t_dsuspc = -1;
		nltchars.t_rprntc = nltchars.t_flushc = -1;
		nltchars.t_werasc = nltchars.t_lnextc = -1;
		nlmode = olmode | LDECCTQ;
		if (ioctl(0, (int)TIOCSETN, (char *)&nstty) < 0) {
			perror("ioctl((int)TIOCSETN)");
			done(1);
		}
		if (ioctl(0, (int)TIOCSETC, (char *)&ntchars) < 0) {
			perror("ioctl((int)TIOCSETC");
			done(1);
		}
		if (ioctl(0, (int)TIOCSLTC, (char *)&nltchars) < 0) {
			perror("ioctl((int)TIOCSLTC");
			done(1);
		}
		if (ioctl(0, (int)TIOCLSET, (char *)&nlmode) < 0) {
			perror("ioctl((int)TIOCLSET)");
			done(1);
		}
		saved = 1;
	} else if (saved) {
		(void)ioctl(0, (int)TIOCSETP, (char *)&ostty);
		(void)ioctl(0, (int)TIOCSETC, (char *)&otchars);
		(void)ioctl(0, (int)TIOCSLTC, (char *)&oltchars);
		(void)ioctl(0, (int)TIOCLSET, (char *)&olmode);
	}
}

done(s)
{
	register struct window *w;
	register fildes_t fd;

	/*
	 * Clean up and exit.  It is overkill to close all of the file
	 * descriptors, but it causes no harm.  After we are sure that
	 * our UNIX-domain network connection is closed we remove the
	 * entry that it created (as a side effect) in the filesystem.
	 *
	 * We also restore the terminal modes.
	 */
	
	/*xmitcmd(CB_FN_MAINT|CB_MF_EXIT);*/
	for (fd=3; fd < NFDS; fd++)
		(void)close(fd);
	(void)unlink(portal);
	tmode(0);
	exit(s);
}

cwait()
{
	union wait status;
	struct rusage rusage;

	/*
	 * Collect dead children.  We don't use the information that
	 * wait3() returns.  (Someday we might.)
	 */
	
	while (wait3(&status, WNOHANG, &rusage) > 0)
		;
}


static char *earray[MAXENV+1];

setenv(env)
char **env;
{
	register char **ep1, **ep2, *cp;
	char **ep3;
	extern char **environ;


	/*
	 * Merge the set of environment strings in "env" into the
	 * environment.
	 */

	/*
	 * The first time through, copy the environment from its
	 * original location to the array "earray".  This makes it a
	 * little easier to change things.
	 */

	if (environ != earray){
		ep1=environ;
		ep2=earray;
		while(*ep1 && ep2 <= earray+MAXENV)
			*ep2++ = *ep1++;
		*ep2++ = NULL;
		environ = earray;
	}


	/*
	 * If "env" is non-NULL, it points to a list of new items to
	 * be added to the environment.  These replace existing items
	 * with the same name.
	 */

	if (env){
		for(ep1=env; *ep1; ep1++){
			for(ep2=environ; *ep2; ep2++)
				if (!envcmp(*ep1, *ep2))
					break;
			if (ep2 < earray+MAXENV) {
				if (!*ep2)
					ep2[1] = NULL;
				*ep2 = *ep1;
			}
		}
	}


	/* Finally, use an insertion sort to put things in order. */

	for(ep1=environ+1; cp = *ep1; ep1++){
		for(ep2=environ; ep2 < ep1; ep2++)
			if (envcmp(*ep1, *ep2) < 0)
				break;
		ep3 = ep2;
		for(ep2=ep1; ep2 > ep3; ep2--)
			ep2[0] = ep2[-1];
		*ep2 = cp;
	}
}


static
envcmp(e1, e2)
register char *e1, *e2;
{
	register d;

	do {
		if (d = *e1 - *e2++)
			return(d);
	} while(*e1 && *e1++ != '=');
	return(0);
}
