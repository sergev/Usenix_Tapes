/*
 *	@(#)system.c	1.2 (TDI) 2/3/87
 *	@(#)Copyright (C) 1984, 85, 86, 87 by Brandon S. Allbery.
 *	@(#)This file is part of UNaXcess version 1.0.2.
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)system.c	1.2 (TDI) 2/3/87";
static char _UAID_[]   = "@(#)UNaXcess version 1.0.2";
#endif lint

#include "ua.h"

struct sys parms = {
#ifdef NOAUTOPATH
	NOAUTOPATH,
#else
	"/usr/unaxcess",
#endif NOAUTOPATH
	1,
	0,
	"/bin/sh",
	1,
	"unaxcess",
	30,
	"sysop",
	1,
	0,
	"",
	"",
	3,
	"trap '' 2; stty -echo; echo 'Begin sending your file.  End with a CONTROL-D.'; cat - > %s; stty echo",
	"trap '' 2; cat %s",
	"umodem -rbld",
	"umodem -sbld",
	"kermit -iwr ;:",
	"kermit -iws",
	A_GUEST,
};

#define NUM		0
#define STR		1
#define BOOL		2

struct rparm {
	char *parmname;
	char parmtype;
	char *parmval;
} sysparms[] = {
	"bbs-directory",STR,	parms.ua_home,
	"readonly",	BOOL,	&parms.ua_roc,
	"restricted",	BOOL,	&parms.ua_xrc,
	"shell",	STR,	parms.ua_shell,
	"read-env",	BOOL,	&parms.ua_env,
	"bbs-user",	STR,	parms.ua_bbs,
	"time-limit",	NUM,	&parms.ua_tlimit,
	"sysop",	STR,	parms.ua_sysop,
 	"private-msgs",	BOOL,	&parms.ua_pm,
	"logging",	BOOL,	&parms.ua_log,
	"banner",	STR,	parms.ua_bnr,
	"login-msg",	STR,	parms.ua_login,
	"login-tries",	NUM,	&parms.ua_nla,
	"ascii-upload",	STR,	parms.ua_auc,
	"ascii-download",STR,	parms.ua_adc,
	"xmodem-upload",STR,	parms.ua_xuc,
	"xmodem-download",STR,	parms.ua_xdc,
	"kermit-upload",STR,	parms.ua_kuc,
	"kermit-download",STR,	parms.ua_kdc,
	"validation-level",NUM,	&parms.ua_vaxs,
	0,		0,	0,
};

static FILE *lfp;
static int __tlog = 0;
short critical = 0;
short quitc = 0;
short intr = 0;
short alrm = 0;
short warned = 0;

logon() {
	struct stat sb;
	char *cp;

	if (!parms.ua_log || stat(LOG, &sb) < 0) {		/* no logfile => no logging */
		lfp = NULL;
		return;
	}
	if ((lfp = fopen(LOG, "a")) == NULL) {
		io_off();
		perror(LOG);
		fprintf(stderr, "panic: log\n");
		exit(2);
	}
}

log(fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9)
char *fmt, *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9; {
	char buf[1024];
	static char lockfile[] = "logfile.lock";

	if (lfp == NULL)			/* logging not enabled */
		return;
	CRIT();
	sprintf(buf, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9);
	mklock(lockfile);
	fprintf(lfp, "%s (%05d)  %s\n", date(), getpid(), visible(buf));
	fflush(lfp);
	rmlock(lockfile);
	NOCRIT();
}

logsig(sig)
int sig; {
	static int dbl = 0;	/* OOPS!  9 lines added 2/3/87 ++bsa */
	
	if (dbl++) {
		fprintf(stderr, "\r\n\r\nDOUBLE PANIC: SIG %d\r\n\r\n", sig);
		setuid(getuid());
		chdir("/tmp");
		signal(SIGIOT, SIG_DFL);
		kill(getpid(), SIGIOT);
	}
	dolog();
	log("Received signal %d.", sig);
	fprintf(stderr, "\r\n\r\nUNaXcess internal error: %d.\r\n\r\n", sig);
	unlink(RIndex(ttyname(fileno(stdin)), '/') + 1);
	io_off();
	signal(SIGIOT, SIG_DFL);
	setuid(getuid());			/* OOPS!  2 lines added */
	chdir(getpwuid(getuid())->pw_dir);	/*   2/3/87 ++bsa */
	kill(getpid(), SIGIOT);
}

panic(s)
char *s; {
	static int dbl = 0;	/* OOPS!  9 lines added 2/3/87 ++bsa */

	if (dbl++) {
		fprintf(stderr, "\r\n\r\nDOUBLE PANIC: %s\r\n\r\n", s);
		setuid(getuid());
		chdir("/tmp");
		signal(SIGIOT, SIG_DFL);
		kill(getpid(), SIGIOT);
	}
	dolog();
	log("panic: %s", s);
	fprintf(stderr, "\r\n\r\nUNaXcess internal error: %s\r\n\r\n", s);
	io_off();
	unlink(RIndex(ttyname(2), '/') + 1);
	setuid(getuid());
	chdir(getpwuid(getuid())->pw_dir);
	signal(SIGIOT, SIG_DFL);
	kill(getpid(), SIGIOT);
}

quit() {
	char line[256];

	if (critical) {
		quitc++;
		return;
	}
	writes("\n\nFast logout\n");
	signal(SIGQUIT, quit);
	log("Signalled QUIT.");
	writef("\nDo you really want to leave UNaXcess? N\b");
	line[0] = readc();
	if (line[0] == 'Y') {
		writef("OK");
		if (user.u_name[0] != '\0')
			writef(", %s", upstr(user.u_name));
		writef(".  See you later!\n\n\n");
		cleanup();
	}
}

intrp() {
	if (critical) {
		intr++;
		return;
	}
	writes("\n\nAborted.");
	log("Command aborted.");
	signal(SIGINT, intrp);
	longjmp(cmdloop, 1);
}

char *visible(s)
char *s; {
	static char vs[256];
	char *sp, *vp;

	vp = vs;
	for (sp = s; *sp != '\0'; sp++)
	if (!iscntrl(*sp))
		*vp++ = *sp;
	else {
		*vp++ = '^';
		*vp++ = uncntrl(*sp);
	}
	*vp = '\0';
	return vs;
}

shell() {
	short sig;

	if (user.u_access == A_GUEST || user.u_access == A_USER || parms.ua_shell[0] == '\0') {
		writes("You don't have shell access privileges.");
		log("Security violation:  Unauthorized SHELL");
		return 1;
	}
	switch (fork()) {
	case -1:
		log("Error %d forking shell", errno);
		writes("Sorry, the system's full.  Try again later.");
		return 1;
	case 0:
		for (sig = SIGINT; sig <= SIGTERM; sig++)
			signal(sig, SIG_DFL);
		setuid(getuid());
		chdir(getpwuid(getuid())->pw_dir);
		run(parms.ua_shell, 0);
		log("Error %d exec'ing %s", errno, parms.ua_shell);
		writes("Couldn't run the shell.");
		exit(1);
	default:
		CRIT();
		io_off();
		for (sig = SIGINT; sig <= SIGTERM; sig++)
			signal(sig, SIG_IGN);
		signal(SIGALRM, thatsall);	/* trapped by the CRIT() */
		wait(NULL);
		signal(SIGINT, intrp);
		signal(SIGQUIT, quit);
		for (sig = SIGIOT; sig < SIGTERM; sig++)
			signal(sig, logsig);
		signal(SIGALRM, thatsall);
		io_on(0);
		NOCRIT();
	}
	return 1;
}

thatsall() {
	if (critical) {
		alrm++;
		return;
	}
	if (warned) {
		log("Timeout.");
		writes("\nI'm sorry, but you're out of time.\n\n");
		cleanup();
	}
	else {
		log("5-minute warning.");
		writes("\nYou have only five minutes left in this session.\n\n");
		warned = 1;
		alarm(5 * 60);
	}
}

mklock(lockfile)
char *lockfile; {
	char lockpath[50];
	int lock_fd;
	struct stat statbuf;
	long now;
	
	strcpy(lockpath, "lock/");
	strcat(lockpath, lockfile);
	while (stat(lockpath, &statbuf) == 0) {
		time(&now);
		if (now - statbuf.st_mtime > 90) {	/* OOPS!  2/3/87 ++bsa */
			unlink(lockpath);
			break;
		}
		sleep(5);	/* OOPS!  1 line added 2/3/87 ++bsa */
	}
	if ((lock_fd = creat(lockpath, 0600)) < 0) {
		fprintf(stderr, "Errno = %d creating lockfile %s\n", errno, lockpath);
		/* OOPS!  1 line deleted 2/3/87 ++bsa */
		panic(lockpath);
	}
	close(lock_fd);
	sync();	/* insure disk files are updated! */
}

rmlock(lockfile)
char *lockfile; {
	char lockpath[50];
	struct stat statbuf;
	
	sync();
	strcpy(lockpath, "lock/");
	strcat(lockpath, lockfile);
	if (stat(lockpath, &statbuf) < 0) {
		log("Lockfile %s deleted???", lockpath);
		writef("\n\nSomeone futzed with the lockfile.  Please tell %s IMMEDIATELY!!!\nSorry, but this means I have to log you out now.\n\n", parms.ua_sysop);
		panic("LOCKFILE DELETED");
	}
	if (unlink(lockpath) < 0) {
		log("Errno = %d, can't unlink lockfile %s", errno, lockpath);
		writes("\nI've got a lockfile problem.  You won't be able to do some\nthings until it's fixed.  Sorry...\n");
	}
}

CRIT() {
	alrm = 0;
	quitc = 0;
	intr = 0;
	if (critical)
		return;	/* clears pending signals */
	critical = 1;
}

NOCRIT() {
	if (!critical)
		return;
	critical = 0;
	if (alrm)
		thatsall(14);
	if (quitc)
		quit(3);
	if (intr)
		intrp(2);
	alrm = 0;
	quitc = 0;
	intr = 0;
}

run(cmd, arg)
char *cmd, *arg; {
	char cmdbuf[5120];
	
	sprintf(cmdbuf, "%s %s", cmd, (arg? arg: ""));
	execl("/bin/sh", "sh", "-c", cmdbuf, 0);
	return -1;
}

copylink(src, dest)
char *src, *dest; {
	int srcp, destp, cnt;
	char buf[1024];
	
	if (link(src, dest) == 0) {
		unlink(src);
		return 0;
	}
	if ((srcp = open(src, 0)) < 0) {
		perror(src);
		return -1;
	}
	unlink(dest);
	if ((destp = creat(dest, 0600)) < 0) {
		perror(dest);
		return -1;
	}
	while ((cnt = read(srcp, buf, sizeof buf)) > 0)
		write(destp, buf, cnt);
	close(destp);
	close(srcp);
	return 0;
}

s_cmp(s1, s2)
char *s1, *s2; {
	for (; *s1 != '\0' && ToLower(*s1) == ToLower(*s2); s1++, s2++)
		;
	return (!(*s1 == '\0' && *s2 == '\0'));
}

char *upstr(s)
char *s; {
	static char sbuf[512];
	register char *cp, *dp;
	
	for (cp = s, dp = sbuf; *cp != '\0'; cp++, dp++)
		*dp = ToUpper(*cp);
	*dp = '\0';
	return sbuf;
}

ismember(word, list)
char *word, *list; {
	char *cp;
	char oldch;
	
	while (*list != '\0') {
		while (*list == ' ' || *list == '\t')
			list++;
		for (cp = list; *cp != '\0' && *cp != ',' && *cp != ' ' && *cp != '\t'; cp++)
			;
		oldch = '\0';
		if (*cp != '\0') {
			oldch = *cp;
			*cp++ = '\0';
		}
		if (strcmp(word, list) == 0) {
			*(cp - 1) = oldch;
			return 1;
		}
		*(cp - 1) = oldch;
		list = cp;
	}
	return 0;
}

dolog() {
	if (lfp != (FILE *) 0 || __tlog)
		return;
	logon();
	__tlog = 1;
}

nolog() {
	if (!__tlog || lfp == (FILE *) 0)
		return;
	fclose(lfp);
	__tlog = 0;
}

alter() {
	char line[256], uname[256];
	char *p, *q, *okcmds;
	struct user ubuf;
	short crypted, not_FW;
	char cmd;

	if (user.u_access == A_MKUSER) {
		log("An A_MKUSER got into alter()???");
		panic("newalter");
	}
	if (user.u_access != A_WITNESS) {
		not_FW = 1;
		log("Alter by non-Witness; restricting control modes.");
		strcpy(line, user.u_name);
	}
	else {
		line[0] = '\0';
		not_FW = 0;
		writef("Examine which user: ");
		reads(line);
		log("User: %s", line);
		if (line[0] == '\0')
			return 1;
		for (p = line; *p != '\0'; p++)
			*p = ToLower(*p);
		line[32] = '\0';
	}
	if (!getuser(line, &ubuf))
		if (not_FW) {
			log("Can't locate current user in the userfile.");
			panic("user");
		}
		else {
			writef("No such user.  Create him ? N\b");
			strcpy(ubuf.u_name, line);
			line[0] = readc();
			log("New user? %c", line[0]);
			if (line[0] != 'Y')
				return 1;
			ubuf.u_pass[0] = '\0';
			ubuf.u_access = A_USER;
			ubuf.u_llen = 80;
			ubuf.u_nbull = 0;
			strcpy(ubuf.u_lconf, "general");
			ubuf.u_lines = 24;
			crypted = 0;
		}
	else if (strlen(ubuf.u_pass) == 0)
		crypted = 0;
	else
		crypted = 1;
	strcpy(uname, ubuf.u_name);
	for (;;) {
		writec('\n');
		writef("Name: %s\n", ubuf.u_name);
		writef("Password: %s%s\n", ubuf.u_pass, (crypted? " (encrypted)": ""));
		writef("Access level: %s\n", ua_acl(ubuf.u_access));
		if (ubuf.u_access == A_MKUSER) {
			writef("Default access level: %s\n", ua_acl(ubuf.u_llen));
			okcmds = "NPADKQF";
		}
		else {
			writef("UNIX(R) login name: %s\n", ubuf.u_login);
			writef("Login conference: %s\n", ubuf.u_lconf);
			writef("Line size: %d\n", ubuf.u_llen);
			writef("Screen lines: %d\n", ubuf.u_lines);
			if (ubuf.u_access == A_WITNESS && s_cmp(user.u_name, parms.ua_sysop) != 0 && s_cmp(user.u_name, uname) != 0)
				okcmds = "Q";
			else if (user.u_access < A_WITNESS)
				okcmds = "PUCSLQF";
			else if (s_cmp(user.u_name, uname) == 0)
				okcmds = "NPUCSLQF";
			else
				okcmds = "NPAUCLSKQF";
		}
		writef("\nAlter command (");
		for (p = okcmds; *p != '\0'; p++)
			switch (*p) {
			case 'N':
				writef("Name, ");
				break;
			case 'P':
				writef("Password, ");
				break;
			case 'A':
				writef("Access, ");
				break;
			case 'U':
				writef("UNIX(R) login, ");
				break;
			case 'C':
				writef("Conference, ");
				break;
			case 'L':
				writef("Lines, ");
				break;
			case 'S':
				writef("Size, ");
				break;
			case 'D':
				writef("Default access, ");
				break;
			case 'K':
				writef("Kill, ");
				break;
			case 'Q':
				writef("Quit, ");
				break;
			case 'F':
				writef("Finished, ");
				break;
			default:
				log("Invalid internal alter() command: '%c'", *p);
				panic("altercmd");
			}
		writef("or ? for help): ");
		cmd = readc();
		log("Change: %c", cmd);
		if (cmd != '?' && Index(okcmds, cmd) == (char *) 0)
			cmd = ' ';
		switch (cmd) {
		case 'K':
			writef("Kill user -- are you sure? N\b");
			cmd = readc();
			log("Kill user? %c", cmd);
			if (cmd != 'Y')
				break;
			putuser(uname, (struct user *) 0);
			writef("User %s killed.\n", upstr(uname));
			return 1;
		case 'N':
			writef("Enter new name: ");
			reads(line);
			log("Name: %s", line);
			if (line[0] == '\0')
				break;
			for (p = line; *p != '\0'; p++)
				if (*p == ':') {
					log("Illegal colon in name.");
					writes("Can't put a colon in a user name.");
					break;
				}
			for (p = line, q = ubuf.u_name; *p != '\0'; p++, q++)
				*q = ToLower(*p);
			*q = '\0';
			break;
		case 'P':
			strcpy(line, getpass("Enter new password: "));
			if (line[0] == '\0')
				break;
			strcpy(ubuf.u_pass, line);
			crypted = 0;		/* it's not encrypted now */
			break;
		case 'A':
			writef("Access level: Deny access, Guest, Messages, Files, System, Witness, Newuser? ");
			cmd = readc();
			log("Access: %c", cmd);
			if (cmd == 'A' && s_cmp(user.u_name, parms.ua_sysop) != 0) {
				writes("Sorry, only the sysop can administer Witness privileges.");
				log("Security violation: WITNESS administering WITNESS");
				break;
			}
			switch (cmd) {
			case 'G':
				ubuf.u_access = A_GUEST;
				break;
			case 'D':
				ubuf.u_access = A_NONE;
				break;
			case ' ':
				break;
			case 'M':
				ubuf.u_access = A_USER;
				break;
			case 'S':
				ubuf.u_access = A_SYSTEM;
				break;
			case 'W':
				ubuf.u_access = A_WITNESS;
				break;
			case 'N':
				ubuf.u_access = A_MKUSER;
				break;
			case 'F':
				ubuf.u_access = A_FILES;
				break;
			default:
				writes("What?  Valid commands are D, G, M, F, S, W, and N.  Access unchanged.");
			}
			break;
		case 'C':
			writef("Enter the default login conference: ");
			reads(line);
			log("Login conference: %s", line);
			if (line[0] =='\0')
				break;
			if (!isconf(line))
				writes("That conference doesn't exist.");
			else if (uisunsub(ubuf.u_name, line))
				writef("%s isn't subscribed to %s.\n", upstr(ubuf.u_name), line);
			else if (!isrcmem(ubuf.u_name, line))
				writef("%s isn't a member of that conference.\n", upstr(ubuf.u_name));
			else
				strcpy(ubuf.u_lconf, line);
			break;
		case 'S':
			writef("Enter new line size, 32-132: ");
			reads(line);
			log("Line length: %s", line);
			if (line[0] == '\0')
				break;
			ubuf.u_llen = atoi(line);
			if (ubuf.u_llen < 32 || ubuf.u_llen > 132) {
				ubuf.u_llen = 80;
				writes("Garbage line size; using 80.");
			}
			break;
		case 'U':
			writef("UNIX(R) login name? ");
			reads(line);
			log("Login name: %s", line);
			if (strlen(line) > 8) {
				writes("That name is too long.");
				break;
			}
			strcpy(ubuf.u_login, line);
			break;
		case 'D':
			writef("Default Access: Deny access, Guest, Messages, Files, System? ");
			cmd = readc();
			log("Dft Access: %c", cmd);
			switch (cmd) {
			case 'G':
				ubuf.u_llen = A_GUEST;
				break;
			case 'D':
				ubuf.u_llen = A_NONE;
				break;
			case ' ':
				break;
			case 'M':
				ubuf.u_llen = A_USER;
				break;
			case 'S':
				ubuf.u_access = A_SYSTEM;
				break;
			case 'F':
				ubuf.u_access = A_FILES;
				break;
			default:
				writes("What?  Valid commands are D, G, M, F, or S.  Default access unchanged.");
			}
			break;
		case 'Q':
			if (okcmds[1] != '\0') {
				writef("Abort user examine, are you sure? N\b");
				cmd = readc();
				log("Abort? %c", cmd);
				if (cmd != 'Y')
					break;
			}
			return 1;
		case 'F':
			if (!crypted) {
				writes("Encrypting password, please wait...");
				strcpy(ubuf.u_pass, crypt(ubuf.u_pass, ubuf.u_pass) + 2);
			}
			putuser(uname, &ubuf);
			if (s_cmp(uname, user.u_name) == 0)
				user = ubuf;
			return 1;
		case 'L':
			writef("Enter new lines/screen, 0-66: ");
			reads(line);
			log("Lines/screen: %s", line);
			if (line[0] == '\0')
				break;
			ubuf.u_lines = atoi(line);
			if (ubuf.u_lines < 0 || ubuf.u_lines > 66) {
				ubuf.u_lines = 24;
				writes("Garbage lines/screen; using 24.");
			}
			break;
		case '?':
			writef("\nThe commands are self-documenting.  If you don't see a command, it's because you can't perform that command on the current user.\n");
			break;
		default:
			writef("What?  Please enter one of ");
			for (p = okcmds; *p != '\0'; p++) {
				writec(*p);
				writec(',');
				writec(' ');
			}
			writes("or ? for help.");
		}
	}
}

getparms() {
 	char home[512], line[512], var[20], sval[50];
 	FILE *cfp;
 	short cnt, pos, scnt, canon;
 	
#ifdef NOAUTOPATH
	strcpy(home, NOAUTOPATH);
#else
 	strcpy(home, getpwuid(geteuid())->pw_dir);
#endif NOAUTOPATH
	strcpy(parms.ua_home, home);
 	strcpy(line, home);
 	strcat(line, "/");
 	strcat(line, CONFIG);
 	if ((cfp = fopen(line, "r")) == NULL) {
 		fprintf(stderr, "panic: param get, %s\n", line);
 		exit(1);
 	}
 	while (fgets(line, 512, cfp) != NULL) {
 		line[strlen(line) - 1] = '\0';
 		if (Index(line, '#') != NULL)
 			*(Index(line, '#')) = '\0';
 		scnt = 0;
 		pos = 0;
 		while (line[pos] != '\0' && line[pos] != ' ' && line[pos] != '\t')
 			var[scnt++] = line[pos++];
 		var[scnt] = '\0';
 		if (var[0] == '\0')
 			continue;
 		for (cnt = 0; sysparms[cnt].parmname != NULL; cnt++)
 			if (strcmp(sysparms[cnt].parmname, var) == 0)
 				break;
 		if (sysparms[cnt].parmname == NULL) {
 			fprintf(stderr, "Please inform the sysop that there is an invalid parameter\n%s in the setup file.\n", var);
 			continue;
 		}
		while (line[pos] == ' ' || line[pos] == '\t')
			pos++;
 		switch (sysparms[cnt].parmtype) {
 			case NUM:
 				*((char *) sysparms[cnt].parmval) = atoi(&line[pos]) & 0xff;
 				break;
 			case BOOL:
 				if (line[pos] == '\0' || ToLower(line[pos]) == 'y')
 					*((char *) sysparms[cnt].parmval) = 1;
 				else
 					*((char *) sysparms[cnt].parmval) = 0;
 				break;
 			case STR:
 				canon = 0;
 				if (line[pos] == '"') {
 					canon = 1;
 					pos++;
 				}
 				for (scnt = 0; (canon? line[pos] != '"': line[pos] != '\0' && line[pos] != ' ' && line[pos] != '\t'); pos++, scnt++) {	
 					if (canon && line[pos] == '\\') {
 						switch (line[++pos]) {	
 							case 'n':
 								sval[scnt] = '\n';
 								break;
 							case 't':
 								sval[scnt] = '\t';
 								break;
 							case 'r':
 								sval[scnt] = '\r';
 								break;
 							case 'b':
 								sval[scnt] = '\b';
 								break;
 							case 'f':
 								sval[scnt] = '\f';
 								break;
 							case 'e':
 							case 'E':
 								sval[scnt] = '\033';
 								break;
 							case 'a':
 								sval[scnt] = '\7';	/* proposed extension of C string metasyntax */
 								break;
 							case '0':
 							case '1':
 							case '2':
 							case '3':
 							case '4':
 							case '5':
 							case '6':
 							case '7':
 								sval[scnt] = 0;
 								while (Index("01234567", line[pos]) != NULL)
 									sval[scnt] = sval[scnt] * 8 + (line[pos++] - '0');
								pos--;
								break;
							default:
								sval[scnt] = line[pos];
 						}
 					}
 					else
 						sval[scnt] = line[pos];
 				}
 				sval[scnt] = '\0';
 				strcpy(sysparms[cnt].parmval, sval);
 		}
 	}
}
