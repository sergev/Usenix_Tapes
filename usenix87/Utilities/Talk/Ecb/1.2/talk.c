/*
 * talk - a two-way, screen-oriented communication program
 *
 *	Talk, which emulates the Berkeley program of the same name, allows
 * both parties in a conversation to type at will, without fear of garbling
 * each others messages. Input from either user is immediatly written to both
 * screens so that there is no waiting like there is with write(1).
 *
 *	Although this program resembles, to the user, the Berkeley program
 * 'talk', the code is entirely original by the author.
 *
 * AUTHOR
 *	Edward C. Bennett (edward@ukecc)
 *
 * Copyright 1985 by Edward C. Bennett
 *
 * Permission is given to alter this code as needed to adapt it to forign
 * systems provided that this header is included and that the original
 * author's name is preserved.
 */
#include <curses.h>
#include <signal.h>
#include <fcntl.h>
#include "talk.h"

struct	msgbuf	sndbuf, rcvbuf;

int	msqid;
int	avail;		/* The availabity of a perspective partner */
int	parpid = 0;	/* The pid of the partner */
char	*ctime(), *getlogin();
time_t	tloc, otime, time();

main(argc, argv)
int argc;
char *argv[];
{
	key_t		msgkey, ftok();
	long		fromtype;	/* The msgtype where we look for characters */
					/* from the partner */
#ifdef FIONREAD
	long		waiting;
#endif
	void		exit();
	int		c, Finish();
	int		MIDDLE;
	short		orgy = 0, orgx = 0;	/* That's ORGinator's Y you pervert */
	short		resy, resx = 0;


	msgkey = ftok(TALK_PATH, MAGIC_ID);

	if ((msqid = msgget(msgkey, MSGQ_PERMS)) == -1) {
		fprintf(stderr, "%s: Nonexistant talk msgqueue\n", argv[0]);
		exit(1);
	}

#ifndef SCHIZO
	if (!isatty(0)) {
		fprintf(stderr, "%s: must have a terminal for input\n", argv[0]);
		exit(1);
	}
#endif

#ifndef SCHIZO
	if (argc == 1) {
		fprintf(stderr, "%s: No user specified\n", argv[0]);
		exit(1);
	}

	/*
	 * We need to tell the demon who we are, who we want to talk to,
	 * the tty of who we want (if it was given) and our process id.
	 */
	strncpy(dmsgbuf.msgval.mtext, getlogin(), NAMESIZ);
	strncpy(&dmsgbuf.msgval.mtext[NAMESIZ], argv[1], NAMESIZ);
	if (argv[2])
		strncpy(&dmsgbuf.msgval.mtext[TTYLOC], argv[2], LINESIZ);
	dmsgbuf.msgval.lines[PIDLOC] = getpid();
	dmsgbuf.mtype = 1;

	/*
	 * Tell the demon we are here.
	 */
	if (msgsnd(msqid, (struct msgbuf *)&dmsgbuf, MSGSIZ, 0) == -1) {
		fprintf(stderr, "%s: msgsnd failure(%d)\n", argv[0], errno);
		exit(1);
	}

	/*
	 * Wait for a response from the demon.
	 */
	if (msgrcv(msqid, (struct msgbuf *)&dmsgbuf, MSGSIZ, (long)2, 0) == -1) {
		fprintf(stderr, "%s: msgrcv failure(%d)\n", argv[0], errno);
		exit(1);
	}

	avail = dmsgbuf.msgval.lines[0];
	/*
	 * A negative availability is untalkable.
	 */
	if (avail < 0) {
		Error(avail, argv);
		exit(1);
	}

	sndbuf.mtype = dmsgbuf.msgval.lines[SEND];
#endif

	/*
	 * Now that it's OK to talk, go ahead and
	 * initialize the terminal and screen
	 */
	if (initscr() == (WINDOW *)NULL) {
		fprintf(stderr, "%s: TERM variable not set\n", argv[0]);
		Finish(SIGINT);
	}

	/*
	 * Since curses in now ineffect, we need to watch for
	 * ALL signals so that we can properly terminate
	 */
	for (c = 0; c < NSIG; c++)
		signal(c, Finish);

	cbreak();
	noecho();
#ifndef SCHIZO
#ifndef FIONREAD
	nodelay(stdscr, TRUE);
#endif
#endif
	MIDDLE = LINES / 2 - 1;
	time(&tloc);
	mvprintw(MIDDLE, 0, "--------------------------------------------------------------- %12.12s --", ctime(&tloc) + 4);
	move(orgy, orgx);
	refresh();

	fromtype = dmsgbuf.msgval.lines[RECEIVE];

#ifndef SCHIZO
	/*
	 * If we are given a ctl type, wait for a ctl message.
	 */
	if (dmsgbuf.msgval.lines[CTL] > 0) {
		mvprintw(0, 0, "[Waiting for %s to answer]\n", argv[1]);
		refresh();
		Ring();
		while (msgrcv(msqid, (struct msgbuf *)&dmsgbuf, MSGSIZ, dmsgbuf.msgval.lines[2], 0) == -1) {
			/*
			 * The 'talkee' may have turned off his messages
			 */
			if (avail < 0) {
				mvprintw(orgy++, orgx, "[%s no longer writable]\n", argv[1]);
				refresh();
			}
			/*
			 * EINTR is an interrupted system call
			 */
			else if (errno == EINTR) {
				mvprintw(orgy++, orgx, "[Ringing %s again]\n", argv[1]);
				refresh();
			}
			else {
				fprintf(stderr, "%s: msgrcv failure(%d)\n", argv[0], errno);
				Finish(SIGINT);
			}
		}
		if (orgy) {
			while (orgy > 1) {
				move(--orgy, orgx);
				clrtoeol();
			}
		} else
			orgy = 1;

		alarm(0);	/* Turn off the ringer */
		beep();
		mvaddstr(0, 0, "[Connection established]\n");
		refresh();
	}
	parpid = dmsgbuf.msgval.lines[PIDLOC];
#endif

	/*
	 * This loop just continually checks both users for input.
	 * It actually runs TOO FAST, but since SysV has no fractional
	 * second sleep we do the best we can.
	 */
	resy = MIDDLE + 1;
	for (;;) {
		/*
		 * This is the owner's half
		 */
#ifdef FIONREAD
		ioctl(0, FIONREAD, &waiting);
		if (waiting) {
			c = getch();
#else
		if ((c = getch()) != EOF) {
#endif
			if (c == erasechar()) {
				c = '\b';
				if (--orgx < 0)
					orgx = 0;
				else
					mvaddstr(orgy, orgx, " \b");
			}
			else if (c == '\004')
				Finish(SIGINT);	
			else if (c == '\f') {
				clearok(curscr, TRUE);
				refresh();
				continue;
			}
			else if (c == '\n') {
				orgy = ++orgy % MIDDLE;
				orgx = 0;
				move(orgy, orgx);
				clrtoeol();
				move((orgy + 1) % MIDDLE, orgx);
				clrtoeol();
				move(orgy, orgx);
			}
			/*
			 * Regular characters
			 */
			else {
				/*
				 * Check for wrap around
				 */
				if (orgx >= 79) {
					orgy = ++orgy % MIDDLE;
					orgx = 0;
					move(orgy, orgx);
					clrtoeol();
					move((orgy + 1) % MIDDLE, orgx);
					clrtoeol();
				}
				mvaddch(orgy, orgx++, c);
			}

			refresh();
#ifndef SCHIZO
			*sndbuf.mtext = c;
			if (msgsnd(msqid, (struct msgbuf *)&sndbuf, 1, 0) == -1) {
				fprintf(stderr, "%s: msgsnd failure(%d)\n", argv[0], errno);
				Finish(SIGINT);
			}
#endif
		}

#ifndef SCHIZO
		/*
		 * This is the partner's half
		 */
		if (msgrcv(msqid, (struct msgbuf *)&rcvbuf, 1, fromtype, IPC_NOWAIT) != -1) {
			switch (*rcvbuf.mtext) {
			case '\b':
				if (--resx < 0)
					resx = 0;
				else
					mvaddstr(resy, resx, " \b");
				break;

			case '\n':
				if (++resy >= LINES - 1)
					resy = MIDDLE + 1;
				resx = 0;
				mvaddch(resy, resx, '\n');
				mvaddch(((resy - MIDDLE) % (LINES - MIDDLE - 2)) + MIDDLE + 1, resx, '\n');
				move(resy, resx);
				break;

			default:
				/*
				 * Check for wrap around
				 */
				if (resx >= 79) {
					if (++resy >= LINES - 1)
						resy = MIDDLE + 1;
					resx = 0;
					move(resy, resx);
					clrtoeol();
					move(((resy - MIDDLE) % (LINES - MIDDLE - 2)) + MIDDLE + 1, resx);
					clrtoeol();
					move(resy, resx);
				}
				mvaddch(resy, resx++, *rcvbuf.mtext);
				break;
			}

			refresh();
		}
#endif
		/*
		 * Update the time
		 */
		time(&tloc);
		if (tloc >= otime + 60) {
			otime = tloc;
			mvprintw(MIDDLE, COLS - 16, "%12.12s", ctime(&tloc) + 4);
			refresh();
		}
#ifdef SLEEP
		SLEEP(SLEEP_TIME);
#endif
	}
}

/*
 * Ring - ring the perspective partner
 *
 * Ring();
 *
 * A request message is sent to the partner's terminal.
 */
Ring()
{
	FILE		*fp, *fopen();
	char		*ttyname();
	int		Ring();

	tloc = time((time_t *)0);
	if ((fp = fopen(&dmsgbuf.msgval.mtext[TTYLOC], "w")) != NULL) {
		fprintf(fp, "\r\n%c%cTalk request from %s on %s at %5.5s\r\n",
			'\007', '\011', getlogin(), ttyname(0)+5, ctime(&tloc)+11);
		fprintf(fp, "%cRespond with 'talk %s'\r\n", '\007', getlogin());
		fclose(fp);
	}
	/*
	 * If the person being rung turns of his messages, set avail to
	 * indicate his new unavailablity
	 */
	else 
		avail = -1;

	signal(SIGALRM, Ring);
	alarm((unsigned)20);
}

/*
 * Error - print an error message
 *
 * Error(code);
 *	code	is the availability of who we want to talk to
 *
 * A message regarding why we can't talk is printed.
 */
Error(code, argv)
int code;
char **argv;
{
	switch (code) {

	case NOTLOGGEDON:
		fprintf(stderr, "%s: %s not logged on\n", argv[0], argv[1]);
		break;

	case NOTWRITE:
		fprintf(stderr, "%s: %s not writeable\n", argv[0], argv[1]);
		break;

	case LOGGEDMORE:
		fprintf(stderr, "%s: %s logged on more than once\n", argv[0], argv[1]);
		break;

	case NOTONLINE:
		fprintf(stderr, "%s: %s not on line %s\n", argv[0], argv[1], argv[2]);
		break;
	}
}

/*
 * Finish - reset and exit
 *
 * Finish();
 *
 * Finish is called upon receipt of SIGINT or SIGUSR1. Finish resets the
 * user's terminal and if called by SIGINT, sends SIGUSR1 signal to his
 * partner's talk process so that both terminate simultaineously
 */
Finish(sig)
int sig;
{
	FILE		*fp, *fopen();
	int		i;

	/*
	 * Prevent from being interrupted while finishing
	 */
	for (i = 0; i < NSIG; i++)
		signal(i, SIG_IGN);
#ifndef SCHIZO
	if (sig == SIGINT) {
		/*
		 * If a conversation was in process, tell the partner's
		 * process to stop also. Otherwise, tell the partner
		 * that the request is no longer pending.
		 */
		if (parpid)
			kill(parpid, SIGUSR1);
		else if ((fp = fopen(&dmsgbuf.msgval.mtext[TTYLOC], "w")) != NULL) {
			tloc = time((long *)0);
			fprintf(fp, "\r\n%cTalk request from %s cancelled at %5.5s\r\n", '\011', getlogin(), ctime(&tloc)+11);
			fclose(fp);
		}
	}
#endif

#ifdef CLRONEXIT
	clear();
#endif
	mvaddstr(0, 0, "[Connection closed]\n");
	refresh();
	nodelay(stdscr, FALSE);
	endwin();

#ifndef SCHIZO
	dmsgbuf.msgval.lines[PIDLOC] = -getpid();
	dmsgbuf.mtype = 1;
	if (msgsnd(msqid, (struct msgbuf *)&dmsgbuf, MSGSIZ, 0) == -1) {
		fprintf(stderr, "talk: msgsnd failure(%d)\n", errno);
		exit(1);
	}
#endif

	exit(0);
}
