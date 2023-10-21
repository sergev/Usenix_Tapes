/*
 * talkdemon - the demon for the talk program
 *
 * The talkdemon maintains a linked list of TALK structures, one for each
 * person engaged in a conversation. The demon sits in the background and
 * 'listens' on msgtype 1 for messages from talk programs. Whenever a talk
 * process starts up or exits, it sends a message to the demon. The demon
 * examines the message to see if it is and startup or an exit message.
 * If it is the former, the demon compares the new structure with the existing
 * list and manages to determine a set of send/receive msgtypes. (Each
 * conversation gets its own set) Now, ctl messages. When the first half
 * of a conversation (the originator) starts up, it has to wait for the	
 * other process to start. If the demon determines that a talk process is
 * an originator, in addition to the send/receive pair, the demon sends back
 * a ctltype. The originating talk process waits on this ctltype for a demon
 * message. This is sent when the second half of the conversation starts
 * up (the responder). When either talk process exits, it sends a message
 * to the demon as well as SIGUSR1 to the other process. When the demon
 * receives an exit message, it removes that process's TALK structure
 * from the linked list.
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
#include <stdio.h>
#include <signal.h>
#include "talk.h"
#include <sys/stat.h>

#define	CONSOLE	"/dev/console"	/* Where to write errors if talkd exits */

/*
 * Each conversationalist gets one of these
 */
typedef	struct	talk	{
	char		user[NAMESIZ];		/* The user */
	char		other[NAMESIZ];		/* Who they are talking to */
	char		tty[LINESIZ];		/* The line of the PARTNER */
	long		types;			/* Msgtype index */
	int		pid;	/* pid of the user's talk process (what else?)*/
	struct	talk	*next;
} TALK;

int	msqid;
char	*program, errbuf[BUFSIZ];
void	exit(), free();

main(argc, argv)
int argc;
char **argv;
{
	TALK		*Head, *Addper(), *Delper();
	key_t		msgkey, ftok();
	long		ctl, Link();
	int		i, avail, Finish();

	program = *argv;

	if (fork())
		exit(0);

	setpgrp();
	for (i = 0; i < NSIG; i++)
		signal(i, Finish);

	msgkey = ftok(TALK_PATH, MAGIC_ID);

	if ((msqid = msgget(msgkey, MSGQ_PERMS|IPC_CREAT|IPC_EXCL)) == -1) {
		sprintf(errbuf, "%s: Unable to create talk msgqueue(%d)", program, errno);
		Finish(NSIG);
	}

	Head = (TALK *)NULL;
	/*
	 * The demon sits in this loop, waiting for a message
	 * from a potential user.
	 */
	for (;;) {
#ifdef DEBUG
		fflush(stdout);
#endif
		if (msgrcv(msqid, (struct msgbuf *)&dmsgbuf, MSGSIZ, (long)1, 0) == -1) {
			sprintf(errbuf, "%s: msgrcv failure(%d)", program, errno);
			Finish(NSIG);
		}

		/*
		 * If a message comes in with the pid '0', the talkdemon
		 * will remove its msgqueue and exit. This is needed for
		 * for reboot procedures. Note that process '0' is ALWAYS
		 * the swapper so there is no danger of a user accidently
		 * killing the demon.
		 */
		if (dmsgbuf.msgval.lines[PIDLOC] == 0) {
#ifdef DEBUG
			printf("\nStoptalk message received\n");
#endif
			sprintf(errbuf, "stopped by a command from stoptalk");
			Finish(NSIG);
		}

		/*
		 * A message with a pid of '1' is a request for status
		 * message. Status info is returned on msgtype 3. Note
		 * that process '1' is ALWAYS /etc/init so it is safe to
		 * assume that a message with a pid of '1' is an info
		 * request.
		 */
		if (dmsgbuf.msgval.lines[PIDLOC] == 1) {
#ifdef DEBUG
			printf("\nInfotalk status request message received\n");
#endif
			Status(Head);
			continue;
		}

		/*
		 * When a users exits talk, he sends a removal message to
		 * the demon. Removal messages are identified by a negative
		 * pid.
		 */
		if (dmsgbuf.msgval.lines[PIDLOC] < 0) {
#ifdef DEBUG
			printf("\nRemoval message received\n");
#endif
			Head = Delper(Head, dmsgbuf.msgval.lines[PIDLOC]);
			continue;
		}

#ifdef DEBUG
		printf("\nInitiate message from: %s pid: %d to: %s\n",
			dmsgbuf.msgval.mtext,
			dmsgbuf.msgval.lines[PIDLOC],
			&dmsgbuf.msgval.mtext[NAMESIZ]);
#endif

		/*
		 * Determine the availability of the potential partner.
		 */
		avail = Find(&dmsgbuf.msgval.mtext[8], &dmsgbuf.msgval.mtext[TTYLOC]);
#ifdef DEBUG
		printf("Availability of %s on %s is %d\n", &dmsgbuf.msgval.mtext[8], &dmsgbuf.msgval.mtext[TTYLOC], avail);
#endif
		if (avail == TALKABLE) {
			if ((Head = Addper(Head, &dmsgbuf)) == NULL) {
				sprintf(errbuf, "%s: Unable to add user, malloc failure(%d)", program, errno);
				Finish(NSIG);
			}

			/*
			 * Determine if the new user is an 'originator' or
			 * a 'responder' and assign them send and receive
			 * msgtypes.
			 */
			ctl = Link(Head, &dmsgbuf);

#ifdef DEBUG
			printf("Sending %d %d %d to type 2\n",
				dmsgbuf.msgval.lines[SEND],
				dmsgbuf.msgval.lines[RECEIVE],
				dmsgbuf.msgval.lines[CTL]);
#endif
		}
		/*
		 * We do this if the requested partner is unavailable.
		 */
		else {
			dmsgbuf.msgval.lines[SEND] = avail;
			ctl = -1;
		}

		/*
		 * Send an acknowledgement to the user informing him of the
		 * availability of his partner and the msgtypes to use for
		 * sending and receiving.
		 */
		dmsgbuf.mtype = 2;
		if (msgsnd(msqid, (struct msgbuf *)&dmsgbuf, MSGSIZ, 0) == -1) {
			sprintf(errbuf, "%s: msgsnd failure(%d)", program, errno);
			Finish(NSIG);
		}

		/*
		 * If the most recent message was a 'responder', send
		 * a ctl message to the 'originator' telling him that
		 * his partner has answered. The ctl message includes
		 * the pid of the respondent.
		 */
		if (ctl > 0) {
#ifdef DEBUG
			printf("Sending ctl message to %d\n", ctl);
#endif
			dmsgbuf.mtype = ctl;
			dmsgbuf.msgval.lines[PIDLOC] = Head->pid;
			if (msgsnd(msqid, (struct msgbuf *)&dmsgbuf, MSGSIZ, 0) == -1) {
				sprintf(errbuf, "%s: msgsnd failure(%d)", program, errno);
				Finish(NSIG);
			}
		}
	}
}

/*
 * Finish - clean up
 *
 * Finish();
 *
 * Finish remove the msgqueue and exits. It is normally not called
 * but is here in case the demon encounters an error.
 */
Finish(sig)
int sig;
{
	FILE		*fp, *fopen();
	time_t		tloc, time();
	char		*ctime();
	int		rmv;

	if (sig != NSIG)
		sprintf(errbuf, "caught signal #%d", sig);

	rmv = msgctl(msqid, IPC_RMID, (struct msqid_ds *)NULL);

	if ((fp = fopen(CONSOLE, "w")) != NULL) {
		tloc = time((long *)0);
		fprintf(fp, "talkdemon exiting: %s", ctime(&tloc));
		fprintf(fp, "%s\n", errbuf);
		if (rmv == 0) {
			fprintf(fp, "talk msgqueue removed\n");
#ifdef DEBUG
			printf("%s: msgqueue removed. demon exiting\n", program);
#endif
		}
		else {
			fprintf(fp, "talk msgqueue not removed\n");
#ifdef DEBUG
			printf("%s: msgqueue not removed. demon exiting\n", program);
#endif
		}
	}
	exit(1);
}

/*
 * Status - report the status of the talkdemon
 *
 * Status(Head);
 *	Head	is a pointer to the first TALK structure in the user list
 *
 * Status runs through the current list of talk users and sends out one status
 * msgbuf for each user. All messages are sent to msgtype 3 which is reserved
 * for this use.
 *
 */
Status(Head)
TALK *Head;
{
	dmsgbuf.mtype = STATUS;

	for (; Head; Head = Head->next) {
		strncpy(dmsgbuf.msgval.mtext, Head->user, NAMESIZ);
		strncpy(&dmsgbuf.msgval.mtext[NAMESIZ], Head->other, NAMESIZ);
		strncpy(&dmsgbuf.msgval.mtext[TTYLOC], Head->tty, LINESIZ);
		/*
		 * Do a little bit play to squeeze in some extra infomation.
		 * No entirely kosher, but it will only screw up in already
		 * hazardous situations. (types > 2^16)
		 */
		dmsgbuf.msgval.lines[PIDLOC] = Head->types;
		dmsgbuf.msgval.lines[PIDLOC] <<= 16;
		dmsgbuf.msgval.lines[PIDLOC] += Head->pid;

		if (msgsnd(msqid, (struct msgbuf *)&dmsgbuf, MSGSIZ, 0) == -1) {
			sprintf(errbuf, "%s: msgsnd failure(%d)", program, errno);
			Finish(NSIG);
		}
	}
	/*
	 * A message containing a pid of '0' indicates the end of the status
	 * information.
	 */
	dmsgbuf.msgval.lines[PIDLOC] = 0;
	if (msgsnd(msqid, (struct msgbuf *)&dmsgbuf, MSGSIZ, 0) == -1) {
		sprintf(errbuf, "%s: msgsnd failure(%d)", program, errno);
		Finish(NSIG);
	}
#ifdef DEBUG
	printf("End-of-status message written\n");
#endif
}

/*
 * Link - determine send and receive msgtypes for a TALK structure
 *
 * ctl = Link(Head, bufptr)
 *	ctl	is a ctl type for the TALK structure
 *	Head	is a pointer to the first TALK structure in the list,
 *		which is ALWAYS the most recently added.
 *	bufptr	is a pointer to a DMSG buffer.
 *
 * The newest TALK structure is compared against the rest of the list to
 * determine if the structure belongs to an 'originator' or a 'responder'.
 * If it's the former, a send/receive/ctl msgtype triple is selected, if
 * the latter, a send/receive pair is taken from the matched initiator's
 * structure.
 *
 * If the TALK structure belongs to a 'responder', the ctl msgtype for the
 * corresponding 'originator' is returned, otherwise a zero is returned.
 */
long
Link(Head, bufptr)
TALK *Head;
DMSG *bufptr;
{
	TALK		*ptr;
	int		i = 4;

	/*
	 * See if there is a 'partner' for the new person.
	 * This is done by looking for a TALK structure with the same
	 * names, but reversed.
	 */
	for (ptr = Head->next; ptr; ptr = ptr->next) {
		/*
		 * Perform a sanity check on the list. Occasionally
		 * talk process die without removing their TALK
		 * structures. Dunno why.
		 *
		 * If a TALK structure represents a dead process, remove
		 * it. Notice that the return value of Delper() is ignored.
		 * We can do this because ptr->pid will never be Head->pid.
		 */
		if (kill(ptr->pid, 0) == -1)
			(void) Delper(Head, ptr->pid);

		if (!strncmp(ptr->user, Head->other, NAMESIZ) &&
			!strncmp(ptr->other, Head->user, NAMESIZ)) {
			bufptr->msgval.lines[RECEIVE] = ptr->types;
			bufptr->msgval.lines[SEND] = ptr->types + 1;
			bufptr->msgval.lines[CTL] = 0;
			bufptr->msgval.lines[PIDLOC] = ptr->pid;
			/*
			 * NULL the tty pointers of the structures
			 * to show that they're connected.
			 */
			*Head->tty = NULL;
			*ptr->tty = NULL;
			Head->types = ptr->types;

			return(ptr->types + 2);
		}
	}

	/*
	 * If we got this far, the new person is an 'originator'.
	 *
	 * First, find an unused send/receive/ctl triple.
	 */
	do {
		for (ptr = Head->next; ptr; ptr = ptr->next) {
			if (i == ptr->types) {
				i += 3;
				break;
			}
		}
	} while (ptr);
	Head->types = i;
	bufptr->msgval.lines[SEND] = i;
	bufptr->msgval.lines[RECEIVE] = i + 1;
	bufptr->msgval.lines[CTL] = i + 2;

	return(0);
}

/*
 * Find - locate the partner and determine their availability
 *
 * avail = Find(p1, p2);
 *	avail	is the availability of the requested partner
 *	p1	is a pointer to the partner's name
 *	p2	is a pointer to the name of the partner's tty
 *
 * Find reads UTMP_FILE to get the tty line of the initiator's requested
 * partner. If no tty was given, Find() fills it into p2.
 */
Find(partner, line)
char *partner, *line;
{
	struct	stat	lstat;
	struct	utmp	utmp;
	char		tmpbuf[18];
	int		fd, flag;

	flag = 0;

	if ((fd = open(UTMP_FILE, 0)) >= 0) {
		while (read(fd, (char *)&utmp, sizeof(utmp)) == sizeof(utmp)) {
#ifndef OLDUTMP
			if (utmp.ut_type != USER_PROCESS)
				continue;
#endif

			if (!strncmp(utmp.ut_line, line, LINESIZ) &&
				strncmp(utmp.ut_name, partner, NAMESIZ)) {
				flag = -1;
				break;
			}
			else if (!strncmp(utmp.ut_name, partner, NAMESIZ)) {
				flag++;
				/* if line was speced and found him on this
				   line, make flag one even if he was already
				   found on another line */
				if (*line != NULL &&
				    !strncmp(utmp.ut_line, line, LINESIZ)) {
					flag = 1;
					break;
				}

				/* safe to copy line if not speced since it
				   cant match again */
				if (*line == NULL)
					strcpy(line, utmp.ut_line);
#ifdef DEBUG
				printf("%s found on %s\n", partner, line);
#endif
			}
		}
		close(fd);
	}
	else {
		sprintf(errbuf, "%s: cannot open %s(%d)", program, UTMP_FILE, errno);
		Finish(NSIG);
	}

	if (flag == 0)
		return(NOTLOGGEDON);
	if (flag < 0)
		return(NOTONLINE);
	if (flag > 1)
		return(LOGGEDMORE);

	strcpy(tmpbuf, line);
	strcpy(line, "/dev/");
	strcat(line, tmpbuf);
	stat(line, &lstat);
	if (!(lstat.st_mode & 0002))
		return(NOTWRITE);

	return(TALKABLE);
}

/*
 * Addper - add a new user to the list
 *
 * tp = Addper(hp, mp);
 *	tp	is a pointer to new TALK structure
 *	hp	is a pointer to the Head of the TALK list
 *	mp	is a pointer to the dmsgbuf
 *
 * A new TALK structure is allocated and linked in at the head of the list.
 * Data from the just-received dmsg is copied into the new structure.
 * A pointer to the new structure is returned unless malloc() was unable
 * to allocate space, in which case NULL is returned.
 */
TALK *
Addper(ptr, mptr)
TALK *ptr;
DMSG *mptr;
{
	TALK		*p;
	char		*malloc();

	if ((p = (TALK *)malloc(sizeof(TALK))) == NULL)
		return(NULL);

	p->next = ptr;

	strncpy(p->user, mptr->msgval.mtext, NAMESIZ);
	strncpy(p->other, &mptr->msgval.mtext[NAMESIZ], NAMESIZ);
	strncpy(p->tty, &mptr->msgval.mtext[TTYLOC], LINESIZ);
	p->pid = mptr->msgval.lines[PIDLOC];

	return(p);
}

/*
 * Delper - remove somebody
 *
 * hp = Delper(p, pid);
 *	hp	is a pointer to the (possibly new) head of the list
 *	p	is a pointer to the head of the list
 *	pid	is the pid of the structure to remove
 *
 * The person whose talk pid is given is removed from the list.
 * The, possibly new, head of the list is returned.
 */
TALK *
Delper(Head, pid)
TALK *Head;
int pid;
{
	TALK		*ptr, *lastptr;

	pid = abs(pid);	/* Make sure pid is positive */
	ptr = Head;
	lastptr = (TALK *)NULL;

	while (ptr->pid != pid) {
		lastptr = ptr;
		ptr = ptr->next;

		if (ptr == (TALK *)NULL)	/* Just to be safe */
			return(Head);
	}

	if (lastptr)
		lastptr->next = ptr->next;
	else
		Head = ptr->next;

#ifdef DEBUG
	printf("Removing: %s %d from list\n", ptr->user, pid);
#endif

	free((char *)ptr);

	return(Head);
}
