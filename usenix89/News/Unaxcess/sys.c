/*
 *	@(#)sys.c	1.1 (TDI) 7/18/86 21:01:53
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:53 by brandon
 *     Converted to SCCS 07/18/86 21:01:53
 *	@(#)Copyright (C) 1984, 85, 86 by Brandon S. Allbery.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:53 by brandon
 *     Converted to SCCS 07/18/86 21:01:53
 *
 *	@(#)This file is part of UNaXcess version 0.4.5.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:53 by brandon
 *     Converted to SCCS 07/18/86 21:01:53
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)sys.c	1.1 (TDI) 7/18/86 21:01:53";
static char _UAID_[]   = "@(#)UNaXcess version 0.4.5";
#endif lint

#include "ua.h"

static FILE *lfp;

short critical = 0;

short quitc = 0;
short intr = 0;
short alrm = 0;

short shhh = 0;

short warned = 0;

#ifdef SYS3
#include <sys/ioctl.h>
#include <termio.h>
struct termio mode;
#else
#include <sgtty.h>
#ifndef V7
#include <sys/ioctl.h>
#endif
struct sgttyb mode;
#endif

logon()
    {
    struct stat sb;
    char *cp;

    	/* first set up ttymode structure */
#ifdef SYS3
    ioctl(0, TCGETA, &mode);
#else
#ifdef V7
    gtty(0, &mode);
#else
    ioctl(0, TIOCGETP, &mode);
#endif
#endif

    if (parms.ua_env) {
    	if ((cp = getenv("SHELL")) != NULL)
    		strcpy(parms.ua_shell, cp);
    	if ((cp = getenv("EDITOR")) != NULL)
    		strcpy(parms.ua_edit, cp);
    }
    
    if (!parms.ua_log || stat(LOG, &sb) < 0)		/* no logfile => no logging */
	{
	lfp = NULL;
	return;
	}
    if ((lfp = fopen(LOG, "a")) == NULL)
	{
	perror(LOG);
	puts("panic: log");
	exit(2);
	}
    }

log(fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9)
    char *fmt, *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9;
    {
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
    int sig;
    {
    log("Received signal %d.", sig);
    fprintf(stderr, "\n\nUNaXcess internal error: %d.\n", sig);
    unlink(RIndex(ttyname(fileno(stdin)), '/') + 1);
    signal(SIGIOT, SIG_DFL);
    abort();
    }

panic(s)
    char *s;
    {
    log("panic: %s", s);
    fprintf(stderr, "panic: %s\n", s);
    unlink(RIndex(ttyname(2), '/') + 1);
    exit(1);
    }

quit()
    {
    char line[256];

    if (critical) {
    	quitc++;
    	return;
    }
    puts("\n\nFast logout\n");
    signal(SIGQUIT, quit);
    log("Signalled QUIT.");
    printf("\nDo you really want to leave UNaXcess (N)? ");
    gets(line);
    if (ToLower(line[0]) == 'y')
	{
	printf("OK, %s.  See you later!\n\n\n", user.u_name);
	cleanup();
	}
    }

intrp()
    {
    if (critical) {
    	intr++;
    	return;
    }
    puts("\n\nAborted.");
    log("Command aborted.");
    signal(SIGINT, intrp);
    longjmp(cmdloop, 1);
    }

char *visible(s)
    char *s;
    {
    static char vs[256];
    char *sp, *vp;

    vp = vs;
    for (sp = s; *sp != '\0'; sp++)
	if (!iscntrl(*sp))
	    *vp++ = *sp;
	else
	    {
	    *vp++ = '^';
	    *vp++ = uncntrl(*sp);
	    }
    *vp = '\0';
    return vs;
    }

shell()
    {
    short sig;
    unsigned altime;

    if (user.u_access == A_GUEST || user.u_access == A_USER || parms.ua_shell[0] == '\0')
	{
	puts("You don't have shell access privileges.");
	log("Security violation:  Unauthorized SHELL");
	return 1;
	}
    switch (fork())
	{
	case -1:
	    log("Error %d forking shell", errno);
	    puts("Sorry, the system's full.  Try again later.");
	    return 1;
	case 0:
	    for (sig = 2; sig < SIGUSR1; sig++)
		signal(sig, SIG_DFL);
	    setuid(getuid());
	    chdir(getpwuid(getuid())->pw_dir);
	    run(parms.ua_shell, 0);
	    log("Error %d exec'ing %s", errno, parms.ua_shell);
	    puts("Couldn't run the shell.");
	    exit(1);
	default:
	    CRIT();
	    for (sig = 2; sig < SIGUSR1; sig++)
		signal(sig, SIG_IGN);
	    signal(SIGALRM, thatsall);	/* trapped by the CRIT() */
	    wait(NULL);
	    signal(SIGINT, intrp);
	    signal(SIGQUIT, quit);
	    for (sig = 4; sig < SIGUSR1; sig++)
		signal(sig, logsig);
	    signal(SIGALRM, thatsall);
	    NOCRIT();
	}
    return 1;
    }

thatsall()
    {
    if (critical) {
    	alrm++;
    	return;
    }
    if (warned) {
        log("Timeout.");
        puts("\nI'm sorry, but you're out of time.\n\n");
        cleanup();
        }
    else 
        {
    	log("5-minute warning.");
    	puts("\nYou have only five minutes left in this session.\n\n");
    	warned = 1;
    	alarm(5 * 60);
        }
    }

/*
 * I've had problems with this.  If it breaks, delete the innards of the lock
 * functions.  Hopefully, I got it right this time...
 */

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
        if (now - statbuf.st_atime > 60) {
            unlink(lockpath);
            break;
        }
    }
    if ((lock_fd = creat(lockpath, 0600)) < 0) {
        fprintf(stderr, "Errno = %d creating lockfile %s\n", errno, lockpath);
        exit(-1);
    }
    close(lock_fd);
}

rmlock(lockfile)
char *lockfile; {
    char lockpath[50];
    struct stat statbuf;
    
    strcpy(lockpath, "lock/");
    strcat(lockpath, lockfile);
    if (stat(lockpath, &statbuf) < 0) {
        log("Lockfile %s deleted???", lockpath);
        printf("\n\nSomeone futzed with the lockfile.  Please tell %s IMMEDIATELY!!!\nSorry, but this means I have to log you out now.\n\n", parms.ua_sysop);
        panic("LOCKFILE DELETED");
    }
    if (unlink(lockpath) < 0) {
        log("Errno = %d, can't unlink lockfile %s", errno, lockpath);
        puts("\nI've got a lockfile problem.  You won't be able to do some\nthings until it's fixed.  Sorry...\n");
    }
}

xedit(file)
    char *file;
    {
    short sig;
    unsigned altime;

    if (user.u_access == A_GUEST || user.u_access == A_USER || parms.ua_edit[0] == '\0')
	{
	puts("You don't have shell access privileges.");
	log("Security violation:  Unauthorized XEDIT");
	return 1;
	}
    if (strcmp(parms.ua_edit, "ua-edit") == 0) {
    	edit(file);
    	return 1;
    }
    switch (fork())
	{
	case -1:
	    log("Error %d forking shell", errno);
	    puts("Sorry, the system's full.  Using the line editor...");
	    edit(file);
	    return 1;
	case 0:
	    for (sig = 2; sig < SIGUSR1; sig++)
		signal(sig, SIG_DFL);
	    setuid(getuid());
	    chdir(getpwuid(getuid())->pw_dir);
	    run(parms.ua_edit, file);
	    log("Error %d exec'ing %s", errno, parms.ua_edit);
	    puts("Couldn't run the editor; using the line editor...");
	    edit(file);
	    exit(0);
	default:
	    CRIT();
	    for (sig = 2; sig < SIGUSR1; sig++)
		signal(sig, SIG_IGN);
	    signal(SIGALRM, thatsall);
	    wait(NULL);
	    signal(SIGINT, intrp);
	    signal(SIGQUIT, quit);
	    for (sig = 4; sig < SIGUSR1; sig++)
		signal(sig, logsig);
	    signal(SIGALRM, thatsall);
	    NOCRIT();
	}
    return 1;
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

silent() {
	if (shhh)
		return;
#ifdef SYS3
	mode.c_lflag &= ~(ICANON|ISIG|ECHO|ECHOE|ECHOK);
	mode.c_cc[VMIN] = 1;
	mode.c_cc[VTIME] = 0;
	ioctl(0, TCSETAW, &mode);
#else
	mode.sg_flags |= CBREAK;
	mode.sg_flags &= ~ECHO;
#ifdef V7
	stty(0, &mode);
#else
	ioctl(0, TIOCSETP, &mode);
#endif
#endif
	shhh = 1;
}

talk() {
	if (!shhh)
		return;
#ifdef SYS3
	mode.c_lflag |= (ICANON|ISIG|ECHO|ECHOE|ECHOK);
	mode.c_cc[VEOF] = CEOF;
	mode.c_cc[VEOL] = CNUL;
	ioctl(0, TCSETAW, &mode);
#else
	mode.sg_flags |= ECHO;
	mode.sg_flags &= ~CBREAK;
#ifdef V7
	stty(0, &mode);
#else
	ioctl(0, TIOCSETP, &mode);
#endif
#endif
	shhh = 0;
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
