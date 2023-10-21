#include "ims.h"

readmbox(cmdp)
char *cmdp; {
	char *cp;
	int cnt;

	for (; *cmdp == ' ' || *cmdp == '\t'; cmdp++)
		;
	if (*cmdp != '\0') {
		for (cp = cmdp; *cp != '\0' && *cp != ' ' && *cp != '\t'; cp++)
			;
		if (*cp != '\0')
			*cp++ = '\0';
		while (*cp != '\0')
			if (*cp != '\t' && *cp != ' ') {
				puts("Too many arguments.  Usage: read [mailbox-name]");
				return -1;
			}
	}
	else
		cmdp = mailbox;
	if ((cnt = receive(cmdp)) == -1) {
		puts("No mailbox.");
		return -1;
	}
	return 1;
}

receive(mbox)
char *mbox; {
	FILE *mbp, *mp;
	char line[1024], from[512], subj[512], date[512], msgn[40], path[1024];
	static char cc[10240], to[10240];
	char *cp;
	int first, next, cnt;
	enum {
		Initial,
		HLine1,
		Header,
		Body,
	} state;

	next = 0;
	do
		sprintf(msgn, "%d", ++next);
	while (access(mlocation(msgn), 0) == 0);
	if (issysmbox(mbox))
		if (lockmbox(1) < 0) {
			puts("Mailbox is locked.  Try again in a few minutes.");
			return 0;
		}
	if ((mbp = efopen(mbox, "r")) == (FILE *) 0) {
		printf("Can't open mailbox: %s\n", mbox);
		if (issysmbox(mbox))
			lockmbox(0);
		return 0;
	}
	first = next;
	state = Initial;
	cnt = 0;
	while (fgets(line, sizeof line, mbp) != (char *) 0) {
		if (strncmp(line, "From ", 5) == 0) {
			if (state != Initial) {
				fclose(mp);
				do
					sprintf(msgn, "%d", ++next);
				while (ismsg(msgn));
			}
			if ((mp = efopen(mlocation(msgn), "w")) == (FILE *) 0) {
				printf("Error:  can't create new message %d.  Mailbox restored.\n", next);
				fclose(mbp);
				if (issysmbox(mbox))
					lockmbox(0);
				return 0;
			}
			cnt++;
			strcpy(to, "");
			strcpy(from, "");
			strcpy(path, "Path: ");
			strcpy(cc, "");
			strcpy(subj, "");
			strcpy(date, "Date: ");
			for (cp = line + 5; *cp != ' '; cp++)
				;
			*cp++ = '\0';
			strcat(path, line + 5);
			strcat(path, "\n");
			while (*cp == ' ')
				cp++;
			strcat(date, cp);
			state = HLine1;
			continue;
		}
		if (state == HLine1) {
			state = Body;
			if (strncmp(line, ">From ", 6) == 0 && rmtinc(path, line)) {
				state = HLine1;
				continue;
			}
			for (cp = line; *cp != '\0'; cp++)
				if (strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz01235456789_-", *cp) == (char *) 0)
					break;
			if (*cp == ':' && *(cp + 1) == ' ')
				state = Header;
			else {
				if (from[0] == '\0') {
					strcpy(from, "From: ");
					strcat(from, &path[6]);
				}
				fputs(date, mp);
				fputs(path, mp);
				fputs(from, mp);
				fputs(subj, mp);
				fputs(to, mp);
				fputs(cc, mp);
				putc('\n', mp);
				if (cnt == 1)
					printf("New mail in folder \"%s\":\n", folder);
				printf("  %5d   %-20.*s", next, (strlen(from) > 27? 20: strlen(from) - 7), &from[6]);
				if (subj[0] != '\0')
					printf(" \"%.*s\"", (subj[0] == '\0'? 0: (strlen(subj) < 50? strlen(subj) - 10: 40)), &subj[9]);
				putchar('\n');
			}
		}
		if (state == Header && line[0] == '\n') {
			if (from[0] == '\0') {
				strcpy(from, "From: ");
				strcat(from, &path[6]);
			}
			fputs(date, mp);
			fputs(path, mp);
			fputs(from, mp);
			fputs(subj, mp);
			fputs(to, mp);
			fputs(cc, mp);
			if (cnt == 1)
				printf("New mail in folder \"%s\":\n", folder);
			printf("  %5d   %-20.*s", next, (strlen(from) > 27? 20: strlen(from) - 7), &from[6]);
			if (subj[0] != '\0')
				printf(" \"%.*s\"", (subj[0] == '\0'? 0: (strlen(subj) < 50? strlen(subj) - 10: 40)), &subj[9]);
			putchar('\n');
			state = Body;
		}
		if (state == Body) {	
			fputs(line, mp);
			continue;
		}
		if (strncmp(line, "Date: ", 6) == 0) {
			strcpy(date, line);
			continue;
		}
		if (strncmp(line, "From: ", 6) == 0) {
			strcpy(from, line);
			continue;
		}
		if (strncmp(line, "Subject: ", 9) == 0) {
			strcpy(subj, line);
			continue;
		}
		if (strncmp(line, "To: ", 4) == 0) {
			strcat(to, line);
			continue;
		}
		if (strncmp(line, "Cc: ", 4) == 0) {
			strcat(cc, line);
			continue;
		}
	}
	if (state != Initial)
		fclose(mp);
	fclose(mbp);
	if (!issysmbox(mbox)) {
		if (unlink(mbox) < 0)
			puts("Couldn't delete the mailbox.");
	}
	else {
		if ((mbp = efopen(mbox, "w")) == (FILE *) 0)
			puts("Couldn't empty your mailbox; check the ownership.");
		else
			fclose(mbp);
		if (lockmbox(0) < 0)
			puts("Can't unlock your mailbox (please inform a system administrator).");
	}
	if (cnt == 0) {
		puts("No new messages.");
		return 0;
	}
	printf("Received %d new message%s into folder.\n", cnt, (cnt == 1? "": "s"));
	return first;
}

lockmbox(flag) {
	char lockname[1024];
	struct stat sbuf;
	long now;
	int lfd;
	
	strcpy(lockname, sysmbox);
	strcat(lockname, ".lock");
	if (flag == 0)
		return unlink(lockname);
	if (stat(lockname, &sbuf) == 0) {
		time(&now);
		if (now - sbuf.st_ctime < 300)
			return -1;
		else if (unlink(lockname) < 0) {
			puts("Can't unlock your mailbox (fatal error).");
			exit(1);
		}
	}
	if ((lfd = creat(lockname, 0600)) < 0) {
		puts("Can't lock your mailbox (fatal error).");
		exit(1);
	}
	close(lfd);
	return 0;
}

newmail(f)
char *f; {
	struct stat sbuf;
	
	if (stat(f, &sbuf) < 0)
		return 0;
	return sbuf.st_size > 0L;
}

/*
 * line contains ``>From user ...''
 * if after date comes ``remote from ...'' then generate new path from it
 */

rmtinc(path, line)
char *path, *line; {
	char *cp, *rpath;
	
	if (line[strlen(line) - 1] == '\n')
		line[strlen(line) - 1] = '\0';
	for (cp = line + strlen(line) - 1; *cp == ' '; cp--)
		if (cp == line)
			return 0;
	*(cp + 1) = '\0';
	while (*cp != ' ')
		if (--cp == line)
			return 0;
	rpath = cp + 1;
	while (*cp == ' ')
		if (--cp == line)
			return 0;
	*(cp + 1) = '\0';
	while (*cp != ' ')
		if (--cp == line)
			return 0;
	if (strcmp(cp + 1, "from") != 0)
		return 0;
	while (*cp == ' ')
		if (--cp == line)
			return 0;
	*(cp + 1) = '\0';
	while (*cp != ' ')
		if (--cp == line)
			return 0;
	if (strcmp(cp + 1, "remote") != 0)
		return 0;
	cp = line + 6;
	while (*cp != ' ')
		cp++;
	*cp = '\0';
	sprintf(path, "Path: %s!%s\n", rpath, line + 6);
	return 1;
}
