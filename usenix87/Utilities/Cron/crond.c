#if !defined(lint) && !defined(LINT)
static char rcsid[] = "$Header: crond.c,v 1.6 87/05/02 17:33:16 paul Exp $";
#endif

/* $Source: /usr/src/local/vix/cron/crond.c,v $
 * $Revision: 1.6 $
 * $Log:	crond.c,v $
 * Revision 1.6  87/05/02  17:33:16  paul
 * baseline for mod.sources release
 * 
 * Revision 1.5  87/03/31  00:23:08  paul
 * another GREAT idea from rs@mirror...
 *   do all time calculations in int instead of double -- the double
 *   stuff was for a previous kludge that turned out to be unneeded.
 *   time.c went away as of this change, also.
 * 
 * Revision 1.4  87/02/12  18:20:29  paul
 * fixed target_time scope misdesign, lots of fixes to sigchld stuff
 * 
 * Revision 1.3  87/02/10  18:26:38  paul
 * POKECRON, time_d(), target_time, other massive changes
 * 
 * Revision 1.2  87/02/02  19:25:43  paul
 * various
 * 
 * Revision 1.1  87/01/26  23:48:55  paul
 * Initial revision
 */

/* Copyright 1987 by Vixie Enterprises
 * All rights reserved
 *
 * Distribute freely, except: don't sell it, don't remove my name from the
 * source or documentation (don't take credit for my work), mark your changes
 * (don't get me blamed for your possible bugs), don't alter or remove this
 * notice.  Commercial redistribution is negotiable; contact me for details.
 *
 * Send bug reports, bug fixes, enhancements, requests, flames, etc., and
 * I'll try to keep a version up to date.  I can be reached as follows:
 * Paul Vixie, Vixie Enterprises, 329 Noe Street, San Francisco, CA, 94114,
 * (415) 864-7013, {ucbvax!dual,ames,ucsfmis,lll-crg,sun}!ptsfa!vixie!paul.
 */


#define	MAIN_PROGRAM


#include "cron.h"
#include <sys/time.h>
#include <sys/signal.h>
#include <sys/types.h>
#if defined(BSD)
# include <sys/wait.h>
# include <sys/resource.h>
#endif /*BSD*/

extern int	fprintf(), exit(), fork(), unlink(), sleep();
extern time_t	time();


void
usage()
{
	(void) fprintf(stderr, "usage:  %s [-x debugflag[,...]]\n", PROGNAME);
	(void) exit(ERROR_EXIT);
}


void
main(argc, argv)
	int	argc;
	char	*argv[];
{
	extern void	set_cron_uid(), be_different(), parse_args(),
			free_database(), sigchld_handler();
	extern user	*load_database();
	extern long	cron();

	register user	*database;
	register long	target_time;

	PROGNAME = argv[0];

	setlinebuf(stdout);
	setlinebuf(stderr);

	/* if there is only one argument, it's the program name.  this means
	 * that debugging can't be on (it would require '-x'), thus test-mode
	 * can't be on, thus we should fork and exit, to be compatible with
	 * the real cron.
	 */
	if (argc == 1)
	{
		if (fork() != 0)
			_exit(0);

		be_different();
	}
	else
	{
		(void) fprintf(stderr, "[%d] vcron started\n", getpid());
	}

#if defined(BSD)
	(void) signal(SIGCHLD, sigchld_handler);
#endif /*BSD*/

#if defined(ATT)
	(void) signal(SIGCLD, SIG_IGN);
#endif /*ATT*/

	parse_args(argc, argv);
	set_cron_uid();
	(void) unlink(POKECRON);
	target_time = (long) time((time_t*)0);
	while (TRUE)
	{
		database = load_database();
		target_time = cron(database, target_time);
		free_database(database);
	}
}


static long
cron(db, target_time)
	register user	*db;
	register long	target_time;
{
	extern int	unlink();
	extern void	cron_tick(), cron_sleep();

	/*
	 * if the POKECRON file exists (i.e., we can successfully delete
	 * it), return to the mainline, which will reread the database
	 * and call us again.
	 */
	while (OK != unlink(POKECRON))
	{
		/* do this iteration
		 */
		cron_tick(db, target_time);

		/* sleep 1 minute
		 */
		target_time += 60;
		cron_sleep(target_time);
	}
	Debug(DSCH, ("POKECRON file detected\n"))
	return target_time;
}


static void
cron_tick(db, target_time)
	user	*db;
	long	target_time;
{
	extern void		do_command();
	extern struct tm	*localtime();
	register struct tm	*tm = localtime((time_t*) &target_time);
	/**/ int		minute, hour, dom, month, dow;
	register user		*u;
	register entry		*e;

	minute = tm->tm_min -FIRST_MINUTE;
	hour = tm->tm_hour -FIRST_HOUR;
	dom = tm->tm_mday -FIRST_DOM;
	month = tm->tm_mon +1 /* 0..11 -> 1..12 */ -FIRST_MONTH;
	dow = tm->tm_wday -FIRST_DOW;

	Debug(DSCH, ("tick(%d,%d,%d,%d,%d)\n", minute, hour, dom, month, dow))

	/* the dom/dow situation is odd.  '* * 1,15 * Sun' will run on the
	 * first and fifteenth AND every Sunday;  '* * * * Sun' will run *only*
	 * on Sundays;  '* * 1,15 * *' will run *only* the 1st and 15th.  this
	 * is why we keep 'e->dow_star' and 'e->dom_star'.
	 */
	for (u = db;  u != NULL;  u = u->next)
		for (e = u->crontab;  e != NULL;  e = e->next)
			if (bit_test(e->minute, minute)
			 && bit_test(e->hour, hour)
			 && bit_test(e->month, month)
			 && ( (e->dom_star || e->dow_star)
			      ? (bit_test(e->dow,dow) && bit_test(e->dom,dom))
			      : (bit_test(e->dow,dow) || bit_test(e->dom,dom))
			    )
			   )
				do_command(e->cmd, u);
}


static void
cron_sleep(target_time)
	long	target_time;
{
	register int	seconds_to_wait;

	seconds_to_wait = (int) (target_time - (long) time((time_t*)0));
	Debug(DSCH, ("target_time=%ld, sec-to-wait=%d\n",
		target_time, seconds_to_wait))

	if (seconds_to_wait > 0)
	{
		Debug(DSCH, ("sleeping for %d seconds\n", seconds_to_wait))
		(void) sleep((unsigned int) seconds_to_wait);
	}
}


#if defined(BSD)
static void
sigchld_handler()
{
	union wait	waiter;
	int		pid;

	for (;;)
	{
		pid = wait3(&waiter, WNOHANG, (struct rusage *)0);
		switch (pid)
		{
		case -1:
			Debug(DPROC,
				("[%d] sigchld...no children\n", getpid()))
			return;
		case 0:
			Debug(DPROC,
				("[%d] sigchld...no dead kids\n", getpid()))
			return;
		default:
			Debug(DPROC,
				("[%d] sigchld...pid #%d died, stat=%d\n",
				getpid(), pid, waiter.w_status))
		}
	}
}
#endif /*BSD*/


static void
parse_args(argc, argv)
	int	argc;
	char	*argv[];
{
	extern	int	optind, getopt();
	extern	void	usage();
	extern	char	*optarg;

	int	argch;

	while (EOF != (argch = getopt(argc, argv, "x:")))
	{
		switch (argch)
		{
		default:
			usage();
		case 'x':
			if (!set_debug_flags(optarg))
				usage();
			break;
		}
	}
}
