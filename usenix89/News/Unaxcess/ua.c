/*
 *	@(#)ua.c	1.1 (TDI) 7/18/86 21:01:57
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:57 by brandon
 *     Converted to SCCS 07/18/86 21:01:57
 *	@(#)Copyright (C) 1984, 85, 86 by Brandon S. Allbery.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:57 by brandon
 *     Converted to SCCS 07/18/86 21:01:57
 *
 *	@(#)This file is part of UNaXcess version 0.4.5.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:57 by brandon
 *     Converted to SCCS 07/18/86 21:01:57
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)ua.c	1.1 (TDI) 7/18/86 21:01:57";
static char _UAID_[]   = "@(#)UNaXcess version 0.4.5";
#endif lint

#include "ua.h"

struct cmd
    {
    char c_ch;				/* command character */
    char c_desc[33];			/* command description */
    int (*c_exec)();			/* command executive */
    }
    ;					/* used for command array */

/* forward references for command executives */

extern int
	readmsg(),	readnew(),	confidx(),	enter(),
	join(),		killmsg(),	helpme(),	scanmsg(),
	logout(),	bulletin(),	linelen(),	shell(),
	userctl(),	userlist(),	qscan(),	udl(),
	unsub(),	setlconf();

struct cmd cmdt[] =
    {
    '?', "Print help messages",			helpme,
    'a', "Alter or examine a user",		userctl,
    'b', "Reprint login bulletins",		bulletin,
    'c', "Shell command access",		shell,
    'd', "Set default login conference",	setlconf,
    'e', "Enter a message",			enter,
    'f', "File area (Downloading)",		udl,
    'g', "Exit UNaXcess",			logout,
    'h', "Print help messages",			helpme,
    'i', "Index of conferences",		confidx,
    'j', "Join a new conference",		join,
    'k', "Kill a message",			killmsg,
    'l', "Set line length",			linelen,
    'n', "Read all new messages",		readnew,
    'q', "Quick scan of messages",		qscan,
    'r', "Read messages in a conference",	readmsg,
    's', "Scan messages",			scanmsg,
    'u', "Unsubscribe from a conference",	unsub,
    'w', "List of UNaXcess users",		userlist,
    NULL,NULL,					NULL
    };

int nopause;
jmp_buf cmdloop;

main(argc, argv)
    char **argv;
    {
    char line[256], *lp;
    short lcnt;
    FILE *tp;

    getparms();
    chdir(parms.ua_home);
    logon();
    if (parms.ua_hco == 1) {
    	printf("\nDo you wish me to stop every few lines to let you read messages (Y)? ");
    	gets(line);
    	log("Pause? %s", line);
    }
    if (parms.ua_hco == 0 || (parms.ua_hco == 1 && ToLower(line[0]) != 'n'))
    	nopause = 0;
    else
    	nopause = 1;
    alarm(parms.ua_tlimit * 60);		/* time limit */
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, quit);
    for (lcnt = 4; lcnt < SIGUSR1; lcnt++)	/* we don't muck with others */
	signal(lcnt, logsig);
    signal(SIGALRM, thatsall);
    if (parms.ua_bnr[0] == '\0')
	puts("\nWelcome to UNaXcess Version 0.04.04\nCopyright (C) 1984, 1985 by Brandon Allbery");
    else
    	cat(parms.ua_bnr);
    if (argc > 2)
	{
	puts("To run UNaXcess from the shell, type `ua' or `ua username'.\nIf username has spaces or shell metacharacters in it, quote it.\n");
	log("Invoked with %d arguments.  Goodbye.", argc);
	exit(1);
	}
    else
	argc--;
    if (parms.ua_bbs[0] != '\0' && strcmp(getlogin(), parms.ua_bbs) == 0) {

nouser:
        for (lcnt = 0; lcnt != 3; lcnt++) {
 		if (argc) {
			strcpy(line, argv[1]);
	    		argc--;
			putchar('\n');
 		}
		else {
	    		if (parms.ua_login[0] == 0)
				printf("\nEnter your user name, GUEST, OFF, or NEW: ");
			else
				fputs(parms.ua_login, stdout);
			gets(line);
		}
	log("Login: %s", line);
	if (line[0] == '\0')
	    {
	    lcnt--;
	    continue;
	    }
	for (lp = line; *lp != '\0'; lp++)
	    *lp = ToLower(*lp);
	if (strcmp(line, "off") == 0)
	    {
	    puts("Goodbye...\n\n");
	    log("Logout.");
	    exit(0);
	    }
	if (!getuser(line, &user))
	    {
	    printf("No such user.\n");
	    log("No such user.");
	    }
	else if (user.u_pass[0] != '\0')
	    {
	    strcpy(line, getpass("Enter your password: "));
	    log("Password: %s", line);
	    puts("\nChecking password...");
	    if (strcmp(crypt(line, line) + 2, user.u_pass) == 0)
		break;
	    }
	else
	    break;
	}
    if (parms.ua_nla > 0 && lcnt == parms.ua_nla)
	{
	puts("\nSorry, you blew it.");
	log("Program aborted.");
	exit(1);
	}
    }
    else if (!getuser(getlogin(), &user))
    	goto nouser;
    log("%s, access = %d, sys = %s, line = %d", user.u_name, user.u_access, user.u_login, user.u_llen);
    if (user.u_access == A_NONE)
	{
	puts("Your access privileges have been revoked.  Goodbye...\n\n");
	log("Security violation:  access revoked.");
	exit(1);
	}
    if ((tp = fopen(RIndex(ttyname(fileno(stdin)), '/') + 1, "w")) == NULL)
	{
	log("Error %d opening %s", errno, RIndex(ttyname(fileno(stdin)), '/') + 1);
	log("Non-interactive session not logged to terminal.");
	}
    else {
	fprintf(tp, "%s on as \"%s\" on %s\n", getlogin(), user.u_name, longdate());
	fclose(tp);
    }
    putchar('\n');
    if (user.u_access != A_MKUSER)
	bulletin(NULL);
    umask(0);					/* so xedit() works */
    if (user.u_lconf[0] != '\0')
        if (isconf(user.u_lconf))
            strcpy(conference, user.u_lconf);
        else {
            putchar('\n');
            for (lp = parms.ua_sysop; *lp != '\0'; lp++)
                putchar(ToUpper(*lp));
            printf(" deleted \"%s\", your login conference.  I'm setting you\nback to the \"general\" conference.\n", user.u_lconf);
            user.u_lconf[0] = '\0';
            strcpy(conference, "general");
        }
    else
        strcpy(conference, "general");
    hicnts = readhigh(&user);
    cleanhigh();	/* kill any lingering corpses */
    if (!setjmp(cmdloop))
	signal(SIGINT, intrp);
    while (cmd())
	;
    printf("Goodbye, ");
    for (lp = user.u_name; *lp != '\0'; lp++)
	putchar(ToUpper(*lp));
    printf(".  Call again soon!\n\n\n");
    log("Logout.");
    cleanup();
    }

cleanup()
    {
    char tmps[256];
    FILE *fp;

    sprintf(tmps, "%s/himotd", MOTD);
    if ((fp = fopen(tmps, "r")) == NULL)
	{
	log("Error %d opening %s", errno, tmps);
	panic("himotd");
	}
    fgets(tmps, 32, fp);
    fclose(fp);
    user.u_nbull = atoi(tmps);
    putuser(user.u_name, &user);
    unlink(RIndex(ttyname(fileno(stdin)), '/') + 1);
    exit(0);
    }

cmd()
    {
    char line[256], *p;
    struct cmd *cmdp;

    if (user.u_access == A_MKUSER) {
	newuser();
	if (user.u_access == A_NONE) {
	    puts("\nYou'll have to be validated before you can use UNaXcess.");
	    return 0;
	}
    }
    printf("\n(%s) Command (? = Help): ", conference);
    gets(line);
    log("Command: %s", line);
    if (line[0] == '\0')
	return 1;
    for (p = line; *p != '\0'; p++)
	*p = ToLower(*p);
    for (cmdp = cmdt; cmdp->c_ch != NULL; cmdp++)
	if (ToLower(cmdp->c_ch) == line[0])
	    return (*cmdp->c_exec)(line);
    puts("Type '?' for help.");
    log("No such command.");
    return 1;
    }

logout()
    {
    char line[256];

    printf("Are you sure you want to log out (N)? ");
    gets(line);
    log("Logout? %s", line);
    return (ToLower(line[0]) != 'y');
    }

helpme()
    {
    short lcnt;
    struct cmd *cmdp;

    putchar('\n');
    lcnt = 2 * ((user.u_llen < 80) + 1);
    for (cmdp = cmdt; cmdp->c_ch != NULL; cmdp++)
	{
	printf("%c - %-32.32s", ToUpper(cmdp->c_ch), cmdp->c_desc);
	lcnt++;
	if (user.u_llen < 80 || !(lcnt % 2))
	    putchar('\n');
	if ((lcnt / 2) % (user.u_llen >= 80? 32: 16) == 0)
	    if (!cont())
		break;
	}
    if (user.u_llen >= 80 && lcnt % 2 != 0)
    	putchar('\n');
    puts("\nIf you need further help, look for (or ask for) a HELP conference.");
    return 1;
    }

linelen(s)
    char *s;
    {
    int llen;
    char line[256], *p;

    p = s;
    while (*p != '\0')
	if (*p++ == ' ')
	    if ((llen = atoi(p)) > 40 && llen < 132)
		{
		printf("New line length = %d.\n", llen);
		user.u_llen = llen;
		putuser(user.u_name, &user);
		return 1;
		}
	    else
		break;
    llen = 0;
    while (llen < 40 || llen > 132)
	{
	printf("\nEnter new line length (40-132): ");
	gets(line);
	llen = atoi(line);
	}
    return 1;
    }

cont()
    {
    char ch;

    if (!isatty(0) || nopause)
	return 1;
    printf("More (Y)? ");
    silent();
    ch = getchar();
    talk();
    log("Cont? %c", ch);
    printf("\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b");
    return (int) (RIndex(" Yy\r\n", ch) != NULL);
    	/* the above cast is because Plexus pcc is stupid! */
    }

cat(file)
    char *file;
    {
    FILE *f;
    char ch;
    short lcnt, ccnt;

    if ((f = fopen(file, "r")) == NULL)
	{
	log("Error %d opening %s", errno, file);
	puts("Cannot open file.");
	return;
	}
    lcnt = ccnt = 0;
    while ((ch = getc(f)) != EOF)
	{
	if (ch == '\n')
	    {
	    putchar(ch);
	    ccnt = 0;
	    if (++lcnt % 16 == 0)
		if (!cont())
		    break;
	    }
	else if (ch == '\t')
		putchar('\t');
	else
	    {
	    if (iscntrl(ch))
		putchar('.');
	    else
		putchar(ch);
	    if (++ccnt == (user.u_llen<40? 80: user.u_llen) - 1)
		{
		ccnt = 0;
		putchar('\n');
		if (++lcnt % 16 == 0)
		    if (!cont())
			break;
		}
	    }
	}
    fclose(f);
    }
