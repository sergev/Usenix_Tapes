
#ifndef lint
static char	*RcsId = "$Header: pad.c,v 1.1 85/01/08 19:58:45 rick Exp $";
#endif !lint

#include "../condevs.h"
#ifdef PAD

/*
 *	padopn: establish connection through a PAD.
 *	Returns descriptor open to tty for reading and writing.
 *	Negative values (-1...-7) denote errors in connmsg.
 *	Be sure to disconnect tty when done, via HUPCL or stty 0.
 */

char *padparms[] = {
	"set1:0,2:0,3:2,4:1,5:1,6:5,7:4,9:0,10:0,13:0",
	"set14:0,15:0,16:0,17:0,18:0,19:1,20:255",
	"set102:0,103:17,104:19,105:0,106:0,107:0,108:0",
	"set109:0,110:0,111:0,112:0,113:0,114:0,115:0,116:0",
	NULL
};

extern	char	*AbortOn;
int	padcls();
padopn(flds)
register char *flds[];
{
	char phone[MAXPH+1];
	register char **parmptr;
	extern errno;
	char *rindex(), *fdig(), dcname[20];
	int dh, ok = 0, speed;
	register struct condev *cd;
	register FILE *dfp;
	struct Devices dev;

	dfp = fopen(DEVFILE, "r");
	ASSERT(dfp != NULL, "Can't open", DEVFILE, 0);

	signal(SIGALRM, alarmtr);
	dh = -1;
	for(cd = condevs; ((cd->CU_meth != NULL)&&(dh < 0)); cd++) {
		if (snccmp(flds[F_LINE], cd->CU_meth) == SAME) {
			fseek(dfp, (off_t)0, 0);
			while(rddev(dfp, &dev) != FAIL) {
				if (strcmp(flds[F_CLASS], dev.D_class) != SAME)
					continue;
				if (snccmp(flds[F_LINE], dev.D_type) != SAME)
					continue;
				DEBUG(4, "Trying line %s\n", dev.D_line);
				if (mlock(dev.D_line) == FAIL)
					continue;

				sprintf(dcname, "/dev/%s", dev.D_line);
				getnextfd();
				alarm(10);
				if (setjmp(Sjbuf)) {
					delock(dev.D_line);
					logent(dev.D_line,"pad open TIMEOUT");
					dh = -1;
					break;
				}
				dh = open(dcname, 2);
				alarm(0);
				next_fd = -1;
				if (dh > 0) {
					break;
				}
				DEBUG(4, "Can't open line %s\n", dev.D_line);
				devSel[0] = '\0';
				delock(dev.D_line);
			}
		}
	}
	fclose(dfp);
	if (dh < 0)
		return CF_NODEV;
	DEBUG(4, "Using line %s\n", dev.D_line);

	speed = atoi(fdig(flds[F_CLASS]));
	fixline(dh, speed);
	/*	Do we need this?  I don't know	*/
	sleep(1);

	/* Synchronize with PAD prompt */
	write(dh, "\r", 1);
	DEBUG(10, "Pad Sync: wanted %s\n", ">");
	ok = expect(">", dh);
	DEBUG(10, "got %s\n", ok ? "?" : "that");

	if (ok) {
		logent(dev.D_line, "PAD SYNC FAILED");
		close(dh);
		return CF_DIAL;
	}

	/*	Initialization of PAD	*/
	AbortOn = "err";
	for (parmptr = padparms; ok == 0 && *parmptr; parmptr++) {
		DEBUG(10, "PAD setup: %s\n", *parmptr);
		write(dh, *parmptr, strlen(*parmptr));
		write(dh, "\r", 1);
		ok = expect(">", dh);
		DEBUG(4, "setup %s\n", ok? "failed": "worked");
	}
	if (Debug > 10) {
		DEBUG(10, "PAD %s:\n", "configuration");
		write(dh, "par?\r", 6);
		ok = expect(">", dh);
	}
	AbortOn = NULL;		/* dochat(login) does this anyways */
	if (ok) {
		logent(dev.D_line, "PAD SETUP/CONFIG DEBUG FAILED");
		close(dh);
		return CF_DIAL;
	}

	/*	Do chat from L-devices */
	if (dochat(&dev, flds, dh)) {
		logent(dev.D_line, "CHAT FAILED");
		close(dh);
		return CF_DIAL;
	}

	if (ok == 0) {
		exphone(flds[F_PHONE], phone);
		DEBUG(4, "PAD: calling %s\n", phone);
		write(dh, "c ", 2);
		write(dh, phone, strlen(phone));
		write(dh, "\r", 1);
		DEBUG(4, "wanted %s ", "com");
		AbortOn = "clr";
		ok = expect("com", dh);
		DEBUG(4, "got %s\n", ok ? "?" : "that");
		AbortOn = NULL;
	}

	if (ok != 0) {
		if (dh > 2)
			close(dh);
		DEBUG(4, "pad failed\n", "");
		delock(dev.D_line);
		return(CF_DIAL);
	} 
	else
		DEBUG(4, "pad ok\n", "");

	CU_end = padcls;
	strcat(devSel, dev.D_line);	/* for later unlock */
	return dh;
}

padcls(fd)
register int fd;
{

	if (fd > 0) {
		DEBUG(4, "Closing %s\n", "PAD");
		close(fd);
		delock(devSel);
	}
}
#endif MICOM
