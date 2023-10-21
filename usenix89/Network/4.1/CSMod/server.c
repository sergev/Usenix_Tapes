#ifndef	lint
static char sccsid[] = "@(#) $Header: server.c,v 1.6 86/06/09 15:27:57 sob Exp $";
#endif
#include <signal.h>
#include <stdio.h>
#include <ctype.h>

/*
 * Generic simple TCP server program, to be called by inetd.
 * Gathers a line of input, converts it into arguments,
 * checks command name against COMMANDS, execs command.
 * Like rshd, except doesn't require authorization,
 * and is limited to a small number of commands.
 *
 * COMMANDS ordinarly consists of one command name per line.
 * There may be other words on the same line, in any order:
 *	login	the connection is logged in wtmp.
 *	local	the foreign host must be in the same domain as this host.
 * If COMMANDS can't be found, the default is man, with no options.
 *
 * Comments may be introduced with # at the beginning of a word.
 * This works both in the input line, and in COMMANDS.
 */
#ifdef MASSCOMP
#define COMMANDS "/etc/net/server.cmds"
#else
#define COMMANDS "/etc/server.cmds"
#endif
#define MAXFULLNAME 50
#ifdef MASSCOMP
FILE * fp;
char PATH[MAXFULLNAME] = "PATH=/bin:/usr/bin:/usr/lbin:/usr/local";
#else
char PATH[MAXFULLNAME] = "PATH=/bin:/usr/bin:/usr/ucb:/usr/local";
#endif
char Shell[MAXFULLNAME];
char HOME[MAXFULLNAME];
char USERNAME[MAXFULLNAME];
char LOGNAME[MAXFULLNAME];
#ifdef MASSCOMP
char tzone[MAXFULLNAME];
#endif
extern char ** environ;
char *nenv[] = {
	PATH,
	Shell,
	HOME,
	USERNAME,
	LOGNAME,
#ifdef MASSCOMP
	tzone,
#endif
	0
};

static
int	logit = 0, local = 0;
static
char *otherd = NULL;

main(argc, argv)
int argc;
char **argv;
{
	extern char *checkname();
	int pid, wpid, status;
	char	**args, *what, *who;
	extern char *them();

	if (dup2(0, 1) < 0 || dup2(1, 2) < 0) {
		perror("dup2");
		_exit(1);
	}
	otherd = checkname(*argv);
	if (getargs(&args) < 1) {
		fprintf(stderr,
		"server:  No input arguments:  don't know what to do.\r\n");
		exit(1);
	}
	what = *args;
#ifdef MASSCOMP
	tzone[0] = '\0';
	fp = fopen("/etc/tz","r");
	if (fp != NULL)
	{
	char  zt[BUFSIZ];
	if (fgets(zt,BUFSIZ,fp) !=NULL)
		sprintf(tzone,"TZ=%s",&zt[0]);
	fclose(fp);
	}
	if (tzone[0] == '\0')
		strcpy(tzone,"TZ=CST6CDT");
#endif
	strcpy(HOME, "HOME=/tmp");
	strcpy(Shell, "SHELL=/bin/sh");
	strcpy(USERNAME,"USER=daemon");
	strcpy(LOGNAME,"LOGNAME=daemon");
	environ = nenv; /* force use if our environment */
	if (otherd == NULL && !checkcommand(what))
		exit(1);
	if (!logit) {
		doit(what, args);
		exit(-1);
	}
	who = them(argv[1]);
	if (local && !checkdomain(who)) {
		fprintf (stderr, "%s\r\n",
			"server:  Sorry, only local access permitted.");
		sleep(3);
		exit(1);
	}
	dologin(what, "tcp", who);
	status = 0;
	switch ((pid = fork())) {
	case -1:	/* error */
		perror("fork");
		dologout(1);
		break;
	case 0:		/* child */
		doit(what, args);
		dologout(-1);
		break;
	default:	/* parent */
		while ((wpid = wait(&status)) != pid) {
			if (wpid == -1) {
				perror("wait");
				status = -1;
				break;
			}
		}
		if (status == 0)
			dologout(0);
		else
			dologout(1);
		break;
	}
	(void)close(1);
	(void)close(2);
	(void)close(0);
	exit(status);
}

doit(what, args)
char *what, **args;
{
	(void)setuid(1);	/* setuid daemon, in case inetd didn't */

	execvp (what, args);
	perror ("exec");
	(void)fprintf (stderr, "\r\nserver:  %s unavailable\r\n", what);
}

char *
checkname(name)
char *name;
{
	char *cp, *rindex();
	register int length;

	if ((cp = rindex(name, '/')) != NULL)
		name = cp;
	if ((length = strlen(name)) > 0 && name[--length] == 'd') {
		name[length] = '\0';
		logit++;
		return(name);
	}
	return(NULL);
}

#include <setjmp.h>
static
jmp_buf jb;
static
sigalarm()
{
	longjmp(jb, 1);
}

/*
 * Collect a line of input from stdin, break it into arguments, return it.
 * Arguments are put in *pargv, and their number is the returned value.
 */
getargs(pargv)
char ***pargv;
{
	static char *arglist[32], namebuf[512];
	register int nread;
	register char *line;

	arglist[0] = NULL;
	*pargv = arglist;
	(void) signal (SIGALRM, sigalarm);
	if(setjmp(jb)) {
		fprintf(stderr, "server:  Connection timed out.\r\n");
		return(0);
	}
	(void) alarm (60);
	for (line = namebuf; line < &namebuf[sizeof(namebuf)-1]; line++) {
		if ((nread = getc(stdin)) == EOF)
			break;
		if (nread == '\n' || nread == '\r')
			break;
		*line = nread;
	}
	*line = '\0';
	(void)alarm (0);
	if (nread < 0)
		perror("read error");
	if (nread <= 0)
		return(0);
	nread = sizeof(arglist) / sizeof(arglist[0]);
	if (otherd != NULL) {	/* stuff the command name in as first arg */
		**pargv = otherd;
		(*pargv)++;
		nread--;
	}
	nread = argcargv(namebuf, *pargv, nread);
	if (nread >= 0 && otherd) {
		nread++;
		*pargv = arglist;
	}
	return(nread);
}

/*
 * Take a line of input, a pointer (argv) to space for args, and max of same.
 * Return actual number of arguments (argc) parsed from line buffer into argv.
 * The argument strings themselves are still in the line buffer,
 * which is modified to null terminate them.  The character # at the beginning
 * of an argument makes it and the rest of the line a comment, which is ignored.
 */
int
argcargv(line, argv, nargv)
register char *line, **argv;
int nargv;
{
	register char **argtop;
	register int argc = 0;

	for (argtop = &argv[nargv - 1]; argv < argtop; argv++) {
		while (*line && isspace (*line))
			line++;		/* skip leading white space */
		if (*line == '\0')
			break;		/* end of line */
		if (*line == '#')
			break;		/* comment to end of line */
		*argv = line;
		argc++;
		while (*line && !isspace(*line))
			line++;		/* save an argument */
		if (*line != '\0')
			*line++ = '\0';	/* null terminate it */
	}
	*argv = (char *)0;	/* NULL terminate argv */
	return(argc);
}

checkcommand(command)
char *command;
{
	extern char *gets();
	FILE *fp;
	char *argv[16], buffer[128];
	int argc;

	if ((fp = fopen(COMMANDS, "r")) == NULL) {
		perror(COMMANDS);
		if (strcmp (command, "man") == 0) {
			logit = 1;
			return(1);
		}
		fprintf(stderr, "server:  No go, mate.\r\n");
		return(0);
	}
	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		argc = argcargv(buffer, argv, sizeof(argv)/sizeof(argv[0]));
		if (argc <= 0)
			continue;
		if (strcmp(command, argv[0]) == 0) {
			int i;

			for (i = 1; i < argc; i++) {
				if (strcmp(argv[i], "login") == 0) {
					logit = 1;
					continue;
				}
				if (strcmp(argv[i], "local") == 0) {
					local = 1;
					continue;
				}
			}
			(void)fclose(fp);
			return(1);
		}
	}
	fprintf(stderr, "server:  Can't do that.\r\n");
	(void)fclose(fp);
	return(0);
}

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/* #include <arpa/inet.h> */
#include <netdb.h>

/*
 * Find the name of the foreign host.
 * SMI inetd passes the address and socket in an argument.
 * Berkeley inetd expects program to get it with getpeername.
 */
char *them(arg)
char *arg;
{
	static char remotehost[128];
	extern char *index(), *inet_ntoa();
	extern u_long inet_addr();
	struct sockaddr_in theirsock;
	int theirlen = sizeof theirsock;
#define addr theirsock.sin_addr
	struct hostent *hp;

	if (arg != 0) {
		if (index(arg, '.') != 0)
			*index(arg, '.') = '\0';
		(void)strcpy (remotehost, "0x");
		(void)strncat(remotehost, arg, sizeof(remotehost) - 1);
		addr.s_addr = inet_addr(remotehost);
		(void)strncpy(remotehost, inet_ntoa(addr),
			sizeof(remotehost) - 1);
	} else
		if (getpeername(0, &theirsock, &theirlen) == -1)
			perror ("getpeername");
	if ((hp = gethostbyaddr(&addr, sizeof(addr), AF_INET)) != 0)
		(void)strncpy(remotehost, hp -> h_name, sizeof(remotehost) - 1);
	return(remotehost);
}

/*
 * Ensure target host is in the same domain as the local host.
 */
checkdomain(machine)
char *machine;
{
	extern char *index();
	char hostname[256];
	char *mydomain, *theirdomain;
	struct hostent *hp;

	if (gethostname(hostname, sizeof(hostname)) != 0) {
		perror("gethostname");
		return(0);
	}
	if ((hp = gethostbyname(hostname)) == NULL) {
		perror("gethostbyname");
		return(0);
	}
	if ((mydomain = index(hp -> h_name, '.')) == NULL) {
		fprintf(stderr, "server:  No domain in primary name %s of %s.\n",
			hp -> h_name, hostname);
		return(0);
	}
	if ((theirdomain = index(machine, '.')) == NULL) {
		fprintf(stderr, "server: No domain in %s.\n", machine);
		return(0);
	}
	if (casecmp (mydomain, theirdomain) != 0)
		return(0);
	return(1);
}

casecmp(one, two)
register char *one, *two;
{
	register int first, second;

	while (*one && *two) {
		first = *one;
		if (islower(first))
			first = toupper(first);
		second = *two;
		if (islower(second))
			second = toupper(second);
		if (first != second)
			break;
		one++;
		two++;
	}
	if (*one == *two)
		return(0);
	if (*one > *two)
		return(1);
	return(-1);
}

#include <utmp.h>
#include <sys/file.h>

#define	SCPYN(a, b)	(void)strncpy(a, b, sizeof (a))
struct	utmp utmp;

/*
 * Record login in wtmp file.
 */
dologin(name, tty, remotehost)
	char *name, *tty, *remotehost;
{
	int wtmp;
	char line[32];

	wtmp = open("/usr/adm/wtmp", O_WRONLY|O_APPEND);
	if (wtmp >= 0) {
		/* hack, but must be unique and no tty line */
		(void)sprintf(line, "%s%d", tty, getpid());
		SCPYN(utmp.ut_line, line);
		SCPYN(utmp.ut_name, name);
		SCPYN(utmp.ut_host, remotehost);
		utmp.ut_time = time((char *)0);
		(void) write(wtmp, (char *)&utmp, sizeof (utmp));
		(void) close(wtmp);
	}
}

/*
 * Record logout in wtmp file.
 */
dologout(status)
	int status;
{
	int wtmp;

#ifdef notdef
	if (!logged_in)
		_exit(status);
#endif
	wtmp = open("/usr/adm/wtmp", O_WRONLY|O_APPEND);
	if (wtmp >= 0) {
		SCPYN(utmp.ut_name, "");
		SCPYN(utmp.ut_host, "");
		utmp.ut_time = time((char *)0);
		(void) write(wtmp, (char *)&utmp, sizeof (utmp));
		(void) close(wtmp);
	}
}
