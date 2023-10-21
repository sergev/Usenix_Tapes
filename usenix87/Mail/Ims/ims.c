#include "ims.h"

struct cmdtab {
	char *c_cmd;
	int (*c_func)();
	char *c_help;
} cmdtab[] = {
	"delete",	delmsg,		"Delete a message",
	"undelete",	undelmsg,	"Recover a deleted message",
	"expunge",	expunge,	"Recover space occupied by deleted messages",
	"type",		readmsg,	"Display a message",
	"print",	printmsg,	"Print a message on printer",
	"save",		savemsg,	"Save a message in a file or folder",
	"set",		varops,		"Display or modify IMS variables",
	"next",		nextmsg,	"Go to the next message",
	"+",		nextmsg,	"Go to the next message",
	"previous",	prevmsg,	"Go to the previous message",
	"-",		prevmsg,	"Go to the previous message",
	"write",	savemsg,	"Save a message in a file or folder",
	"reply",	reply,		"Reply to a message",
	"goto",		gomsg,		"Set the current message",
	"mail",		mailto,		"Send mail to users",
	"send",		mailto,		"Send mail to users",
	"list",		listmsg,	"List the headers of messages",
	"headers",	listmsg,	"List the headers of messages",
	"forward",	forward,	"Forward a message to another person",
	"from",		listmsg,	"List the headers of messages",
	"quit",		byebye,		"Leave IMS, expunging deleted messages",
	"exit",		nxbyebye,	"Leave IMS without expunging messages",
	"xit",		nxbyebye,	"Leave IMS without expunging messages",
	"?",		help,		"Get help on a command",
	"help",		help,		"Get help on a command",
	"folder",	setfolder,	"Set or show the current folder",
	"read",		readmbox,	"Read old-style mailbox into folder",
	"inc",		readmbox,	"Read old-style mailbox into folder",
	"folders",	foldlist,	"List the existing folders",
	"alias",	aliasops,		"Display or create mail aliases",
	"unalias",	unalias,	"Delete mail aliases",
	(char *) 0,	readmsg,	"Read the named or numbered message",
};

char folder[256];
char cabinet[256];
char mailbox[1024];
char prompt[256];
int msg;
jmp_buf mainloop;
char sysmbox[1024];
char autoread[256] = "No";
char lines[256] = "24";
int _x;

intr() {
	longjmp(mainloop, 1);
}

main(argc, argv)
char *argv[]; {
	char line[512];
	char *cp;
	int nmsg, incf;
	struct stat sbuf;

	if ((cp = getenv("HOME")) == (char *) 0)
		cp = "/usr/guest";
	sprintf(cabinet, "%s/.mail", cp);
	sprintf(sysmbox, SYSMAILBOX, getlogin());
	strcpy(mailbox, sysmbox);
	strcpy(line, "incoming-mail");
	strcpy(prompt, "IMS> ");
	readvars();
	readalias();
	if (access(cabinet, 0) < 0)
		if (mkdir(cabinet) < 0) {
			puts("Can't create your mail cabinet.  Please check your CABINET and your home\ndirectory.");
			exit(1);
		}
	if ((cp = getenv("FOLDER")) != (char *) 0)
		strcpy(line, cp);
	if (setfolder(line) < 0) {
		puts("Please check your CABINET and FOLDER.");
		exit(1);
	}
	if ((cp = getenv("MAIL")) != (char *) 0)
		strcpy(mailbox, cp);
	if ((cp = getenv("PAGER")) != (char *) 0)
		strcpy(pager, cp);
	if ((cp = getenv("VISUAL")) == (char *) 0)
		if ((cp = getenv("EDITOR")) == (char *) 0)
			cp = (char *) 0;
	if (cp != (char *) 0)
		strcpy(editor, cp);
	if ((cp = getenv("PRINTMSG")) == (char *) 0)
		strcpy(printcmd, "pr -h \"Printed by: ${LOGNAME:-${USER:-anonymous}} \" | lpr");
	else
		strcpy(printcmd, cp);
	if ((cp = getenv("SAVEFOLDER")) != (char *) 0)
		strcpy(savefolder, cp);
	if ((cp = getenv("LINES")) != (char *) 0)
		strcpy(lines, cp);
	incf = 1;
	nmsg = 1;
	if (argc >= nmsg + 1 && strcmp(argv[nmsg], "-i") == 0) {
		incf = 0;
		nmsg++;
	}
	line[0] = '\0';
	for (; nmsg < argc; nmsg++) {
		strcat(line, " ");
		strcat(line, argv[nmsg]);
	}
	if (line[0] != '\0') {
		dispatch(line);
		exit(0);
	}
	if (incf) {
		msg = receive(mailbox);
		if (msg == -1)
			puts("Your system mailbox has been deleted.  Please contact your system administrator\nin order to have it restored.");
		if (msg != 0)
			printf("New messages start at %d.\n", msg);
		else {
			listmsg("");
			msg = 1;
		};
	}
	_x = 0;
	if (isatty(2)) {
		stat(ttyname(2), &sbuf);
		if (sbuf.st_mode & 0111) {
			_x = 1;
			chmod(ttyname(2), sbuf.st_mode & ~0111);
		}
	}
	signal(SIGINT, intr);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	if (setjmp(mainloop) != 0)
		puts("\nCommand interrupted.");
	for (;;) {
		if (newmail(mailbox)) {
			puts("New mail has arrived.  Reading your mailbox...");
			if (autoread[0] == 'Y' || autoread[0] == 'y' || autoread[0] == 'T' || autoread[0] == 't') {
				nmsg = receive(mailbox);
				if (nmsg == -1)
					puts("Your system mailbox has been deleted.  Please contact your system administrator\nin order to have it restored.");
				if (nmsg != 0)
					printf("New messages start at %d.\n", nmsg);
			}
		}
		printf("%s", prompt);
		if (gets(line) == (char *) 0) {
			putchar('\n');
			expunge("");
			break;
		}
		for (cp = line; *cp == ' ' || *cp == '\t'; cp++)
			;
		if (*cp == '!') {
			system(cp + 1);
			continue;
		}
		if (dispatch(cp) == 0)
			break;
	}
	if (_x) {
		stat(ttyname(2), &sbuf);
		chmod(ttyname(2), sbuf.st_mode | 0111);
	}
	exit(0);
}

dispatch(cmd)
char *cmd; {
	char *cp, *cmdp, *nextp;
	int cnt, status, ran;
	
	for (;;) {
		if ((nextp = strchr(cmd, ';')) == (char *) 0)
			nextp = "\376";
		else
			*nextp++ = '\0';
		for (cp = cmd; *cp == ' ' || *cp == '\t'; cp++)
			;
		for (cmdp = cp; *cp != '\0' && *cp != '\t' && *cp != ' '; cp++)
			;
		if (*cp != '\0')
			*cp++ = '\0';
		if (*cmdp == '\376')
			break;
		ran = 0;
		cnt = 0;
		if (strlen(cmdp) != 0) {
			for (cnt = 0; cmdtab[cnt].c_cmd != (char *) 0; cnt++) {
				if (strlen(cmdp) > strlen(cmdtab[cnt].c_cmd))
					continue;
				if (strncmp(cmdp, cmdtab[cnt].c_cmd, strlen(cmdp)) == 0)
					if ((status = (*cmdtab[cnt].c_func)(cp)) < 1)
						return status;
					else {
						ran = 1;
						break;
					}
			}
		}
		while (cmdtab[cnt].c_cmd != (char *) 0)
			cnt++;
		if (!ran) {
			if (cmdtab[cnt].c_func != (int (*)()) 0) {
				if ((status = (*cmdtab[cnt].c_func)(cmdp)) < 1)
					return status;
			}
			else {
				printf("Unknown command: \"%s\".  Type \"help\" for help.\n", cmdp);
				return -1;
			}
		}
		cmd = nextp;
	}
	return status;
}

help(cmdp)
char *cmdp; {
	char *cp, *dp;
	int cnt, icnt;

	for (cp = cmdp; *cp == ' ' || *cp == '\t'; cp++)
		;
	for (dp = cp; *dp != ' ' && *dp != '\t' && *dp != '\0'; dp++)
		;
	if (*dp != '\0')
		*dp++ = '\0';
	while (*dp == ' ' || *dp == '\t')
		;
	if (*dp != '\0') {
		puts("Usage:  help [topic]");
		return -1;
	}
	icnt = 0;
	if (atoi(lines) <= 3)
		strcpy(lines, "24");
	for (cnt = 0; cmdtab[cnt].c_cmd != (char *) 0; cnt++) {
		if (strlen(cp) > strlen(cmdtab[cnt].c_cmd))
			continue;
		if (*cp == '\0' || strncmp(cp, cmdtab[cnt].c_cmd, strlen(cp)) == 0) {
			printf("  %-16s   %s\n", cmdtab[cnt].c_cmd, cmdtab[cnt].c_help);
			icnt++;
			if (icnt % (atoi(lines) - 1) == 0)
				getcont();
		}
	}
	if (*cp != '\0' && icnt == 0)
		if (strcmp(cp, "default") == 0)
			printf("  %-16s   %s\n", "Default:", cmdtab[cnt].c_help);
		else {
			printf("Unknown topic: \"%s\".\n", cp);
			return -1;
		}
	else if (*cp == '\0')
		printf("  %-16s   %s\n", "Default:", cmdtab[cnt].c_help);
	return 1;
}

setfolder(cmdp)
char *cmdp; {
	char *cp;
	
	for (; *cmdp == ' ' || *cmdp == '\t'; cmdp++)
		;
	if (*cmdp == '\0') {
		printf("The current folder is \"%s\".\n", folder);
		return 1;
	}
	for (cp = cmdp; *cp != '\0' && *cp != ' ' && *cp != '\t'; cp++)
		;
	if (*cp != '\0')
		*cp++ = '\0';
	while (*cp != '\0')
		if (*cp != ' ' || *cp != '\t') {
			puts("Too many arguments.  Usage: folder [folder-name]");
			return -1;
		}
	if (access(location(cmdp), 0) < 0) {
		if (mkdir(location(cmdp)) < 0) {
			fprintf(stderr, "Can't create \"%s\" folder.\n", cmdp);
			return -1;
		}
		if (access(location(cmdp), 0) < 0) {
			fprintf(stderr, "I can't get at your \"%s\" folder.\n, cmdp");
			return -1;
		}
	}
	strcpy(folder, cmdp);
	return 1;
}

mkdir(dir)
char *dir; {
	int pid, status;
	
	switch (pid = fork()) {
		case -1:
			perror("");
			printf("Can't create directory \"%s\".\n", dir);
			return -1;
		case 0:
			execl("/bin/mkdir", "mkdir", dir, (char *) 0);
			_exit(-100);
		default:
			while (wait(&status) != pid)
				;
			if (status != 0)
				return -1;
	}
	return 0;
}

char *location(folder)
char *folder; {
	static char buf[1024];
	
	if (folder[0] == '/' || folder[0] == '.')
		return folder;
	sprintf(buf, "%s/%s", cabinet, folder);
	return buf;
}

byebye(cmdp)
char *cmdp; {
	for (; *cmdp == ' ' || *cmdp == '\t'; cmdp++)
		;
	if (*cmdp != '\0') {
		puts("Too many arguments.  Usage: quit\n                        Or: xit");
		return -1;
	}
	if (expunge("") < 0)
		return -1;
	return 0;
}

nxbyebye(cmdp)
char *cmdp; {
	for (; *cmdp == ' ' || *cmdp == '\t'; cmdp++)
		;
	if (*cmdp != '\0') {
		puts("Too many arguments.  Usage: xit");
		return -1;
	}
	return 0;
}

char *basename(s)
char *s; {
	char *cp;
	
	if ((cp = strrchr(s, '/')) != (char *) 0)
		return cp + 1;
	return s;
}

isfolder(f)
char *f; {
	struct stat s;
	
	if (stat(location(f), &s) < 0)
		return 0;
	return (s.st_mode & S_IFMT) == S_IFDIR;
}

issysmbox(mbox)
char *mbox; {
	return strcmp(mbox, sysmbox) == 0;
}
