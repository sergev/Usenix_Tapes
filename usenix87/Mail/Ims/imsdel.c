#include "ims.h"

char autonext[128] = "No";

delmsg(cmdp)
char *cmdp; {
	char *cp;
	char mname[80], curmsg[80], mt1[100], mt2[100];

	for (; *cmdp == ' ' || *cmdp == '\t'; cmdp++)
		;
	if (*cmdp != '\0') {
		for (cp = cmdp; *cp != '\0' && *cp != ' ' && *cp != '\t'; cp++)
			;
		if (*cp != '\0')
			*cp++ = '\0';
		while (*cp != '\0')
			if (*cp != '\t' && *cp != ' ') {
				puts("Too many arguments.  Usage: delete [message-number]");
				return -1;
			}
		if (!ismsg(cmdp)) {
			puts("No such message.");
			return -1;
		}
	}
	else {
		sprintf(curmsg, "%d", msg);
		cmdp = curmsg;
	}
	sprintf(mname, ".%s", cmdp);
	if (ismsg(mname)) {
		puts("The message is already deleted.  Type \"expunge\" to free its space.");
		return -1;
	}
	if (!ismsg(cmdp)) {
		puts("There is no such message.");
		return -1;
	}
	strcpy(mname, ".");
	strcat(mname, cmdp);
	strcpy(mt1, mlocation(cmdp));
	strcpy(mt2, mlocation(mname));
	if (link(mt1, mt2) < 0) {
		puts("Could not flag the message as deleted.");
		return -1;
	}
	if (unlink(mt1) < 0) {
		puts("Could not flag the message as deleted.");
		unlink(mt2);
		return -1;
	}
	msg++;
	if (autonext[0] == 'y' || autonext[0] == 'Y' || autonext[0] == 't' || autonext[0] == 'T')
		if (readmsg("-q") < 0) {
			puts("No next message.");
			return -1;
		}
	return 1;
}

undelmsg(cmdp)
char *cmdp; {
	char *cp;
	char mname[80], curmsg[80], mt1[100], mt2[100];

	for (; *cmdp == ' ' || *cmdp == '\t'; cmdp++)
		;
	if (*cmdp != '\0') {
		for (cp = cmdp; *cp != '\0' && *cp != ' ' && *cp != '\t'; cp++)
			;
		if (*cp != '\0')
			*cp++ = '\0';
		while (*cp != '\0')
			if (*cp != '\t' && *cp != ' ') {
				puts("Too many arguments.  Usage: undelete [message-number]");
				return -1;
			}
		if (!ismsg(cmdp)) {
			puts("No such message.");
			return -1;
		}
	}
	else {
		sprintf(curmsg, "%d", msg);
		cmdp = curmsg;
	}
	sprintf(mname, ".%s", cmdp);
	if (!ismsg(mname)) {
		puts("The message isn't deleted.");
		return -1;
	}
	strcpy(mname, ".");
	strcat(mname, cmdp);
	strcpy(mt1, mlocation(cmdp));
	strcpy(mt2, mlocation(mname));
	if (link(mt2, mt1) < 0) {
		puts("Could not flag the message as restored.");
		return -1;
	}
	if (unlink(mt2) < 0) {
		puts("Could not flag the message as restored.");
		unlink(mt1);
		return -1;
	}
	return 1;
}

expunge(cmdp)
char *cmdp; {
	char *cp;
	DIR *dp;
	struct direct *entry;
	extern int errno;
	extern char *sys_errlist[];

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
				puts("Too many arguments.  Usage: expunge [folder-name]");
				return -1;
			}
		if (!isfolder(cmdp)) {
			puts("No such folder.");
			return -1;
		}
	}
	if ((dp = opendir(location(cmdp))) == (DIR *) 0) {
		puts("Can't open folder for expunging.");
		return -1;
	}
	while ((entry = readdir(dp)) != (struct direct *) 0) {
		if (entry->d_name[0] != '.')
			continue;
		if (entry->d_name[1] == '\0' || (entry->d_name[1] == '.') && entry->d_name[2] == '\0')
			continue;
		if (unlink(mflocation(cmdp, entry->d_name)) < 0)
			printf("Couldn't expunge message: %s (%s)\n", &entry->d_name[1], sys_errlist[errno]);
	}
	return 1;
}
