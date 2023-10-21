#include "ims.h"

char pager[1024] = "/usr/ucb/more";
char printcmd[1024];
char savefolder[256] = "saved-mail";
char askappend[] = "Yes";

nextmsg(cmdp)
char *cmdp; {
	for (; *cmdp == ' ' || *cmdp == '\t'; cmdp++)
		;
	if (*cmdp != '\0') {
		puts("Usage:  next");
		return -1;
	}
	msg++;
	if (autonext[0] == 'y' || autonext[0] == 'Y' || autonext[0] == 't' || autonext[0] == 'T')
		if (readmsg("-q") < 0) {
			puts("No next message.");
			return -1;
		}
	else if (!ismsg(msg)) {
		puts("No next message.");
		return -1;
	}
	return 1;
}

prevmsg(cmdp)
char *cmdp; {
	for (; *cmdp == ' ' || *cmdp == '\t'; cmdp++)
		;
	if (*cmdp != '\0') {
		puts("Usage:  previous");
		return -1;
	}
	msg--;
	if (autonext[0] == 'y' || autonext[0] == 'Y' || autonext[0] == 't' || autonext[0] == 'T')
		if (readmsg("-q") < 0) {
			puts("No next message.");
			return -1;
		}
	else if (!ismsg(msg)) {
		puts("No next message.");
		return -1;
	}
	return 1;
}

readmsg(cmdp)
char *cmdp; {
	char *cp;
	int foldmsg, quiet;
	char mname[80];

	quiet = 0;

_again:
	foldmsg = 0;
	for (; *cmdp == ' ' || *cmdp == '\t'; cmdp++)
		;
	if (*cmdp != '\0') {
		for (cp = cmdp; *cp != '\0' && *cp != ' ' && *cp != '\t'; cp++)
			if (strchr("0123456789", *cp) == (char *) 0)
				foldmsg = 1;
		if (*cp != '\0')
			*cp++ = '\0';
		if (strcmp(cmdp, "-q") == 0) {
			if (quiet) {
				puts("Usage:  type [-q] [message-number]");
				return -1;
			}
			else {
				quiet = 1;
				cmdp = cp;
				goto _again;
			}
		}
		while (*cp != '\0')
			if (*cp != '\t' && *cp != ' ') {
				puts("Too many arguments.  Usage: type [-q] [message-number]");
				return -1;
			}
		if (!ismsg(cmdp)) {
			if (!quiet)
				puts("No such message.");
			return -1;
		}
		if (!foldmsg)
			msg = atoi(cmdp);
	}
	if (foldmsg)
		return dorms(cmdp, pager);
	sprintf(mname, ".%d", msg);
	if (ismsg(mname))
		return dorms(mname, pager);
	sprintf(mname, "%d", msg);
	if (!ismsg(mname)) {
		if (!quiet)
			printf("Message %d doesn't exist.\n", msg);
		return -1;
	}
	return dorms(mname, pager);
}

char *mlocation(msg)
char *msg; {
	static char path[1024];
	
	if ((msg[0] == '.' && msg[1] == '/') || msg[0] == '/')
		strcpy(path, msg);
	else
		sprintf(path, "%s/%s", location(folder), msg);
	return path;
}

char *mflocation(folder, msg)
char *folder, *msg; {
	static char path[1024];
	
	if ((msg[0] == '.' && msg[1] == '/') || msg[0] == '/')
		strcpy(path, msg);
	else
		sprintf(path, "%s/%s", location(folder), msg);
	return path;
}

dorms(msg, cmd)
char *msg, *cmd; {
	FILE *fp;
	int status;

	if (cmd == (char *) 0)
		fp = (FILE *) 0;
	else if ((fp = epopen(cmd, "w")) == (FILE *) 0) {
		printf("Can't open pipe to command: %s\n", cmd);
		return -1;
	}
	status = docopy(msg, fp);
	if (fp != (FILE *) 0)
		pclose(fp);
	return 1;
}

dosave(msg, file)
char *msg, *file; {
	FILE *fp;
	int status;
	char yesno[512];
	char *cp;

	if ((askappend[0] == 'Y' || askappend[0] == 'y' || askappend[0] == 'T' || askappend[0] == 't') && file != (char *) 0 && access(file, 0) == 0) {
		printf("File exists.  Do you wish to append to it? ");
		if (gets(yesno) == (char *) 0) {
			puts("No");
			return -1;
		}
		for (cp = yesno; *cp != ' ' && *cp != '\t'; cp++)
			;
		if (*cp != 'y' && *cp != 'Y')
			return -1;
	}
	if (file == (char *) 0)
		fp = (FILE *) 0;
	else if ((fp = efopen(file, "a")) == (FILE *) 0) {
		printf("Can't open file: %s\n", file);
		return -1;
	}
	status = docopy(msg, fp);
	if (fp != (FILE *) 0)
		fclose(fp);
	return status;
}

docopy(msg, mfp)
char *msg;
FILE *mfp; {
	char line[1024];
	FILE *fp;

	if ((fp = efopen(mlocation(msg), "r")) == (FILE *) 0) {
		printf("Message %s doesn't exist.\n", msg);
		return -1;
	}
	if (mfp == (FILE *) 0)
		mfp = stdout;
	if (msg[0] == '.')
		fprintf(mfp, "Message %s (deleted):\n", msg + 1);
	else
		fprintf(mfp, "Message %s:\n", msg);
	dopipe(fp, mfp);
	fclose(fp);
	return 1;
}

dopipe(msg, outp)
FILE *msg, *outp; {
	char ch;
	
	while ((ch = getc(msg)) != EOF)
		putc(ch, outp);
}

ismsg(msg)
char *msg; {
	char delmsg[256];
	struct stat sbuf;

	strcpy(delmsg, ".");
	strcat(delmsg, msg);
	if (access(mlocation(delmsg), 0) == 0) {
		stat(mlocation(delmsg), &sbuf);
		return (sbuf.st_mode & S_IFMT) != S_IFDIR;
	}
	if (access(mlocation(msg), 0) < 0)
		return 0;
	stat(mlocation(msg), &sbuf);
	return (sbuf.st_mode & S_IFMT) != S_IFDIR;
}

isdeleted(msg)
char *msg; {
	char delmsg[256];

	strcpy(delmsg, ".");
	strcat(delmsg, msg);
	return access(mlocation(delmsg), 0) == 0;
}

printmsg(cmdp)
char *cmdp; {
	char *cp;
	int foldmsg;
	char mname[80];

	foldmsg = 0;
	for (; *cmdp == ' ' || *cmdp == '\t'; cmdp++)
		;
	if (*cmdp != '\0') {
		for (cp = cmdp; *cp != '\0' && *cp != ' ' && *cp != '\t'; cp++)
			if (strchr("0123456789", *cp) == (char *) 0)
				foldmsg = 1;
		if (*cp != '\0')
			*cp++ = '\0';
		while (*cp != '\0')
			if (*cp != '\t' && *cp != ' ') {
				puts("Too many arguments.  Usage: print [message-number]");
				return -1;
			}
		if (!ismsg(cmdp)) {
			puts("No such message.");
			return -1;
		}
		if (!foldmsg)
			msg = atoi(cmdp);
	}
	if (foldmsg)
		return dorms(cmdp, printcmd);
	sprintf(mname, ".%d", msg);
	if (ismsg(mname))
		return dorms(mname, printcmd);
	sprintf(mname, "%d", msg);
	if (!ismsg(mname)) {
		printf("Message %d doesn't exist.\n", msg);
		return -1;
	}
	return dorms(mname, printcmd);
}

savemsg(cmdp)
char *cmdp; {
	char *cp, *savep;
	int foldmsg, status;
	char mname[80], savefile[80];

	foldmsg = 0;
	for (; *cmdp == ' ' || *cmdp == '\t'; cmdp++)
		;
	if (*cmdp == '\0')
		sprintf(savefile, "%s/%s", location(savefolder), folder);
	else {
		for (cp = cmdp; *cp != ' ' && *cp != '\t' && *cp != '\0'; cp++)
			;
		if (*cp != '\0')
			*cp++ = '\0';
		if (strchr(cmdp, '/') != (char *) 0)
			strcpy(savefile, cmdp);
		else
			sprintf(savefile, "%s/%s", location(savefolder), cmdp);
		for (savep = cp; *cp != '\0' && *cp != ' ' && *cp != '\t'; cp++)
			if (strchr("0123456789", *cp) == (char *) 0)
				foldmsg = 1;
		if (*cp != '\0')
			*cp++ = '\0';
		while (*cp == ' ' || *cp == '\t')
			cp++;
		if (*cp != '\0') {
			puts("Too many arguments.  Usage: save file [message-number]");
			return -1;
		}
		if (*savep != '\0') {
			if (!ismsg(savep)) {
				puts("No such message.");
				return -1;
			}
			if (!foldmsg)
				msg = atoi(savep);
		}
	}
	if (foldmsg)
		return dosave(savep, savefile);
	sprintf(mname, ".%d", msg);
	if (ismsg(mname))
		status = dosave(mname, savefile);
	else {
		sprintf(mname, "%d", msg);
		if (!ismsg(mname)) {
			printf("Message %d doesn't exist.\n", msg);
			return -1;
		}
		status = dosave(mname, savefile);
	}
	if (status < 1)
		return status;
	return delmsg("");
}

gomsg(cmdp)
char *cmdp; {
	char *cp;

	for (; *cmdp == ' ' || *cmdp == '\t'; cmdp++)
		;
	if (*cmdp == '\0') {
		puts("Usage: goto message-number");
		return -1;
	}
	for (cp = cmdp; *cp != '\0' && *cp != ' ' && *cp != '\t'; cp++)
		if (strchr("0123456789", *cp) == (char *) 0) {
			puts("Invalid message number.  Usage: goto message-number");
			return -1;
		}
	if (*cp != '\0')
		*cp++ = '\0';
	while (*cp != '\0')
		if (*cp != '\t' && *cp != ' ') {
			puts("Too many arguments.  Usage: goto message-number");
			return -1;
		}
	if (!ismsg(cmdp)) {
		puts("No such message.");
		return -1;
	}
	msg = atoi(cmdp);
	if (autonext[0] == 'y' || autonext[0] == 'Y' || autonext[0] == 't' || autonext[0] == 'T')
		return readmsg("");
	return 1;
}

listmsg(cmdp)
char *cmdp; {
	char *cp;
	DIR *dp;
	struct direct *entry;
	extern int errno;
	extern char *sys_errlist[];
	FILE *mp;
	char mline[2048], from[21], subj[51];
	int cnt, cmsg;

	for (; *cmdp == ' ' || *cmdp == '\t'; cmdp++)
		;
	if (*cmdp == '\0')
		cmdp = folder;
	else {
		for (cp = cmdp; *cp != '\0' && *cp != ' ' && *cp != '\t'; cp++)
			;
		if (*cp != '\0')
			*cp++ = '\0';
		while (*cp != '\0')
			if (*cp != '\t' && *cp != ' ') {
				puts("Too many arguments.  Usage: list [folder-name]\nOr: list folders");
				return -1;
			}
		if (!isfolder(cmdp)) {
			puts("No such folder.");
			return -1;
		}
	}
	if ((dp = opendir(location(cmdp))) == (DIR *) 0) {
		puts("Can't open folder for listing.");
		return -1;
	}
	cnt = 0;
	if (atoi(lines) < 3)
		strcpy(lines, "24");
	while ((entry = readdir(dp)) != (struct direct *) 0) {
		if (entry->d_name[0] == '.' && (entry->d_name[1] == '\0' || (entry->d_name[1] == '.' && entry->d_name[2] == '\0')))
			continue;
		from[0] = '\0';
		subj[0] = '\0';
		cnt++;
		if (cnt == 1)
			printf("Contents of folder \"%s\":\nSt Name     From                 Subject\n", cmdp);
		if (sscanf(entry->d_name, "%d", &cmsg) != 1)
			putchar(' ');
		else if (cmsg == msg)
			putchar('*');
		else
			putchar(' ');
		if (entry->d_name[0] == '.')
			printf("D %-8.8s ", &entry->d_name[1]);
		else
			printf("  %-8.8s ", entry->d_name);
		if ((mp = efopen(mflocation(cmdp, entry->d_name), "r")) == (FILE *) 0) {
			puts("unreadable");
			if (cnt % (atoi(lines) - 1) == 0)
				getcont();
			continue;
		}
		while (fgets(mline, sizeof mline, mp) != (char *) 0) {
			if (mline[0] == '\n')
				break;
			if (strncmp(mline, "Subject: ", 9) == 0) {
				mline[strlen(mline) - 1] = '\0';
				strncpy(subj, &mline[9], 50);
				subj[50] = '\0';
			}
			if (strncmp(mline, "From: ", 6) == 0) {
				mline[strlen(mline) - 1] = '\0';
				strncpy(from, &mline[6], 20);
				from[20] = '\0';
			}
		}
		printf("%-20.20s", from);
		if (subj[0] != '\0')
			printf(" \"%.50s\"", subj);
		putchar('\n');
		if (cnt % (atoi(lines) - 1) == 0)
			getcont();
		fclose(mp);
	}
	if (cnt == 0)
		printf("No messages in folder \"%s\".\n", folder);
	return 1;
}

foldlist(cmdp)
char *cmdp; {
	DIR *dp;
	struct direct *entry;
	int cnt;
	
	while (*cmdp == '\t' || *cmdp == ' ')
		cmdp++;
	if (*cmdp != '\0') {
		puts("Usage:  folders");
		return -1;
	}
	if ((dp = opendir(cabinet)) == (DIR *) 0) {
		puts("Can't open cabinet to list folders.");
		return -1;
	}
	puts("The following folders exist in the cabinet:");
	if (atoi(lines) <= 3)
		strcpy(lines, "24");
	while ((entry = readdir(dp)) != (struct direct *) 0) {
		if (entry->d_name[0] == '.' && (entry->d_name[1] == '\0' || (entry->d_name[1] == '.' && entry->d_name[2] == '\0')))
			continue;
		printf("  %s\n", entry->d_name);
		if (++cnt % atoi(lines) - 1 == 0)
			getcont();
	}
	return 1;
}
