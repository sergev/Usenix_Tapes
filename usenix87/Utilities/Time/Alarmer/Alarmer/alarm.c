/*
**  #define SUN ==> Suntools alarmer
**	   else ==> general alarmer
**
**	Sam Black
**	22 January, 1986
**
**  This program reads in an alarm database (default ~/.alarm) and
**  signals the user when an alarm goes off.
**
**  #define SUN ==> Flash the alarm bell icon.
**	   else ==> Ring the terminal's bell.
**
*/
#include <stdio.h>
#include <pwd.h>
#include <sgtty.h>
#include <ctype.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>

/*
**  define some useful macros
**  (DEB, when, otherwise, nitems, IOCTL, NE, FREENODE).
*/

#ifdef DEBUG
#	define DEB(x)	fprintf/**/x
#else
#	define DEB(x)
#endif DEBUG

#define when		break; case
#define otherwise	break; default
#define nitems(x)	(sizeof(x) / sizeof(x[0]))

#define IOCTL(op, par)			\
	if (ioctl(0, op, par) < 0)	\
	{				\
		perror("tty");		\
		exit(-1);		\
	}

#define NEW(t)	(t *) malloc(sizeof(t))
#define FREENODE			\
	{				\
		TIMES *tmp = alarms;	\
					\
		alarms = alarms->next;	\
		free(tmp);		\
	}

/*
**  Set up some default program parameters.
*/
#define CONFIG_FILE	".alarm"
#define RESET_FILE	".alarmreset"
#define RESET		"kill -%d %ld"
#define SIGREAD		SIGEMT

#ifdef SUN
#	define QUERY		"\033[11t"
#	define RESPONSE		"\033[%dt"
#	define ICONIC		2

#	define SIGNAL(x)	printf(alarm_/**/x)

#	ifndef ICON_DIR
#		define ICON_DIR	"/unc/black/icons"
#	endif ICON_DIR

#	define IMAGE		"\033]I%s/alarm_%s\033\134"
#	define IHEADER		"\033]L\033\134"
#else
#	define SIGNAL(x)
#endif SUN

/*
**  Time constants
*/
#define SPM	 60		/* seconds per minute */
#define SPH	(60 * SPM)	/* seconds per hour */
#define SPD	(24 * SPH)	/* seconds per day */
#define LOOK	(SPD >> 1)
#define JAN_86	504921600L	/* 1 January, 1986 */

/*
**  Types
*/
#define STRLEN		80
typedef char STRING[STRLEN];

typedef struct TIMES		/* the TIMES structure */
{
	int time;		/* how many seconds into the epoch */
	STRING message;
	struct TIMES *next;
} TIMES;

/*
**  Routines used
*/
static TIMES *read_alarm_file();
static TIMES *insert();
static long gettime();
static void usage();
static void lowercase();
static long date_to_time();
int reset_alarm();
long time();

#ifdef SUN
#	define REPEAT 512
#	define SHOWCOUNT 8
	static int iconic();
	static void setup_term();
	int reset_term();
#endif SUN

jmp_buf env;
char *pname;

/*
**  main():  process command line and loop on the database.
*/
main(argc, argv)
int argc;
char *argv[];
{
#ifdef SUN
	char *icon_dir;
	STRING	alarm_on,	/* alarm on icon (normal) */
		alarm_inv,	/* alarm on icon (inverted) */
		alarm_off;	/* alarm off icon */
#endif SUN
	STRING alarmdb;		/* alarm file name */
	STRING reset;		/* reset file name */
	int rfile;
	TIMES *alarms;		/* alarm structure */
	long getpid();
	char *getenv();
	struct passwd *getpwuid();
	struct passwd *p_entry;
	int history = -SPH;
	int i;

	pname = *argv;

	/*
	**  Get user's home directory (for ~/.alarm file).
	*/
	if ( (p_entry = getpwuid(getuid())) )
	{
		sprintf(alarmdb, "%s/%s", p_entry->pw_dir, CONFIG_FILE);
		sprintf(reset, "%s/%s", p_entry->pw_dir, RESET_FILE);
	}
	else
	{
		perror("getuid");
		exit(-1);
	}

	/*
	**  Take the arguments.  The only legal ones now are an
	**  optional database file and an optional history value.
	*/
	while (--argc)
		if (**++argv == '-')
			switch ((*argv)[1])
			{
				when 'h':  i = atoi(*++argv);
					   if ((i >= 0) && (i <= SPM))
						history = -60 * i;
					   else
						usage(pname);
					   --argc;
				otherwise: usage(pname);
			}
		else
			strcpy(alarmdb, *argv);


	/*
	**  Enable the user to signal this process
	*/
	if ((rfile = open(reset, O_TRUNC | O_WRONLY | O_CREAT, 0700)) == -1)
	{
		perror("open");
		exit(-1);
	}
	else
	{
		sprintf(reset, RESET, SIGREAD, getpid());
		write(rfile, reset, strlen(reset));
		close(rfile);
	}

	/*
	**  Read in the alarm file.
	*/
	DEB((stderr, "About to read alarm file\n"));
	alarms = read_alarm_file(alarmdb, history);

#ifdef SUN
	/*
	**  Set up the icons
	*/
	if (!(icon_dir = getenv("ICONDIR")))
		icon_dir = ICON_DIR;
	sprintf(alarm_on,  IMAGE, icon_dir, "on");
	sprintf(alarm_inv, IMAGE, icon_dir, "on2");
	sprintf(alarm_off, IMAGE, icon_dir, "off");

	/*
	**  Put up the initial icon and set up the tty driver.
	*/
	setup_term();
	printf(IHEADER);
	SIGNAL(off);
	fflush(stdout);

	/*
	**  Set up signal trapping (Note:  DO NOT trap SIGWINCH!!).
	*/
	for (i = SIGHUP; i <= SIGPROF; ++i)
		signal(i, reset_term);
#endif SUN

	/*
	**  Trap SIGREAD to re-read .alarm file.
	*/
	signal(SIGREAD, reset_alarm);
	if (setjmp(env))
	{
		while (alarms)
			FREENODE;
		alarms = read_alarm_file(alarmdb, 0);
		signal(SIGREAD, reset_alarm);
	}

	/*
	**  Repeat ad nauseum.
	*/
	for (;;)
	{
		/*
		**  Sleep until the next alarm.
		*/
		if (alarms->time > time(0))
		{
			DEB((stderr, "Sleeping for %ld seconds.\n",
				alarms->time - time(0)));
			sleep((int) (alarms->time - time(0)));
		}
		else
			DEB((stderr, "Not sleeping.\n"));

		/*
		**  Check to see if we need to re-read the file.
		*/
		if (!alarms->next)
		{
			FREENODE;
			alarms = read_alarm_file(alarmdb, 0);
		}
		else
		{
#ifdef SUN
			int times = 0;
			/*
			**  Indicate the alarm, and wait for the user
			**  to open the window.
			*/
			while (iconic())
			{
				if ((++times % REPEAT) < SHOWCOUNT)
				{
					SIGNAL(on);
					SIGNAL(inv);
				}
			}
#else
			printf("\007\007\007");
#endif SUN

			/*
			**  Dump ALL current alarm messages.
			*/
			putchar('\n');
			do
			{
				puts(alarms->message);
				FREENODE;
			} while (alarms->next && (alarms->time <= time(0)));

			/*
			**  Turn the alarm off.
			*/
			SIGNAL(off);
			fflush(stdout);
		}
	}
}

#ifdef SUN
struct sgttyb save_tty;

/*
**  setup_term(): set up tty driver for suntools query.
*/
static void
setup_term()
{
	struct sgttyb tty;

	IOCTL(TIOCGETP, &tty);
	save_tty = tty;

	tty.sg_flags &= ~ECHO;
	tty.sg_flags |= CBREAK;

	IOCTL(TIOCSETP, &tty);
}

/*
**  reset_term(): reset tty driver to its initial state.
*/
int
reset_term(sig)
int sig;
{
	IOCTL(TIOCSETP, &save_tty);
	exit(sig);
}
#endif SUN

/*
**  reset_alarm(): trap alarm signals.
*/
int
reset_alarm()
{
	longjmp(env);
}

int daily;
int cwday,	/* day of week */
    cday,	/* day of month */
    cmonth,	/* month of year */
    cyear;	/* year */
int TZ;		/* difference to GMT */

/*
**  read_alarm_file(): Read in alarm database.
*/
static TIMES *
read_alarm_file(fname, history)
STRING fname;
int history;
{
	FILE *fopen();
	struct tm *localtime();
	FILE *alarmdb;
	TIMES *alarms, *cur;
	char input[160];
	long ctime;		/* current time-of-day */
	long itime;		/* alarm entry time-of-day */
	long rtime;		/* relative to current time-of-day */
	struct tm *ltime;
	struct timeb tp;

	/*
	**  Open the alarm database
	*/
	if (!(alarmdb = fopen(fname, "r")))
	{
		perror(fname);
		usage(pname);
	}

	/*
	**  Calculate the current time of day.
	*/
	ftime(&tp);
	ctime = tp.time;
	DEB((stderr, "dstflag = %d\n", tp.dstflag));
	TZ = (tp.timezone * SPM) - (tp.dstflag * SPH);
	ltime = localtime(&ctime);
	DEB((stderr, "Current time = %ld\n", ctime));
	cmonth = ltime->tm_mon + 1;
	cday = ltime->tm_mday;
	cyear = ltime->tm_year;
	cwday = ltime->tm_wday - 7;
	DEB((stderr, "cmonth = %d, cday = %d, cyear = %d, cwday = %d\n",
		cmonth, cday, cyear, cwday));

	/*
	**  Read the alarms file and put each alarm
	**  in the structure, sorted by time.  Only
	**  alarms in the next LOOK seconds (default
	**  12 hours) are entered.
	*/
	alarms = NEW(TIMES);
	alarms->time = ctime + LOOK;
	strcpy(alarms->message, "12 hour timer interrupt");
	alarms->next = NULL;

	while (fgets(input, sizeof(input), alarmdb))
	{
		/*
		**  Parse the input line.
		*/
		input[strlen(input) - 1] = '\0';
		rtime = (itime = gettime(input)) - ctime;

		/*
		**  Fix daily alarms to be in the previous 12 hours
		**  or the last 12 hours, whichever is closer.
		*/
		if (daily)
		{
			if (rtime > LOOK)
			{
				rtime -= SPD;
				itime -= SPD;
			}
			else if (rtime < -LOOK)
			{
				rtime += SPD;
				itime += SPD;
			}
		}

		/*
		**  Only insert alarms that are in the next
		**  LOOK seconds, and are within the history
		**  limit (default 1/2 hour).
		*/
		if ((rtime < LOOK) && (rtime >= history))
		{
			cur = NEW(TIMES);
			cur->time = itime;
			strcpy(cur->message, input);

			alarms = insert(alarms, cur);
		}
	}

	fclose(alarmdb);
	traverse(alarms);
	return(alarms);
}

#define ISDAY(s) !strncmp(dow, s, strlen(dow))

/*
**  gettime(): Input line parser.
*/
static long
gettime(input)
char *input;
{
	int date[5];
	STRING dow;	/* for weekly or daily alarms */
	static STRING days[] =
	{
		"sundays",
		"mondays",
		"tuesdays",
		"wednesdays",
		"thursdays",
		"fridays",
		"saturdays"
	};

	daily = 0;
	/*
	**  Look for comments (blank lines, # lines)
	*/
	if ((sscanf(input, "%s", dow) != 1) || (*dow == '#'))
		return(0L);

	/*
	**  Look for a specific date.
	*/
	if (isdigit(*dow))
		sscanf(input, "%d/%d %d:%d",
			&date[0], &date[1], &date[3], &date[4]);

	/*
	**  Look for daily or weekly alarms.
	*/
	else
	{
		register int i;

		sscanf(input, "%s %d:%d", dow, &date[3], &date[4]);
		lowercase(dow);
		*date = cmonth;

		/*
		**  Daily.
		*/
		if (ISDAY("any") || ISDAY("daily"))
		{
			date[1] = cday;
			daily = 1;
		}
		/*
		**  Weekly.
		*/
		else
		{
			for (i = 0; i < nitems(days); ++i)
				if (ISDAY(days[i]))
				{
					date[1] = cday + ((i - cwday) % 7);
					break;
				}
			if (i == nitems(days))
			{
				fprintf(stderr,
					"'%s' is an invalid alarm entry.\n",
					input);
				return(0L);
			}
		}

		DEB((stderr, "periodic reminder: %s becomes %d/%d\n",
				dow, date[0], date[1]));
	}

	/*
	**  Take care of December 31.
	*/
	if ((date[0] > cmonth) || ((date[0] == cmonth) && (date[1] >= cday)))
		date[2] = cyear;
	else
		date[2] = cyear + 1;

	return(date_to_time(date));
}

/*
**  lowercase(): Convert string to lower case.
*/
static void
lowercase(str)
char *str;
{
	for ( ; *str; ++str)
		if (isupper(*str))
			*str = tolower(*str);
}

/*
**  date_to_time(): Take an array of
**
**		[month day year hour minute]
**
**  and return the number of seconds since the epoch.
**  (1 January, 1970)
*/
static long
date_to_time(date)
int *date;
{
	static int corr[] =
	{
		0, 1, -1, 0, 0, 1, 1, 2, 3, 3, 4, 4
	};
	int month  = *date - 1;
	int day  = *++date - 1;
	int year = *++date;
	int hour = *++date;
	int min = *++date;
	int leap = ((month > 1) && (dysize(1900 + year) == 366));
	register int ydisp = (30 * month + day + corr[month]) * SPD;
	register int i;

	if (leap)
		ydisp += SPD;

	/*
	**  Add in years since 1986.
	*/
	for (i = 86; i < year; ++i)
		ydisp += (SPD * dysize(1900 + i));

	DEB((stderr, "Time for %d/%d, %d at %d:%2d = %ld\n",
			month+1, day+1, year, hour, min,
			JAN_86 + TZ + ydisp + (hour * SPH) + (min * SPM)));

	/*
	**  Sum everything (including time zone adjustment).
	*/
	return(JAN_86 + TZ + ydisp + (hour * SPH) + (min * SPM));
}

/*
**  insert(): Perform an insertion sort.
*/
static TIMES *
insert(node, el)
TIMES *node, *el;
{
	TIMES *last = NULL;
	TIMES *root = node;

	DEB((stderr, "Inserting node.  time = %ld, msg = '%s'\n",
			el->time, el->message));
	while (node->next && (node->time < el->time))
	{
		last = node;
		node = node->next;
	}

	el->next = node;
	if (last)
		last->next = el;
	else
		root = el;

	return(root);
}

/*
**  traverse(): Dump the alarm list.
*/
traverse(alarms)
TIMES *alarms;
{
#ifdef DEBUG
	putc('\n', stderr);
	for ( ; alarms; alarms = alarms->next)
		fprintf(stderr, "alarm at time = %ld; msg = %s\n",
			alarms->time, alarms->message);
#endif DEBUG
}

#ifdef SUN
/*
**  iconic(): Query suntools to see if the window is iconic.
*/
static int
iconic()
{
	int ans;

	IOCTL(TIOCFLUSH, 0);
	printf(QUERY);
	scanf(RESPONSE, &ans);

	return(ans == ICONIC);
}
#endif SUN

/*
**  usage(): Error message.
*/
static void
usage(pname)
char *pname;
{
	printf("usage: %s [-h history] [alarmfile]\n", pname);
	exit(-1);
}
