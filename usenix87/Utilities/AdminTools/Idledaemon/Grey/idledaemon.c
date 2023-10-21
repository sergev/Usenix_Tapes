/*
 *  	Idledaemon		E. Gray		12 Apr 85
 *  Kicks a user off the system if his idle time has exceeded
 *  a specified number of minutes.  Only applies to dialup
 *  lines, and certain users are exempt.
 */

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

main(argc, argv)
int argc;
char *argv[];
{
	FILE *fp, *sp, *popen(); 
	int uid, pid, gid, maxidle, tty, line, min;
	struct stat buff;
	struct passwd *pwline, *getpwuid();
	long last, diff, now;
	char ttyname[20], pipe[20], buf[BUFSIZ];
	if (argc <= 1) {
		fprintf(stderr,"Idledaemon : requires a time argument\n");
		exit(1);
	}
	min = atoi(argv[1]);
	maxidle = min * 60;

	nice(10);
					/* turn off interrrupts */
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
					/* put it in the background */
	if (fork())
		exit(0);

	while(1) {
		sleep(60);
					/* dialups are tty8 -> tty11 */
		for (tty=8; tty<=11; tty++) {
			sprintf(ttyname, "/dev/tty%d", tty);
			if (stat(ttyname, &buff) < 0) {
				fprintf(stderr, "Idledaemon: stat failed\n");
				continue;
			}
			time(&now);
					/* Is the time of last I/O */
					/* but NOT process inactivity */
			last = buff.st_atime;
			diff = now - last;
			if (diff < 0)
				diff = 0;
			uid = buff.st_uid;
			gid = buff.st_gid;
					/* exclude non-humans */
			if (uid < 100)
				continue;
					/* exclude special groups */
			if (gid < 100)
				continue;
			if (diff < maxidle)
				continue;
			sprintf(pipe, "ps -ot%d", tty);
				/* lazy man's way to get pid */
			if ((sp = popen(pipe, "r")) == NULL) {
				fprintf(stderr, "Idledaemon : Can't open ps\n");
				continue;
			}
				/* strip off the header line */
			fgets(buf, BUFSIZ, sp);
			line = 0;
			while (fgets(buf, BUFSIZ, sp) != NULL) {
				line++;
				sscanf(buf, "%d", &pid);
			}
			pclose(sp);
				/* has silent process ? */
			if (line > 1)
				continue;
			if (!(fp = fopen(ttyname, "w"))) {
				fprintf(stderr,"Idledaemon : can't open %s\n", ttyname);
				continue;
			}
			fprintf(fp, "\n\n******************************************************************************\n");
			fprintf(fp, "Message from Idledaemon at %s", ctime(&now));
			fprintf(fp, "Your connection is being dropped due to inactivity in excess of %d minutes\n", min);
			fprintf(fp, "******************************************************************************\n");
			fprintf(fp, "\n\007\007\007");
			fclose(fp);
				/* send SIGKILL to tty */
			kill(pid, 9);
				/* find login name */
			pwline = getpwuid(uid);
				/* mail him a nasty note */
			sprintf(pipe, "mail %s", pwline->pw_name);
			if ((sp = popen(pipe, "w")) == NULL) {
				fprintf(stderr, "Idledaemon : can't open mail\n");
				continue;
			}
			fprintf(sp, "\nSubject : Inactive terminal\n\n");
			fprintf(sp, "Your terminal was found to be inactive in excess of %d minutes at the\n", min);
			fprintf(sp, "above date and time.\n\n");
			fprintf(sp, "Please use your ADP resources wisely and consistent with security\n");
			fprintf(sp, "regulations.\n\n");
			fprintf(sp, "Thank you\n");
			fprintf(sp, "Emmet P Gray\n");
			pclose(sp);
		}
	}
}
