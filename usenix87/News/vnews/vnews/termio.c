#include "vnews.h"
#if USGREL >= 30
#include <termio.h>
#include <fcntl.h>
#else
#include <sgtty.h>
#endif
#include <errno.h>
extern int errno;
static int alflag;			/* set if unprocessed alarm signal */
static int intflag;			/* set if interrupt received */
#ifdef SIGTSTP
static int inraw;			/* true if in raw mode */
#endif
int onstop();


/*** Terminal I/O ***/

#define INBUFSIZ 8

char inbuf[INBUFSIZ];			/* input buffer */
char outbuf[BUFSIZ];			/* output buffer */
int innleft = 0;			/* # of chars in input buffer */
int outnleft = BUFSIZ;			/* room left in output buffer */
char *innext;				/* next input character */
char *outnext = outbuf;			/* next space in output buffer */
#ifndef CURSES
#if USGREL >= 30
int oflags;				/* fcntl flags (for nodelay read) */
#endif
#endif


/*
 * Input a character
 */

vgetc() {
	register c;
#if BSDREL >= 42
	int readfds, exceptfds;
#endif

recurse:
	if (--innleft >= 0) {
		c = *innext++;
	} else {
		if (alflag)
			timer();
		updscr();	/* update the display */
		for (;;) {
			if (innleft > 0 || alflag)
				goto recurse;
			intflag = 0;
#ifndef CURSES
#if USGREL >= 30
			if (oflags & O_NDELAY) {
				oflags &=~ O_NDELAY;
				fcntl(0, F_SETFL, oflags);
			}
#endif
#endif
#if BSDREL >= 42
			/* Use a select because can be interrupted */
			readfds = 1; exceptfds = 1;
			innleft = select(1, &readfds, 0, &exceptfds, 0);
			if (innleft > 0) {
				if ((innleft = read(0, inbuf, INBUFSIZ)) > 0)
					break;
			}
#else
			if ((innleft = read(0, inbuf, INBUFSIZ)) > 0)
				break;
#endif
			if (hupflag)
				return INTR;
			if (innleft == 0 || errno != EINTR)
				abort();	/* "Can't happen" */
			if (intflag) {
				intflag--;
				return INTR;
			}
		}
		innext = inbuf + 1;
		innleft--;
		c = inbuf[0];
	}
#ifdef V6
	c &= 0177;
	if (c == '\034')	/* FS character */
		onquit();
	if (c == '\177')	/* DEL */
		onint();
#endif
	if (c == '\f') {
		clearok(curscr, 1);
		goto recurse;
	}
	if (c == '\r')
		c = '\n';
	return c;
}


/*
 * Push a character back onto the input stream.
 */

pushback(c) {
	if (innext <= inbuf)
		abort();
	*--innext = c;
	innleft++;
}



/*
 * Check for terminal input
 */

checkin() {
#ifndef CURSES
#if BSDREL > 7
	long count;

#endif
#endif
#ifdef STATTOP
	if (innleft > 0)
#else
	if (innleft > 0 || alflag)
#endif
		return 1;
#if !defined(CURSES) && (USGREL >= 30 || BSDREL > 7)
	if (ospeed == B9600)
		return 0;
	vflush();
	if (ospeed <= B300)
		ttyowait();
#if USGREL >= 30
	if ((oflags & O_NDELAY) == 0) {
		oflags |= O_NDELAY;
		fcntl(0, F_SETFL, oflags);
	}
	if ((innleft = read(0, inbuf, INBUFSIZ)) > 0) {
		innext = inbuf;
		return 1;
	}
#else
	count = 0;			/* in case FIONREAD fails */
	ioctl(0, FIONREAD, &count);
	if (count)
		return 1;
#endif
#endif
	return 0;
}



/*
 * flush terminal input queue.
 */

clearin() {
#if USGREL >= 30
	ioctl(0, TCFLSH, 0);
#else
#ifdef TIOCFLUSH
	ioctl(0, TIOCFLUSH, 0);
#else
	struct sgttyb tty;
	gtty(0, &tty);
	stty(0, &tty);
#endif
#endif
	innleft = 0;
}


vputc(c) {
	if (--outnleft < 0) {
		vflush();
		outnleft--;
	}
	*outnext++ = c;
}


/*
 * Flush the output buffer
 */

vflush() {
	register char *p;
	register int i;
#if BSDREL <= 7 && USGREL < 30
	int svalarm = alarm(0);	    /* signals cause UNIX to drop characters */
#endif

	for (p = outbuf ; p < outnext ; p += i) {
		if (hupflag)
			break;
		if ((i = write(1, p, outnext - p)) < 0) {
			if (errno != EINTR)
				abort();	/* "Can't happen" */
			i = 0;
		}
	}
	outnleft = BUFSIZ;
	outnext = outbuf;
#if BSDREL <= 7 && USGREL < 30
	alarm(svalarm);
#endif
}




/*** terminal modes ***/

#ifndef CURSES
#if USGREL >= 30
static struct termio oldtty, newtty;


/*
 * Save tty modes
 */

ttysave() {
	if (ioctl(1, TCGETA, &oldtty) < 0)
		xerror("Can't get tty modes");
	newtty = oldtty;
	newtty.c_iflag &=~ (INLCR|IGNCR|ICRNL);
	newtty.c_oflag &=~ (OPOST);
	newtty.c_lflag &=~ (ICANON|ECHO|ECHOE|ECHOK|ECHONL);
	newtty.c_lflag |=  (NOFLSH);
	newtty.c_cc[VMIN] = 1;
	newtty.c_cc[VTIME] = 0;
	cerase = oldtty.c_cc[VERASE];
	ckill = oldtty.c_cc[VKILL];
	ospeed = oldtty.c_cflag & CBAUD;
	initterm();
}


/*
 * Set tty modes for visual processing
 */

ttyraw() {
	while (ioctl(1, TCSETAF, &newtty) < 0 && errno == EINTR);
	rawterm();
}



ttyowait() {	/* wait for output queue to drain */
	while (ioctl(1, TCSETAW, &newtty) < 0 && errno == EINTR);
}


/*
 * Restore tty modes
 */

ttycooked() {
	cookedterm();
	while (ioctl(1, TCSETAF, &oldtty) < 0 && errno == EINTR);
	oflags &=~ O_NDELAY;
	fcntl(0, F_SETFL, oflags) ;
}

#else

static struct sgttyb oldtty, newtty;
#if BSDREL >= 40 && BSDREL <= 41
static struct tchars oldtchars, newtchars;
#endif


/*
 * Save tty modes
 */

ttysave() {
#ifdef SIGTSTP
	/* How to get/change terminal modes in a job control environment.
	   This code is right from the 4.1 bsd jobs(3) manual page.
	 */
#if BSDREL >= 42
	int tpgrp, getpgrp();
#else
	short tpgrp, getpgrp();
#endif

retry:
#if BSDREL >= 42
	sigblock(1<<(SIGTSTP-1) | 1<<(SIGTTIN-1) | 1<<(SIGTTOU-1));
#else
	sigset(SIGTSTP, SIG_HOLD);
	sigset(SIGTTIN, SIG_HOLD);
	sigset(SIGTTOU, SIG_HOLD);
#endif
	if (ioctl(2, TIOCGPGRP, &tpgrp) != 0)
		goto nottty;
	if (tpgrp != getpgrp(0)) { /* not in foreground */
		sigset(SIGTTOU, SIG_DFL);
#if BSDREL >= 42
		sigsetmask(sigblock(0) & ~(1<<(SIGTTOU-1)));
#endif
		kill(0, SIGTTOU);
		/* job stops here waiting for SIGCONT */
		goto retry;
	}
	sigset(SIGTTIN, onstop);
	sigset(SIGTTOU, onstop);
	sigset(SIGTSTP, onstop);
#if BSDREL >= 42
	sigsetmask(sigblock(0) & ~(1<<(SIGTSTP-1) | 1<<(SIGTTIN-1) | 1<<(SIGTTOU-0)));
#endif
#endif
	if (gtty(1, &oldtty) < 0)
nottty:		xerror("Can't get tty modes");
	newtty = oldtty;
	newtty.sg_flags &=~ (CRMOD|ECHO|XTABS);
#if BSDREL >= 7
	newtty.sg_flags |= CBREAK;
#else
	newtty.sg_flags |= RAW;
#endif
	cerase = oldtty.sg_erase;
	ckill = oldtty.sg_kill;
	ospeed = oldtty.sg_ospeed;
#if BSDREL >= 40 && BSDREL <= 41
	ioctl(1, TIOCGETC, (char *)&oldtchars);
	newtchars = oldtchars;
	if (oldtchars.t_intrc == '\7')
		newtchars.t_intrc = -1;
#endif
	initterm();
}


/*
 * Set tty modes for visual processing
 */

ttyraw() {
#ifdef SIGTSTP
	inraw = 1;
#endif
	while (stty(1, &newtty) < 0 && errno == EINTR);
#if BSDREL >= 40 && BSDREL <= 41
	ioctl(1, TIOCSETC, (char *) &newtchars);
#endif
	rawterm();
}



ttyowait() {	/* wait for output queue to drain */
#ifdef TIOCDRAIN	/* This ioctl is a local mod on linus */
	ioctl(1, TIOCDRAIN, 0);
#endif
}


/*
 * Restore tty modes
 */

ttycooked() {
	cookedterm();
	while (stty(1, &oldtty) < 0 && errno == EINTR);
#if BSDREL >= 40 && BSDREL <= 41
	ioctl(1, TIOCSETC, (char *) &oldtchars);
#endif
#ifdef SIGTSTP
	inraw = 0;
#endif
}

#endif
#else
#ifdef HCURSES

ttysave() {
	initscr();
	idlok(stdscr, 1);
	intrflush(curscr, 0);
	cerase = erasechar();
	ckill = killchar();
}


ttyraw() {
	reset_prog_mode();
	cbreak();
	nonl();
	noecho();
}


ttycooked() {
	reset_shell_mode();
}


int _endwin;		/* [expletives deleted] */

#else

ttysave() {
#if USGREL >= 30
	struct termio tty;
#else
	struct sgttyb tty;
#endif

	initscr();
#if USGREL >= 30
	ioctl(0, TCGETA, &tty);
	cerase = tty.c_cc[VERASE];
	ckill = tty.c_cc[VKILL];
#else
	gtty(0, &tty);
	cerase = tty.sg_erase;
	ckill = tty.sg_kill;
#endif
}


ttyraw() {
	/*
	 * Vnews really isn't designed to work with RAW mode, so if you
	 * have anything approaching CBREAK mode, use it.
	 */
#ifdef CBREAK
	crmode();
#else
	raw();
#endif
	nonl();
	noecho();
}


ttycooked() {
	resetty();
}

#endif
#endif CURSES



/*** signal handlers ***/

onint() {
	signal(SIGINT, onint);
	clearin();			/* flush input queue */
#if BSDREL >= 40 && BSDREL <= 41
	ioctl(0, TIOCSTI, "\7");
#else
	intflag++;
#endif
}


onhup() {
	signal(SIGHUP, onhup);
	hupflag++;
}


onquit() {
	botscreen();
	vflush();
	ttycooked();
#ifdef COREDUMP
	abort();
#else
	exit(0);
#endif
}

#ifdef SIGTSTP

onstop(signo)
	int signo;
{
	int restore = inraw;
	int e = errno;

	/* restore old terminal state */
	if (restore) {
		botscreen();
		vflush();
		ttycooked();
	}
	sigset(signo, SIG_DFL);
#if BSDREL >= 42
	sigsetmask(sigblock(0) & ~(1<<(signo-1)));
#endif
	kill(getpid(), signo);	/* stop here until continued */

	/* fprintf(stderr,"Vnews restarted."); */
	sigset(signo, onstop);
	/* restore our special terminal state */
	if (restore) {
		ttyraw();
#ifdef TIOCSTI
		ioctl(0, TIOCSTI, "\f");
#else
		clearok(curscr, 1);	/* doesn't seem to work */
#endif
	}
	errno = e;
}

#endif



/*** alarm handler ***/

/* #include <time.h> */


/*
 * Called on alarm signal.
 * Simply sets flag, signal processed later.
 */

onalarm() {
	alflag++;
}


/*
 * Process alarm signal (or start clock)
 */

timer() {
	long tod;
	int hour;
	int i;
	struct tm *t;
	struct stat statb;
	long time();
	static char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
	static long oldmsize = 1000000L;
	static int rccount = 10;

	alflag = 0;
	signal(SIGALRM, onalarm);
	time(&tod);
	t = localtime(&tod);
	i = 60 - t->tm_sec;
	alarm(i > 30? 30 : i);			/* reset alarm */
	hour = t->tm_hour % 12;
	if (hour == 0)  hour = 12;
	sprintf(timestr, "%.3s %d %d:%02d",
		months + 3 * t->tm_mon, t->tm_mday, hour, t->tm_min);
#ifdef GGRMAIL
	if (mailf == NULL || stat(mailf, &statb) < 0) {
		statb.st_size = 0;
	}
	if(statb.st_size > oldmsize) {
		ismail = 1;
		needbeep++;
	} else if (statb.st_size < oldmsize) {
		ismail = 0;
	}
	oldmsize = statb.st_size;
#else
	ismail = 0;
	if (mailf != NULL && stat(mailf, &statb) >= 0 && statb.st_size > 0L) {
		ismail = 1;
		if (oldmsize < statb.st_size) {
			ismail = 2;		/* new mail */
			needbeep++;
		}
	} else {
		statb.st_size = 0L;
	}
	oldmsize = statb.st_size;
#endif
	if (uflag && --rccount < 0) {
		writeoutrc();
		if (secpr[0] == '\0')
			strcpy(secpr, ".newsrc updated");
		rccount = 10;
	}
}
