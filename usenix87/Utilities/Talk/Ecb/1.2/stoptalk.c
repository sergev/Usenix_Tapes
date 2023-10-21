/*
 * stoptalk - shut down the talk facility
 *
 * SYNOPSIS
 *	stoptalk
 *
 * Stoptalk is used to stop the talk facility. Prior to a reboot the
 * current talk msgqueue needs to be removed manually because the
 * reboot procedure does not clear system msgqueues. The talkdemon
 * is incapable of removing its own msgqueue automatically because
 * /etc/kill stops process with SIGKILL which cannot be caught.
 *
 * Stoptalk sends a message to the talkdemon instructing it to exit.
 * If this fails, stoptalk tries to remove the msgsqueue itself.
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
#include "talk.h"

main(argc, argv)
int argc;
char **argv;
{
	key_t		msgkey, ftok();
	void		exit();
	int		msqid;

	msgkey = ftok(TALK_PATH, MAGIC_ID);

	if ((msqid = msgget(msgkey, MSGQ_PERMS)) == -1) {
		fprintf(stderr, "%s: Nonexistant talk msgqueue\n", *argv);
		exit(1);
	}

	/*
	 * Set up a msg containing the pid '0'. This will signal
	 * the talkdemon to exit gracefully.
	 */
	dmsgbuf.msgval.lines[PIDLOC] = 0;
	dmsgbuf.mtype = 1;

	/*
	 * Send the 'kill' msg to the demon.
	 */
	if (msgsnd(msqid, (struct msgbuf *)&dmsgbuf, MSGSIZ, IPC_NOWAIT) == 0) {
		/*
		 * Wait while the demon prints its exit message
		 */
		sleep(2);
		exit(0);
	}
	/*
	 * The graceful method didn't work, get brutal
	 */
	fprintf(stderr, "%s: msgsnd failure(%d)\n", *argv, errno);
	if (msgctl(msqid, IPC_RMID, (struct msqid_ds *)NULL) == 0) {
		fprintf(stderr, "%s: Talk msgqueue removed\n", *argv);
		exit(0);
	}

	fprintf(stderr, "%s: Unable to remove talk msgqueue\n", *argv);
	exit(1);
}
