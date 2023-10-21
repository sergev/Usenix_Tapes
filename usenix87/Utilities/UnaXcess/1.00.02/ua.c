/*
 *	@(#)ua.c	1.2 (TDI) 2/3/87
 *	@(#)Copyright (C) 1984, 85, 86, 87 by Brandon S. Allbery.
 *	@(#)This file is part of UNaXcess version 1.0.2.
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)ua.c	1.2 (TDI) 2/3/87";
static char _UAID_[]   = "@(#)UNaXcess version 1.0.2";
#endif lint

#include "ua.h"

/* forward references for command executives */

extern int
	readmsg(),	readnew(),	confidx(),	enter(),
	join(),		killmsg(),	scanmsg(),	msgmenu(),
	logout(),	bulletin(),	shell(),	edrest(),
	alter(),	userlist(),	qscan(),	udl(),
	unsub(),	setlconf(),	readmenu(),	mkbull(),
	adminmenu(),	validate();

struct cmd maincmd[] = {
	'a', "Administration menu",		adminmenu,
	'b', "Reprint login bulletins",		bulletin,
	'c', "Shell command access",		shell,
	'd', "Set default login conference",	setlconf,
	'f', "File area (Downloading)",		udl,
	'g', "Exit UNaXcess",			logout,
	'm', "UNaXcess Message Base",		msgmenu,
	'w', "List of UNaXcess users",		userlist,
	'\0',"help/mainmenu",			NULL
};

struct cmd admincmd[] = {
	'a', "Alter or examine a user",		alter,
	'b', "Enter a bulletin",		mkbull,
	'c', "Shell command access",		shell,
	'e', "Edit conference membership",	edrest,
	'g', "Exit UNaXcess",			logout,
	'v', "Validate new users",		validate,
	'w', "List of UNaXcess users",		userlist,
	'x', "Return to the Main Menu",		(int (*)()) 0,
	'\0',"help/adminmenu",			NULL
};

struct cmd msgcmd[] = {
	'd', "Set default login conference",	setlconf,
	'e', "Enter a message",			enter,
	'g', "Exit UNaXcess",			logout,
	'i', "Index of conferences",		confidx,
	'j', "Join a new conference",		join,
	'k', "Kill a message",			killmsg,
	'n', "Read all new messages",		readnew,
	'r', "Read commands menu",		readmenu,
	'u', "Unsubscribe from a conference",	unsub,
	'x', "Return to the Main Menu",		(int (*)()) 0,
	'\0',"help/msgbase",			NULL
};

struct cmd readcmd[] = {
	'g', "Exit UNaXcess",			logout,
	'n', "Read all new messages",		readnew,
	'q', "Quick scan of messages",		qscan,
	'r', "Read messages in a conference",	readmsg,
	's', "Scan messages",			scanmsg,
	'x', "Return to the Message Base Menu",	(int (*)()) 0,
	'\0',"help/readcmds",			NULL
};

jmp_buf cmdloop;
int __recurse;

main(argc, argv)
char **argv; {
	char line[256], *lp;
	short lcnt;
	FILE *tp;

	getparms();
	chdir(parms.ua_home);
	if (parms.ua_env && (lp = getenv("SHELL")) != NULL)
		strcpy(parms.ua_shell, lp);
	__recurse = 0;
	logon();
	io_on(1);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, quit);
	for (lcnt = SIGILL; lcnt <= SIGTERM; lcnt++)	/* we don't muck with others */
		signal(lcnt, logsig);
	signal(SIGALRM, thatsall);
	/* OOPS!  1 lines deleted, 2 lines changed 2/3/87 ++bsa */
	writes("\nWelcome to UNaXcess Conferencing, Version 1.00.02\nCopyright (C) 1984, 1985, 1986 by Brandon S. Allbery");
	if (parms.ua_bnr[0] != 0)
		cat(parms.ua_bnr);
	if (argc > 2) {
		writes("To run UNaXcess from the shell, type `ua' or `ua username'.\nIf username has spaces or shell metacharacters in it, quote it.\n");
		log("Invoked with %d arguments.  Goodbye.", argc);
		exit(1);
	}
	else
		argc--;
	if (ismember(getlogin(), parms.ua_bbs)) {

nouser:
		for (lcnt = 0;; lcnt++) {
	 		if (argc) {
				strcpy(line, argv[1]);
				argc--;
				writec('\n');
	 		}
			else {
				if (parms.ua_login[0] == 0)
					writef("\nEnter your user name, GUEST, OFF, or NEW: ");
				else
					writef("%s", parms.ua_login);
				reads(line);
			}
			log("Login: %s", line);
			if (line[0] == '\0') {
				lcnt--;
				continue;
			}
			for (lp = line; *lp != '\0'; lp++)
				*lp = ToLower(*lp);
			if (strcmp(line, "off") == 0) {
				writes("Goodbye...\n\n");
				log("Logout.");
				exit(0);
			}
			if (!getuser(line, &user)) {
				writef("No such user.\n");
				log("No such user.");
			}
			else if (user.u_pass[0] != '\0') {
				xecho();
				writef("Enter your password: ");
				reads(line);
				doecho();
				log("Password: %s", line);
				writes("\nChecking password...");
				if (strcmp(crypt(line, line) + 2, user.u_pass) == 0)
					break;
			}
			else
				break;
		}
		if (parms.ua_nla > 0 && lcnt == parms.ua_nla) {
			writes("\nSorry, you blew it.");
			log("Program aborted.");
			exit(1);
		}
	}
	else if (!getuser(getlogin(), &user))
		goto nouser;
	if (s_cmp(user.u_name, parms.ua_sysop) != 0)
		alarm(parms.ua_tlimit * 60);		/* time limit */
	log("%s, access = %d, sys = %s, line = %d", user.u_name, user.u_access, user.u_login, user.u_llen);
	if (user.u_access == A_NONE) {
		writes("Your access privileges have been revoked.  Goodbye...\n\n");
		log("Security violation:  access revoked.");
		exit(1);
	}
	if ((tp = fopen(RIndex(ttyname(fileno(stdin)), '/') + 1, "w")) == NULL) {
		log("Error %d opening %s", errno, RIndex(ttyname(fileno(stdin)), '/') + 1);
		log("Non-interactive session not logged to terminal.");
	}
	else {
		fprintf(tp, "%s on as \"%s\" on %s\n", getlogin(), user.u_name, longdate());
		fclose(tp);
	}
	writec('\n');
	if (user.u_access != A_MKUSER)
		bulletin();
	if (user.u_lconf[0] != '\0')
		if (isconf(user.u_lconf))
			if (isrcmem(user.u_name, user.u_lconf))
				strcpy(conference, user.u_lconf);
			else {
				writef("\nYour login conference, \"%s\",  has been restricted and you are not a member.  I'm moving you back to \"general\".\n", user.u_lconf);
				strcpy(user.u_lconf, "general");
				strcpy(conference, "general");
			}
		else {
			writef("\n%s deleted \"%s\", your login conference.  I'm setting you back to the \"general\" conference.\n", upstr(parms.ua_sysop), user.u_lconf);
			strcpy(user.u_lconf, "general");
			strcpy(conference, "general");
		}
	else
		strcpy(conference, "general");
	if (user.u_access != A_GUEST && user.u_access != A_MKUSER) {
		hicnts = readhigh(&user);
		cleanhigh();	/* kill any lingering corpses */
	}
	if (!setjmp(cmdloop))
		signal(SIGINT, intrp);
	if (user.u_access == A_MKUSER) {
		newuser();
		if (user.u_access == A_NONE) {
			writes("\nYou'll have to be validated before you can use UNaXcess.");
			return 0;
		}
	}
	pcmd("Command (? for help): ", maincmd, (char *) 0);
	writef("Goodbye");
	if (user.u_name[0] != '\0')
		writef(", %s", upstr(user.u_name));
	writef(".  Call again soon!\n\n\n");
	log("Logout.");
	cleanup();
}

cleanup() {
	char tmps[256];
	FILE *fp;

	sprintf(tmps, "%s/himotd", MOTD);
	if ((fp = fopen(tmps, "r")) == NULL) {
		log("Error %d opening %s", errno, tmps);
		panic("himotd");
	}
	fgets(tmps, 32, fp);
	fclose(fp);
	user.u_nbull = atoi(tmps);
	putuser(user.u_name, &user);
	unlink(RIndex(ttyname(fileno(stdin)), '/') + 1);
	io_off();
	exit(0);
}

logout() {
	char line[256];

	if (__recurse > 0) {
		__recurse--;
		return 0;
	}
	writef("Are you sure you want to log out? N\b");
	line[0] = readc();
	log("Logout? %c", line[0]);
	if (line[0] != 'Y')
		return 1;
	writef("Goodbye");
	if (user.u_name[0] != '\0')
		writef(", %s", upstr(user.u_name));
	writef(".  Call again soon!\n\n\n");
	log("Logout.");
	cleanup();
}

msgmenu() {
	pcmd("Message Base Command (? for help, X to exit): ", msgcmd, "Message Base");
	return 1;
}

readmenu() {
	pcmd("Read Command (? for help, X to exit): ", readcmd, "Read Commands");
	return 1;
}

adminmenu() {
	pcmd("Admin Command (? for help, X to exit): ", admincmd, "Administration");
	return 1;
}

pcmd(prompt, cmdtab, previous)
char *prompt, *previous;
struct cmd cmdtab[]; {
	char ch;
	struct cmd *cmd;
	short fullhelp;
	unsigned int tleft;
	
	fullhelp = 0;
	for (;;) {
		if ((tleft = alarm((unsigned int) 0)) != 0) {
			alarm(tleft);
			writef("%d:%02d ", tleft / 60, tleft % 60);
		}
		writef("[%s] %s", conference, prompt);
		ch = readc();
		if (ch == '?') {
			if (fullhelp) {
				for (cmd = cmdtab; cmd->c_cmd != '\0'; cmd++)
					;
				writec('\n');
				cat(cmd->c_desc);
				writec('\n');
				continue;
			}
			writef("\n%s Commands\n\n", (previous == (char *) 0? "Main": previous));
			for (cmd = cmdtab; cmd->c_cmd != '\0'; cmd++)
				writef("  %c - %s\n", ToUpper(cmd->c_cmd), cmd->c_desc);
			fullhelp = 1;
			writes("\nType ? for the full help file.\n");
			continue;
		}
		for (cmd = cmdtab; cmd->c_cmd != '\0'; cmd++)
			if (ToUpper(cmd->c_cmd) == ch)
				break;
		if (cmd->c_cmd == '\0') {
			writes("Unrecognized command.  Type ? for help.");
			continue;
		}
		if (cmd->c_func == (int (*)()) 0)
			break;
		if ((*cmd->c_func)() == 0)
			break;
		fullhelp = 0;
	}
}
