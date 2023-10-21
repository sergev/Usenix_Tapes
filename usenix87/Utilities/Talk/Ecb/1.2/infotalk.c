/*
 * infotalk - provide current status of the talk system
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

int	msqid;

main(argc, argv)
int argc;
char *argv[];
{
	key_t		msgkey, ftok();
	void		exit();
	int		busy;

	msgkey = ftok(TALK_PATH, MAGIC_ID);

	if ((msqid = msgget(msgkey, MSGQ_PERMS)) == -1) {
		fprintf(stderr, "%s: Nonexistant talk msgqueue\n", argv[0]);
		exit(1);
	}

	dmsgbuf.msgval.lines[PIDLOC] = 1;
	dmsgbuf.mtype = 1;

	/*
	 * Send the request to the demon
	 */
	if (msgsnd(msqid, (struct msgbuf *)&dmsgbuf, MSGSIZ, 0) == -1) {
		fprintf(stderr, "%s: msgsnd failure(%d)\n", argv[0], errno);
		exit(1);
	}
#ifdef DEBUG
	printf("Info request sent to talkdemon\n");
#endif

	printf("Talker      Talkee      TTY             PID    MTYPE\n");
	do {
		if (msgrcv(msqid, (struct msgbuf *)&dmsgbuf, MSGSIZ, (long)STATUS, 0) == -1) {
			fprintf(stderr, "%s: msgrcv failure(%d)\n", argv[0], errno);
			exit(1);
		}
#ifdef DEBUG
	printf("Info message received from talkdemon\n");
#endif
		if (dmsgbuf.msgval.lines[PIDLOC]) {
			printf("%-12s", dmsgbuf.msgval.mtext);
			printf("%-12s", &dmsgbuf.msgval.mtext[NAMESIZ]);
			printf("%-16s", &dmsgbuf.msgval.mtext[TTYLOC]);
			printf("%5d", dmsgbuf.msgval.lines[PIDLOC] & 0177777);
			printf("  %5d\n", dmsgbuf.msgval.lines[PIDLOC] >> 16);
			busy++;
		}
	} while (dmsgbuf.msgval.lines[PIDLOC]);
	if (!busy)
		printf("No one is currently using talk\n");
}
