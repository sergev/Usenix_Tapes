#include "ims.h"

char *mdate();

char sender[1024] = "/bin/mail";
char editor[1024] = "/usr/ucb/vi";
char edforward[256] = "No";
char alicount[256] = "20";

static char *_month_[] = {
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December",
};

static char *_weekday_[] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
};

sendmail(fd, to, cc, subj, bcc)
FILE *fd;
char *to, *cc, *subj, *bcc; {
	char *cp;
	FILE *rmail;
	char name[512];
	static char toline[512], ccline[512];
	struct passwd *curuser;
	
	if (fd == (FILE *) 0)
		return 0;
	if ((curuser = getpwnam(getlogin())) == (struct passwd *) 0) {
		puts("Who are you, anyway?  I can't send mail from nobody!");
		return -1;
	}
	strcpy(name, curuser->pw_gecos);
	if ((cp = strchr(name, ',')) != (char *) 0)
		*cp = '\0';
	strcpy(toline, to);
	strcpy(ccline, cc);
	while (*to == ' ' || *to == '\t')
		to++;
	if (*to == '\0') {
		puts("Empty To: list.");
		return 0;
	}
	sendlist(to, name, toline, ccline, subj, fd, 0);
	sendlist(cc, name, toline, ccline, subj, fd, 0);
	sendlist(bcc, name, toline, ccline, subj, fd, 0);
	return 1;
}

sendlist(list, from, toline, ccline, subj, fd, acount)
char *list, *from, *toline, *ccline, *subj;
FILE *fd; {
	char *cp;
	char aexp[1024];

	if (atoi(alicount) < 0)
		strcpy(alicount, "20");
	if (acount > atoi(alicount)) {
		puts("Alias loop.");
		return -1;
	}
	while (*list != '\0') {
		while (*list == ' ' || *list == '\t')
			list++;
		for (cp = list; *cp != ',' && *cp != '\0' && *cp != ' ' && *cp != '\t'; cp++)
			;
		if (*cp != '\0')
			*cp++ = '\0';
		if (*list == '\0')
			break;
		if (xalias(list, aexp))
			return sendlist(aexp, from, toline, ccline, subj, fd, acount + 1);
		smtp(from, list, toline, ccline, subj, fd);
		list = cp;
	}
	return 1;
}

smtp(from, to, toline, ccline, subjline, fp)
char *from, *to, *toline, *ccline, *subjline;
FILE *fp; {
	char cmd[256];
	char ch;
	FILE *rmail;

	sprintf(cmd, "exec %s %s", sender, to);
	if ((rmail = epopen(cmd, "w")) == (FILE *) 0) {
		puts("Can't run mailer.");
		return 0;
	}
	fprintf(rmail, "Date: %s\nFrom: %s  <%s>\nSubject: %s\nTo: %s\n%s%s\n", mdate(), from, getlogin(), subjline, toline, (ccline[0] == '\0'? "": "Cc: "), ccline, (ccline[0] == '\0'? "": "\n"));
	rewind(fp);
	while ((ch = getc(fp)) != EOF)
		putc(ch, rmail);
	if (pclose(rmail) != 0)
		printf("Mailer failed to %s.\n", to);
}

reply(cmdp)
char *cmdp; {
	char *cp;
	char curbuf[256], to[256], cc[256], subj[256], bcc[256], tmpf[80], mline[5120];
	FILE *fp, *mfp;

	while (*cmdp == ' ' || *cmdp == '\t')
		cmdp++;
	if (*cmdp != '\0') {
		for (cp = cmdp; *cp != ' ' && *cp != '\t' && *cp != '\0'; cp++)
			;
		if (*cp != '\0')
			*cp++ = '\0';
		while (*cp == ' ' || *cp == '\t')
			cp++;
		if (*cp != '\0') {
			puts("Wrong number of arguments.  Usage:  reply [message-number]");
			return -1;
		}
		if (!ismsg(cmdp)) {
			puts("No such message.");
			return -1;
		}
		if (isdeleted(cmdp)) {
			sprintf(curbuf, ".%s", cmdp);
			cmdp = curbuf;
		}
	}
	else {
		sprintf(curbuf, "%d", msg);
		if (isdeleted(curbuf))
			sprintf(curbuf, ".%d", msg);
		cmdp = curbuf;
	}
	bcc[0] = '\0';
	gettcs(cmdp, to, cc, subj);
	edenv(to, cc, subj, bcc);
	sprintf(tmpf, "/tmp/ms%05dr", getpid());
	if ((fp = efopen(tmpf, "w")) == (FILE *) 0) {
		puts("Can't open message temp file.");
		return -1;
	}
	if ((mfp = efopen(mlocation(cmdp), "r")) == (FILE *) 0) {
		perror(mlocation(cmdp));
		puts("Can't open original message.");
		fputs("> Couldn't copy the original message.\n", fp);
		fclose(fp);
	}
	else {
		fputs("+---------------\n", fp);
		while (fgets(mline, sizeof mline, mfp) != (char *) 0) {
			fputs("| ", fp);
			fputs(mline, fp);
		}
		fputs("+---------------\n\n\n", fp);
		fclose(fp);
		fclose(mfp);
	}
	enter(tmpf);
	if (!suresend()) {
		unlink(tmpf);
		puts("Reply aborted.");
		return 1;
	}
	puts("Please check the envelope below and make any necessary changes.");
	if (!edenv(to, cc, subj, bcc))
		return -1;
	if ((fp = efopen(tmpf, "r")) == (FILE *) 0) {
		puts("Can't open reply file.");
		return -1;
	}
	if (!sendmail(fp, to, cc, subj, bcc)) {
		fclose(fp);
		dead(tmpf);
	}
	fclose(fp);
	unlink(tmpf);
	return 1;
}

mailto(cmdp)
char *cmdp; {
	char *cp;
	char curbuf[256], to[256], cc[256], subj[256], bcc[256], tmpf[80];
	FILE *fp;

	while (*cmdp == ' ' || *cmdp == '\t')
		cmdp++;
	if (*cmdp != '\0') {
		for (cp = cmdp; *cp != ' ' && *cp != '\t' && *cp != '\0'; cp++)
			;
		if (*cp != '\0')
			*cp++ = '\0';
		while (*cp == ' ' || *cp == '\t')
			cp++;
		if (*cp != '\0') {
			puts("Wrong number of arguments.  Usage:  mail [user-name]");
			return -1;
		}
	}
	bcc[0] = '\0';
	strcpy(to, cmdp);
	subj[0] = '\0';
	cc[0] = '\0';
	edenv(to, cc, subj, bcc);
	sprintf(tmpf, "/tmp/ms%05ds", getpid());
	if ((fp = efopen(tmpf, "w")) == (FILE *) 0) {
		puts("Can't open message temp file.");
		return -1;
	}
	fclose(fp);
	enter(tmpf);
	if (!suresend()) {
		unlink(tmpf);
		puts("Mail aborted.");
		return 1;
	}
	puts("Please check the envelope below and make any necessary changes.");
	if (!edenv(to, cc, subj, bcc))
		return -1;
	if ((fp = efopen(tmpf, "r")) == (FILE *) 0) {
		puts("Can't open message temp file.");
		return -1;
	}
	if (!sendmail(fp, to, cc, subj, bcc)) {
		fclose(fp);
		dead(tmpf);
	}
	fclose(fp);
	unlink(tmpf);
	return 1;
}

forward(cmdp)
char *cmdp; {
	char *cp;
	char curbuf[256], to[256], cc[256], subj[256], bcc[256], tmpf[80];
	static char mline[5120];
	FILE *fp, *mfp;

	while (*cmdp == ' ' || *cmdp == '\t')
		cmdp++;
	if (*cmdp == '\0') {
		puts("Wrong number of arguments.  Usage:  forward user-name [message-number]");
		return -1;
	}
	for (cp = cmdp; *cp != ' ' && *cp != '\t' && *cp != '\0'; cp++)
		;
	if (*cp != '\0')
		*cp++ = '\0';
	strcpy(to, cmdp);
	cmdp = cp;
	if (*cmdp != '\0') {
		for (cp = cmdp; *cp != ' ' && *cp != '\t' && *cp != '\0'; cp++)
			;
		if (*cp != '\0')
			*cp++ = '\0';
		while (*cp == ' ' || *cp == '\t')
			cp++;
		if (*cp != '\0') {
			puts("Wrong number of arguments.  Usage:  forward user-name [message-number]");
			return -1;
		}
		if (!ismsg(cmdp)) {
			puts("No such message.");
			return -1;
		}
		if (isdeleted(cmdp)) {
			sprintf(curbuf, ".%s", cmdp);
			cmdp = curbuf;
		}
	}
	else {
		sprintf(curbuf, "%d", msg);
		if (isdeleted(curbuf))
			sprintf(curbuf, ".%d", msg);
		cmdp = curbuf;
	}
	gettcs(cmdp, (char *) 0, (char *) 0, subj);
	if (subj[0] == '\0')
		sprintf(subj, "Mail forwarded from \"%s\"", getlogin());
	bcc[0] = '\0';
	cc[0] = '\0';
	edenv(to, cc, subj, bcc);
	sprintf(tmpf, "/tmp/ms%05df", getpid());
	if ((fp = efopen(tmpf, "w")) == (FILE *) 0) {
		puts("Can't open message temp file.");
		return -1;
	}
	if ((mfp = efopen(mlocation(cmdp), "r")) == (FILE *) 0) {
		perror(mlocation(cmdp));
		puts("Can't open original message.");
		fclose(fp);
		unlink(tmpf);
		return -1;
	}
	else {
		while (fgets(mline, sizeof mline, mfp) != (char *) 0)
			fputs(mline, fp);
		fclose(fp);
		fclose(mfp);
	}
	if ((edforward[0] == 'Y' || edforward[0] == 'y' || edforward[0] == 'T' || edforward[0] == 't') && isatty(0))
		enter(tmpf);
	if (!suresend()) {
		unlink(tmpf);
		puts("Forward aborted.");
		return 1;
	}
	if (edforward[0] == 'Y' || edforward[0] == 'y' || edforward[0] == 'T' || edforward[0] == 't')
		edenv(to, cc, subj, bcc);
	if ((fp = efopen(tmpf, "r")) == (FILE *) 0) {
		puts("Can't open message temp file.");
		return -1;
	}
	if (!sendmail(fp, to, cc, subj, bcc)) {
		fclose(fp);
		dead(tmpf);
	}
	fclose(fp);
	unlink(tmpf);
	return 1;
}

enter(file)
char *file; {
	int (*intr)(), (*quit)();
	int pid, status;

	if (!isatty(0)) {
		FILE *fp;
		char iline[1024];
		
		if ((fp = efopen(file, "w")) == (FILE *) 0) {
			printf("Can't open file \"%s\".\n", file);
			return;
		}
		while (gets(iline) != (char *) 0)
			fprintf(fp, "%s\n", iline);
		fclose(fp);
		return;
	}
	switch (pid = fork()) {
		case -1:
			puts("Couldn't fork.");
			break;
		case 0:
			setgid(getgid());
			setuid(getuid());
			execlp(editor, editor, file, (char *) 0);
			perror(editor);
			exit(0);
		default:
			intr = signal(SIGINT, SIG_IGN);
			quit = signal(SIGQUIT, SIG_IGN);
			while (wait(&status) != pid)
				;
			signal(SIGINT, intr);
			signal(SIGQUIT, quit);
			if (status != 0)
				puts("Editor failure detected.");
	}
}

dead(file)
char *file; {
/*
 * Usually, the real mailer (rmail) will deal with dead.letter files.  If
 * yours doesn't, change this function.
 */
}

edenv(to, cc, subj, bcc)
char *to, *subj, *cc, *bcc; {
	printf("To: ");
	edstr(to);
	printf("Subject: ");
	edstr(subj);
	printf("Cc: ");
	edstr(cc);
	printf("Bcc: ");
	edstr(bcc);
}

edstr(str)
char *str; {
	char ch;
	char *sp;
	struct TERMIO ottyp, nttyp;
	int (*intr)(), (*quit)();
	
	if (ioctl(0, TIO_GET, &ottyp) < 0)
		return;
	intr = signal(SIGINT, SIG_IGN);
	quit = signal(SIGQUIT, SIG_IGN);
	nttyp = ottyp;
#ifdef USG
	nttyp.c_lflag &= ~(ICANON|ECHO|ECHOE);
	nttyp.c_cc[VMIN] = 1;
	nttyp.c_cc[VTIME] = 0;
#else  USG
	nttyp.sg_flags |= CBREAK;
	nttyp.sg_flags &= ~ECHO;
#endif USG
	ioctl(0, TIO_SET, &nttyp);
	for (sp = str; *sp != '\0'; sp++)
		putchar(*sp);
	for (;;) {
		if ((ch = (getchar() & 0x7f)) == '\n')
			break;
		if (ch == nttyp.TIO_ERASE) {
			if (sp == str) {
				putchar('\7');
				continue;
			}
			putchar('\b');
			putchar(' ');
			putchar('\b');
			sp--;
			continue;
		}
		if (ch < ' ') {
			putchar('\7');
			continue;
		}
		putchar(ch);
		*sp++ = ch;
	}
	ioctl(0, TIO_SET, &ottyp);
	putchar('\n');
	*sp = '\0';
	signal(SIGINT, intr);
	signal(SIGQUIT, quit);
}

gettcs(msg, to, cc, subj)
char *msg, *to, *cc, *subj; {
	FILE *fp;
	char mline[5120];
	char *cp;
	
	if ((fp = efopen(mlocation(msg), "r")) == (FILE *) 0) {
		puts("Can't read envelope of message.");
		return;
	}
	if (to != (char *) 0)
		to[0] = '\0';
	if (cc != (char *) 0)
		cc[0] = '\0';
	if (subj != (char *) 0)
		subj[0] = '\0';
	while (fgets(mline, sizeof mline, fp) != (char *) 0) {
		if (mline[0] == '\n' || mline[0] == ' ')
			break;
		if ((cp = strrchr(mline, '\n')) != (char *) 0)
			*cp = '\0';
		if (to != (char *) 0 && strncmp(mline, "Path: ", 6) == 0) {
			if (to[0] != '\0')
				strcat(to, ", ");
			strcat(to, &mline[6]);
			continue;
		}
		if (to != (char *) 0 && strncmp(mline, "To: ", 4) == 0) {
			if (to[0] != '\0')
				strcat(to, ", ");
			strcat(to, &mline[4]);
			continue;
		}
		if (subj != (char *) 0 && strncmp(mline, "Subject: ", 9) == 0) {
			strcpy(subj, &mline[9]);
			continue;
		}
		if (cc != (char *) 0 && strncmp(mline, "Cc: ", 4) == 0) {
			if (cc[0] != '\0')
				strcat(cc, ", ");
			strcat(cc, &mline[4]);
			continue;
		}
	}
	fclose(fp);
}

char *mdate() {
	long now;
	struct tm *today;
	static char dbuf[80];
	int hr;
	char ap;
#ifdef USG
	extern int daylight;
	extern char *tzname[];
#endif USG
	
	time(&now);
	today = localtime(&now);
	if (today->tm_hour < 12) {
		ap = 'A';
		hr = today->tm_hour;
	}
	else {
		ap = 'P';
		hr = today->tm_hour - 12;
	}
	if (hr == 0)
		hr = 12;
	sprintf(dbuf, "%s, %s %d, 19%d at %d:%02d %c.M.", _weekday_[today->tm_wday], _month_[today->tm_mon], today->tm_mday, today->tm_year, hr, today->tm_min, ap);
#ifdef USG
	tzset();
	strcat(dbuf, " ");
	strcat(dbuf, tzname[daylight]);
#endif USG
	return dbuf;
}

suresend() {
	char ch;
	struct TERMIO ottyp, nttyp;
	int (*intr)(), (*quit)();
	
	if (ioctl(0, TIO_GET, &ottyp) < 0)
		return 1;
	intr = signal(SIGINT, SIG_IGN);
	quit = signal(SIGQUIT, SIG_IGN);
	nttyp = ottyp;
#ifdef USG
	nttyp.c_lflag &= ~(ICANON|ECHO|ECHOE);
	nttyp.c_cc[VMIN] = 1;
	nttyp.c_cc[VTIME] = 0;
#else  USG
	nttyp.sg_flags |= CBREAK;
	nttyp.sg_flags &= ~ECHO;
#endif USG
	ioctl(0, TIO_SET, &nttyp);
	printf("Are you sure you want to send this (Y)? ");
	while ((ch = getchar()) == EOF || (ch < ' ' && ch != '\r' && ch != '\n' && ch != '\t') || ch > '~')
		;
	if (ch == 'N' || ch == 'n' || ch == '\t')
		puts("No");
	else
		puts("Yes");
	ioctl(0, TIO_SET, &ottyp);
	signal(SIGINT, intr);
	signal(SIGQUIT, quit);
	return ch != 'N' && ch != 'n' && ch != '\t';
}

getcont() {
	struct TERMIO ottyp, nttyp;
	int (*intr)(), (*quit)();
	
	intr = signal(SIGINT, SIG_IGN);
	quit = signal(SIGQUIT, SIG_IGN);
	if (ioctl(0, TIO_GET, &ottyp) < 0)
		return;
	nttyp = ottyp;
#ifdef USG
	nttyp.c_lflag &= ~(ICANON|ECHO|ECHOE);
	nttyp.c_cc[VMIN] = 1;
	nttyp.c_cc[VTIME] = 0;
#else  USG
	nttyp.sg_flags |= CBREAK;
	nttyp.sg_flags &= ~ECHO;
#endif USG
	ioctl(0, TIO_SET, &nttyp);
	printf("--- Press any key to continue ---");
	getchar();
	printf("\r                                 \r");
	ioctl(0, TIO_SET, &ottyp);
	signal(SIGINT, intr);
	signal(SIGQUIT, quit);
}
