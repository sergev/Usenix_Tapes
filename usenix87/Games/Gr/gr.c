/*
 *                  GR - A  G A M E   R E G U L A T O R
 *                          for Berkeley UNIX
 *
 */

#ifndef lint
static char *RCSid = "$Header: gr.c,v 1.32 86/12/02 20:26:57 mcooper Locked $";
#endif

/*
 *------------------------------------------------------------------
 *
 * $Source: /big/src/usc/bin/gr/RCS/gr.c,v $
 * $Revision: 1.32 $
 * $Date: 86/12/02 20:26:57 $
 * $State: Exp $
 * $Author: mcooper $
 * $Locker: mcooper $
 *
 *------------------------------------------------------------------
 *
 * Michael A. Cooper (mcooper@oberon.USC.EDU)
 * University Computing Services,
 * University of Southern California
 *
 *------------------------------------------------------------------
 * $Log:	gr.c,v $
 * Revision 1.32  86/12/02  20:26:57  mcooper
 * #ifdef'd all dprintf's.
 * 
 * Revision 1.31  86/08/06  20:07:55  mcooper
 * Yet another attempt at fixing the setuid
 * problem.  Think I got it this time!
 * 
 * Revision 1.30  86/08/05  12:54:22  mcooper
 * Fixed setuid() problem (again).
 * 
 * Revision 1.29  86/07/30  12:22:30  mcooper
 * If DEBUG is defined, use a SLEEPTIME of 5
 * to speed things up.
 * 
 * Revision 1.28  86/07/17  16:13:04  mcooper
 * All fatal errors now use fatal().
 * 
 * Revision 1.27  86/07/17  14:49:21  mcooper
 * - Fixed bug in freettys() that caused
 *   was_freettys to be TRUE when there
 *   where exactly cf->cf_nofreettys free.
 * - Tweaked was_freettys user message a bit.
 * 
 * Revision 1.26  86/07/15  14:53:36  mcooper
 * Determine user name in setup() instead of
 * everytime logfile() is called.
 * 
 * Revision 1.25  86/07/14  19:03:45  mcooper
 * Log more info to a log file.
 * 
 * Revision 1.24  86/07/14  15:53:53  mcooper
 * Moved most of the compile time configuration
 * options to the control file.
 * 
 * Revision 1.23  86/07/03  15:54:41  mcooper
 * Now checks for NOGAMING file along with the
 * other tests in case game playing is turn off
 * while we're playing.
 * 
 * Revision 1.22  86/07/03  14:25:07  mcooper
 * - Major work on "special" ttys.  
 *   - Bug fix in deltty().
 *   - Changed are #define macros.
 *   - Added findtty() (in findtty.c) to try and
 *     determine the users real tty.  (4.2bsd)
 * - Added debug mode.
 * - Misc. - Can't remember them all.
 * 
 * Revision 1.21  86/06/18  13:46:47  mcooper
 * Implemented BAD_TTYS - Check to see if
 * the user is on a BAD_TTYS.  If so, game
 * playing is not permited at all.  Also
 * cleaned up #ifdef TOD stuff.
 * 
 * Revision 1.20  86/06/04  12:21:23  mcooper
 * Use SP_NAME for name of special ttys when
 * printing messages to the user.
 * 
 * Revision 1.19  86/06/02  13:31:53  mcooper
 * More cleanup on SP_TTYS.
 * 
 * Revision 1.18  86/05/30  21:33:10  mcooper
 * Added support for "special" ttys.  i.e. only a limited
 * number of SP_TTYS.
 * 
 * Revision 1.17  86/05/14  16:08:44  mcooper
 * Cleaned up and somewhat de-linted.
 * 
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <pwd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/file.h>
#include <utmp.h>
#include <nlist.h>

#include <sys/vm.h>
#include <machine/pte.h>

#include "gr.h"

#ifdef DEBUG
int sleeptime = 5;
#else
int sleeptime = SLEEPTIME;
#endif

int hastried = 0;

int Fswap;
int Fpmem;
int Fkmem;
int Futmp;

char *mytty;
char *user;
char *prog = "GR";

static char *badtty;
#ifdef FINDTTY
struct pte *usrptmap, *usrpt;	/* Hold values of _Usrptmap & _usrpt */
int i_proc, i_nproc;			/* Hold values of _proc & _nproc */
#endif

int was_freettys;
int was_badttys;
int was_tod;
int was_load;
int was_nogames;
int was_users;

static double avenrun[3];
static char *game;
char mbuf[BUFSIZ];


char *warnmesg1 = "to save your game.  If you do not leave\r\nthe game ";
char *warnmesg2 = "within this period, your game will be terminated.";

char *warntime[3] = {
	"4 minutes ",
	"2 minutes ",
	"1 minute "
};


static struct utmp buf;
static struct nlist nl[] = {
#define N_AVENRUN	0
	{ "_avenrun" },
#ifdef FINDTTY
#define N_PROC		1				/* Address of process table */
	{ "_proc" },
#define N_NPROC		2				/* Length of process table */
	{ "_nproc" },
#define N_USRPTMAP 	3
	{ "_Usrptmap" },
#define N_USRPT 	4
	{ "_usrpt" },
#endif
	{ "" },
};

char *rindex();
char *deltty();
char *gettyname();
char *getaline();
long time();

int debug = 0;
struct cfinfo *cf, *parse();

/*VARARGS*/
main(argc, argv)
int argc;
char **argv;
{
	game = rindex(argv[0], '/');
	if (game == 0)
		game = argv[0];
	else
		game++;	/* Skip the '/' */

	if (strcmp(game, "debug") == 0)
		debug = 1;

	if (debug) {
		setbuf(stdout, 0);
#ifdef DEBUG
		dprintf("debug is ON\n");
#endif DEBUG
		sleeptime = 5;
	}

	setup(game);

	if (checkfor(cf->cf_nogames) || tod() || badttys() || freettys() || 
	  users() || load()) {
		fprintf(stderr, "Sorry, no games now.\n");
		if (was_nogames)
			fprintf(stderr, getaline(cf->cf_nogames));
		if (was_freettys)
			fprintf(stderr, "There %s not %d %s tty%c available.\n",
				(cf->cf_nofreettys > 1) ? "are" : "is", cf->cf_nofreettys,
				cf->cf_spname, (cf->cf_nofreettys > 1) ? 's' : 0);
		if (was_badttys)
			fprintf(stderr, 
			"Game playing is not permitted on tty%s at any time.\n",
				badtty);
		if (was_load)
			fprintf(stderr, 
				"The system load is greater than %.2lf.\n",
				cf->cf_load);
		if (was_users)
			fprintf(stderr, 
				"There are more than %d users logged in.\n",
				cf->cf_users);
		if (was_tod)
			fprintf(stderr, 
"Game playing is not permitted between %d00 and %d00 hours on weekdays.\n", 
				cf->cf_todmorn, cf->cf_todeven);
		exit(1);
	}
	if (cf->cf_logfile[0]) {
		sprintf(mbuf, "startup of %s (%d)", game, getpid());
		logfile(mbuf, cf->cf_logfile);
	}
	play(game, argv);
}

play(game, args)
char *game;
char **args;
{
	int pid;
	char tmp[BUFSIZ];

	switch (pid = fork()) {
		case -1:
			fprintf(stderr, "Cannot fork.\n");
			exit(1);
		case 0:
			(void) strcpy(tmp, cf->cf_hidedir);
			(void) strcat(tmp, "/");
			(void) strcat(tmp, game);
			(void) signal(SIGINT, SIG_DFL);
			(void) signal(SIGQUIT, SIG_DFL);
			(void) signal(SIGTSTP, SIG_DFL);
#ifdef DEBUG
			dprintf("before: getuid = %d\n", getuid());
			dprintf("before: geteuid = %d\n", geteuid());
#endif DEBUG
			(void) setuid(getuid());
#ifdef DEBUG
			dprintf("after: getuid = %d\n", getuid());
			dprintf("after: geteuid = %d\n", geteuid());
#endif DEBUG
			if(setpriority(PRIO_PROCESS, 0, cf->cf_priority) < 0) {
				fatal("setpriority", TRUE);
				/*NOTREACHED*/
			}
			execv(tmp, args);
			fatal(tmp, TRUE);
			/*NOTREACHED*/
	}
	for (;;) {
		sleep(sleeptime);
		if (load() || checkfor(cf->cf_nogames) || tod() || freettys() || 
		  users()) {
			warn(0);
			sleep(sleeptime);
			if (reprieve())
				continue;
			sleep(sleeptime);
			if (reprieve())
				continue;
			warn(1);
			sleep(sleeptime);
			if (reprieve())
				continue;
			warn(2);
			sleep(sleeptime);
			if (reprieve())
				continue;
			blast(pid);
		}
	}
}

reprieve()
{
	static char mess[] = "\r\nYou have a reprieve.  Continue playing.\r\n";

	if (checkfor(cf->cf_nogames) || load() || tod() || freettys() || users())
		return (0);
	(void) write(2, mess, sizeof(mess));
	return (1);
}

/*
 * warn - Warn the user how close his game is to ending.
 */
warn(which)
int which;
{
	char buf1[BUFSIZ];
	char buf2[BUFSIZ];
	char *space = "                                                        ";
	extern errno;

#ifdef DEBUG
	dprintf("warn(): which = %d\n", which);
#endif DEBUG

	if (was_nogames)
		(void) sprintf(buf2, getaline(cf->cf_nogames));
	if (was_load)
		(void) sprintf(buf2, 
			"The system load is greater than %.2lf.",
			cf->cf_load);
	if (was_users)
		(void) sprintf(buf2, 
			"There are more than %d users logged in.",
				cf->cf_users);
	if (was_freettys)
		(void) sprintf(buf2, "There %s not %d %s tty%s available.",
			(cf->cf_nofreettys > 1) ? "are" : "is", cf->cf_nofreettys,
			cf->cf_spname, (cf->cf_nofreettys > 1) ? "s" : "");
	if (was_tod)
		(void) sprintf(buf2, "Game time is over.");

	(void) sprintf(buf1, 
			"\r\n\007\007\007%s\r\n%s\r\nYou have %s%s%s\r\n%s\r\n",
			space, buf2, warntime[which], warnmesg1, warnmesg2, space);
	(void) write(2, buf1, strlen(buf1));
}

/*
 * blast - Kill the game.
 */
blast(pid)
int pid;
{
	static char mess[] = "\rYour game is forfeit.\r\n";

#ifdef DEBUG
	dprintf("blast(): pid = %d myuid = %d(%d)\n", pid, getuid(), geteuid());
#endif DEBUG

	if(!hastried)
		(void) write(2, mess, sizeof(mess));

	if (cf->cf_logfile[0]) {
		(void) sprintf(mbuf, "blast(%d) %s", pid, game);
		(void) logfile(mbuf, cf->cf_logfile);
	}

	if(kill(pid, SIGHUP) < 0) {
		(void) sprintf(mbuf, "kill(%d, SIGHUP) failed: %d", pid, errno);
		(void) logfile(mbuf, cf->cf_logfile);
#ifdef DEBUG
		dprintf("%s: ", mbuf);
		if(debug)perror();
#endif DEBUG
	}
	sleep(30);
	if(kill(pid, SIGKILL) < 0) {
		(void) sprintf(mbuf, "kill(%d, SIGKILL) failed: %d", pid, errno);
		(void) logfile(mbuf, cf->cf_logfile);
#ifdef DEBUG
		dprintf("%s: ", mbuf);
		if(debug)perror();
#endif DEBUG
	}
	if(!hastried) {
#ifdef DEBUG
		dprintf("_before: getuid = %d\n", getuid());
		dprintf("_before: geteuid = %d\n", geteuid());
#endif DEBUG
		(void) setuid(getuid());
#ifdef DEBUG
		dprintf("_after: getuid = %d\n", getuid());
		dprintf("_after: geteuid = %d\n", geteuid());
#endif DEBUG
		hastried = 1;
		blast(pid);
		return;
	} else
		hastried = 0;
}

/*
 * death - More on killing.
 */
death()
{
	union wait status;

	if (wait3(&status, WNOHANG | WUNTRACED, (char *)0) == 0) {
		return;
	}
	if (status.w_stopval == WSTOPPED) {
		(void) signal(SIGTSTP, SIG_DFL);
		(void) kill(getpid(), SIGTSTP);
		(void) signal(SIGTSTP, SIG_IGN);
		return;
	} else {
#ifdef DEBUG
		dprintf("\nClosing with exit(0)...\n");
#endif DEBUG
		if(cf->cf_logfile[0]) {
			(void) sprintf(mbuf, "end of %s (%d)", game, getpid());
			(void) logfile(mbuf, cf->cf_logfile);
		}
		(void) close(Fkmem);
		(void) close(Fpmem);
		(void) close(Fswap);
		(void) close(Futmp);
		exit(0);
	}
}

/*
 * setup - Setup constants.
 */
setup(game)
char *game;
{
	FILE *list;
	char *getlogin();
	struct passwd *pw, *getpwuid();

	Fswap = Fpmem = Fkmem = Futmp = 0;

	if ((Futmp = open("/etc/utmp", 0)) < 0) {
		fatal("/etc/utmp", TRUE);
		/*NOTREACHED*/
	}
	if ((Fkmem = open("/dev/kmem", 0)) < 0) {
		fatal("/dev/kmem", TRUE);
		/*NOTREACHED*/
	}
	if ((Fpmem = open("/dev/mem", 0)) < 0) {
		fatal("/dev/mem", TRUE);
		/*NOTREACHED*/
	}
	if ((Fswap = open("/dev/drum", 0)) < 0) {
		fatal("/dev/drum", TRUE);
		/*NOTREACHED*/
	}

	nlist("/vmunix", nl);
	if (nl[0].n_type == 0) {
		fatal("/vmunix", TRUE);
		/*NOTREACHED*/
	}

#ifdef FINDTTY
	usrptmap = (struct pte *) nl[N_USRPTMAP].n_value;
	usrpt = (struct pte *) nl[N_USRPT].n_value;

#ifdef DEBUG
	dprintf("seeking _nproc...\n");
#endif DEBUG
	memseek(Fkmem, (long) nl[N_NPROC].n_value);
	if (read(Fkmem, (char *) &i_nproc, sizeof i_nproc) != sizeof i_nproc) {
		fatal("Could not get value of nproc.", FALSE);
		/*NOTREACHED*/
	}
#ifdef DEBUG
	dprintf("seeking _proc...\n");
#endif DEBUG
	memseek(Fkmem, (long) nl[N_PROC].n_value);
	if (read(Fkmem, (char *) &i_proc, sizeof i_proc) != sizeof i_proc) {
		fatal("Could not get value of proc.", FALSE);
		/*NOTREACHED*/
	}
#endif
	mytty = gettyname();
#ifdef DEBUG
	dprintf("mytty = '%s'\n", mytty);
#endif DEBUG
	
	if ((list = fopen(CONTROL, "r")) == NULL) {
		fatal(CONTROL, TRUE);
		/*NOTREACHED*/
	}

	if ((user = getlogin()) == NULL) {
		if ((pw = getpwuid(getuid())) != NULL)
			user = pw->pw_name;
		else
			user = "(unknown)";
#ifdef DEBUG
		dprintf("getlogin() failed. user = '%s'\n", user);
#endif DEBUG
	}
#ifdef DEBUG
		else dprintf("getlogin() succeeded. user = '%s'\n", user);
#endif DEBUG

	cf = parse(list, game);

#ifdef DEBUG
	dprintf("game = '%s'\tmax load = '%.2f'\tmax users = '%d'\tpriority = %d\n",
			cf->cf_game, cf->cf_load, cf->cf_users, cf->cf_priority);
	dprintf("badttys = '%s'\tfreettys = '%s'\nnofree = %d\tsp_name = '%s'\n", 
		cf->cf_badttys, cf->cf_freettys, cf->cf_nofreettys, cf->cf_spname);
	dprintf("tod morn = %d \t even = %d\n", cf->cf_todmorn, cf->cf_todeven);
#endif DEBUG
		
	(void) fclose(list);
	(void) signal(SIGCHLD, death);
	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);
	(void) signal(SIGTSTP, SIG_IGN);
}

/*
 * load - Return the 5 minute load average
 */
load()
{
	(void) lseek(Fkmem, (long)nl[N_AVENRUN].n_value, 0);
	(void) read(Fkmem, avenrun, sizeof(avenrun));
	return (was_load = (avenrun[1] >= cf->cf_load));
}

/*
 * users - Count the number of users logged in.
 */
users()
{
	int count = 0;

	(void) lseek(Futmp, (long)0, 0);
	while (read(Futmp, &buf, sizeof(buf)) > 0) {
		if (buf.ut_name[0] != '\0') {
			count++;
		}
	}
	return (was_users = (count > cf->cf_users));
}

/*
 * tod - Find out if it is okay to play right now.
 */
tod()
{
	long now;
	struct tm *localtime();
	struct tm *ntime;

	if (cf->cf_todmorn == 0 && cf->cf_todeven == 0) {
#ifdef DEBUG
		dprintf("tod(): tod not set.\n");
#endif DEBUG
		return(was_tod = FALSE);
	}

	time(&now);
	ntime = localtime(&now);
	if(ntime->tm_wday == 0 || ntime->tm_wday == 6)
		return(was_tod = FALSE);/* OK on Sat & Sun */
	if(ntime->tm_hour < cf->cf_todmorn || ntime->tm_hour >= cf->cf_todeven)
		return(was_tod = FALSE);/* OK during non working hours */
	
	return(was_tod = TRUE);
}

checkfor(file)
char *file;
{
#ifdef DEBUG
	dprintf("checkfor(%s) called\n", file);
#endif DEBUG

	was_nogames = access(file, R_OK) == 0;
#ifdef DEBUG
	dprintf("checkfor(): access = %d\n", was_nogames);
#endif DEBUG
	return(was_nogames);
}

/*
 * getaline - Returns the first line from file.
 */
char *
getaline(file)
char *file;
{
	char buf[BUFSIZ];
	FILE *fd, *fopen();

	if((fd = fopen(file, "r")) != NULL) {
		fgets(buf, sizeof(buf), fd);
		(void) fclose(fd);
		return(buf);
	}
	return(NULL);
}

/*
 * freettys - Find out if there are enough free ttys.
 *		The user invoking a game (under GR) must be on
 *		one of the ttys listed in cf->cf_freettys for this check.
 */
freettys()
{
	int left = 0;
	char *remttys;

	remttys = cf->cf_freettys;
	if(strlen(remttys) < 2)
		return;

#ifdef DEBUG
	dprintf("freeyttys started. ttys = '%s'\n", remttys);
#endif DEBUG

	/*
	 * If you're not on a "special" tty, we don't
	 * worry about you.
	 */
	if(!sgrep(mytty, remttys)) {
#ifdef DEBUG
		dprintf("You're NOT on a special tty.\n");
#endif DEBUG
		return(0);
	}

#ifdef DEBUG
	dprintf("You're on a special tty.\n");
#endif DEBUG

	(void) lseek(Futmp, (long)0, 0);
	while (read(Futmp, &buf, sizeof(buf)) > 0) {
		if (buf.ut_name[0] != '\0') {
			if(strncmp(buf.ut_line, "tty", 3) == 0)
				remttys = deltty(buf.ut_line+3, remttys);
		}
	}

	left = strlen(remttys) / 2;
#ifdef DEBUG
	dprintf("remaining ttys = '%s' # left = %d\n", remttys, left);
#endif DEBUG
	return(was_freettys = (left < cf->cf_nofreettys));
}

/*
 * deltty - delete a tty "name" from the remaining ttys
 */
char *
deltty(name, list)
char *name;
char *list;
{
	char tmp[BUFSIZ];
	register int i;

	i = 0;
	while(*list) {
		if (*list == *name) {
			if(*++list == *++name) {
				*++list;
			} else {
				*--list;
			}
			*--name;
		} 
		tmp[i++] = *list++;
	}
	tmp[i] = NULL;
	return(tmp);
}

/*
 * sgrep(x, str) -- check for x in str.  Return 1 (TRUE) if exists.
 *			Otherwise 0 (FALSE).
 */

sgrep(x, str)
char 	*x;
char 	*str;
{
	if (!str)
		return(FALSE);
	while(*str) {
		if(*str == *x){
			if(*++str == *++x) {
				return(TRUE);
			} else {
				*--x;
			}
		}
		*++str;
	}
	return(FALSE);
}

/*
 * badttys - Are we allowed to play games on this tty?
 */
badttys()
{
	if(sgrep(mytty, cf->cf_badttys)) {
		badtty = mytty;
		return(was_badttys = TRUE);
	}
	return(was_badttys = FALSE);
}

/*
 * gettyname - Determine which tty we're on.
 *             Returns 2 char string.
 */
char *
gettyname()
{
	char *ttyname();
#ifdef FINDTTY
	char *findtty();
	char *t, *t2;

#ifdef DEBUG
	dprintf("gettyname(): calling findtty()...\n");
#endif DEBUG
	t = findtty(getpid());
	if (strcmp(t, "co") == 0) {	/* We're probably  using rlogin or telnet */
		t2 = ttyname(2)+8;
#ifdef DEBUG
		dprintf("t2 = '%s'\n", t2);
#endif DEBUG
		if (strcmp(t, t2)) { 	/* there's a difference: so believe ttyname() */
#ifdef DEBUG
			dprintf("gettyname: difference.  using t2\n");
#endif DEBUG
			t = t2;
		}
	}
	return(t);
#else
#ifdef DEBUG
	dprintf("gettyname(): using ttyname()...\n");
#endif DEBUG
	return(ttyname(2)+8);
#endif
}

fatal(msg, pflag)
char *msg;
int pflag;
{
	fprintf(stderr, "%s: ", prog);
	if (pflag)
		perror(msg);
	else
		fprintf(stderr, "Fatal Error: %s\n", msg);
	exit(1);
}
