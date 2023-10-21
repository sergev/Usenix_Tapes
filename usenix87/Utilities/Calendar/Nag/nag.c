/*----------------------------------------------------------------
 *
 * Nag.c -- annoying reminder service daemon
 *
 * Sun Aug 24 14:18:08 PDT 1986
 *
 * by Dave Brower, {amdahl, cbosgd, mtxinu, sun}!rtech!gonzo!daveb
 *
 * Copyright 1986, David C Brower.  All rights reserved.
 *
 * This is a preliminary version.  The final release will be offered
 * with fewer restrictions.
 *
 * Nag should be launched out of your .login or .profile.  It
 * periodically reads your ~/.nag file and executes commands
 * that can be used as reminders of upcoming events.  The environment
 * variable NAGFILE can be used to get input from something other than
 * the ~/.nag file.
 *
 * NAGFILE FORMAT:
 * ---------------
 *
 * The ~/.nag file should contain lines of the form:
 *
 *    status day time interval command
 *
 * where:
 *
 * status	is one of
 *		   1. '#' indicating a commented out reminder
 *		   2. ':' indicating a silenced reminder
 *		   3. ' ' for an activate reminder.
 *		Other values produce unpredicatable results.
 *
 * day		is one of:
 *		   1.  A date, "8/8/88", "8-Aug-88", etc., but no blanks.
 *		   2.  '*' for any day.
 *		   3.  A day, "Sun", "Mon", ...
 *		The last is presently unimplemented (sorry).
 *
 * time		is a time spec, "8AM", "23:00", etc., but no blanks
 *
 * interval	is a colon separated list of minutes after time at which
 *		   to execute the command, e.g.,
 *
 *			 -30:-15:0:5:10
 *
 *		   produces execution 30 and 15 minutes before the event,
 *		   at the time, and 5 and 10 minutes later.
 *
 * command	is a command to execute with /bin/sh.  Some shell variables
 *		are set for use in messages:
 *
 *			$pretime	-interval
 *			$posttime	interval
 *			$now		hh:mm of the current time
 *			$then		hh:mm of the parent event
 *
 * Blank lines are ignored.
 *
 * Example:
 *
 *	# don't forget to eat.
 *	 * 12:30PM 0 writebig "Lunch Time"
 *
 *	# Weekly warning that has been silenced.
 *      :Mon 3:00PM -30:-20:-10:-5:0 echo "^GStatus report in $time"
 *
 *	# Active Weekly warning.
 *	 Fri 1:30PM -20:-10:-5:0:5:10 echo "^GCommittee meeting in $time"
 *
 *	# One shot warning to call the east coast.
 *	 8/25/86 1:30PM -180:-120:-60:0:10:20 echo "^GCall DEC Marlblerow"
 *
 * NAG
 * ---
 *
 * Nag puts itself in the background, and exits when you logout.
 * Standard output and standard error go to your terminal.
 *
 * Each time it wakes up, it sees if the ~/.nag file has changed.
 * If so, it builds an event queue for lines without '#' comment symbols.
 *
 * Events that were not silenced with 'X' and were due before "now"
 * are executed.  If the event was the last for an entry in ~/.nag,
 * the file is edited to re-enable it from a gagged state.
 *
 * The program then sleeps for at most MINSLEEP minutes.
 *
 * OKOK
 * ----
 *
 * The "okok" program just edits ~/.nag and prepends an 'X' to lines
 * that need to be shut up.
 *
 * BUILD INSTRUCTIONS:
 * -------------------
 *
 *  cc -o nag [ -DSYS5 ] nag.c gdate.c
 *  ln nag okok
 *
 *  The code compiles for a BSD system by default.
 *
 * CAVEATS:
 * --------
 *
 * Sorry Christopher, it probably won't work if stty nostop is set.
 *
 */

# include <stdio.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <signal.h>
# include <pwd.h>
# include <ctype.h>

# ifdef SYS5
#	include		<string.h>
#	include		<time.h>
#	define		index		strchr
#	define		rindex		strrchr
# else
#	include		<strings.h>
#	include		<sys/time.h>
# endif

/*----------------
 *
 *	defines
 *
 */

# define DPRINTF	if(Debug) (void)fprintf

# define COMCHAR	'#'
# define SILCHAR	':'

# define HRSECS		3600L

# define CTIMELEN	32	/* length of a date/time string */

# define MINSLEEP	(5*60)
# define MAXARGS	5120	/* max arg/env size on System V */

# define TRUE		(1)
# define FALSE		(0)

# define min(a,b)	((a) < (b) ? (a) : (b))

/*----------------
 *
 *	typedefs and structure definitions
 *
 */

/*
 * A NAGLINE is a parsed entry from the .nag file.  We keep
 * a list of them representing the current file, so we can
 * write it back out easily.
 */

typedef struct nagline	NAGLINE;

struct nagline
{
    NAGLINE * next;		/* Next in the chain */
    int type;			/* COMMENT, SILENT, PENDING, BAD */
#	define UNKNOWN		0
#	define COMMENT		1
#	define SILENT		2
#	define PENDING		3
#	define BAD		4

    int errtype;		/* if type is BAD, cause of error */
#	define NOERR		0
#	define EMPTY		1
#	define DATEBAD		2
#	define NOTIME		3
#	define TIMEBAD		4
#	define NOINTERVALS	5
#	define NOCMD		6

    time_t atime;		/* absolute time of event */
    char *err;			/* string that caused the error */
    char *line;			/* the raw line, allocated */
    char *datestr;		/* the date string, allocated */
    char *timestr;		/* the time string, allocated */
    char *intstr;		/* extracted string of intervals, allocated */
    char *cmd;			/* extracted command to execute, allocated */
};

static
char *linetypes[] =
{
  "Unknown",
  "Comment",
  "Silent",
  "Pending",
  "Bad"
};

static
char *parserrs[] =
{
  "No error",
  "Empty line",
  "Bad date",
  "No time",
  "Bad time",
  "No intervals",
  "No command"
};


/*
 * An EVENT is an entry in the event queue.
 */

typedef struct event	EVENT;

struct event
{
    EVENT * next;		/* next event in chain */
    NAGLINE *lp;		/* the parent nagline */
    time_t etime;		/* absolute time of the event */
    int offset;			/* minutes difference with parent time */
};


/*----------------
 *
 *	File local variables
 *
 */

static char *Myname="";		/* name from argv[0] */
static time_t Now = 0;		/* absolute time of "now" */
static time_t Last = 0;		/* time last time we were awake */
static NAGLINE *Flist = NULL;	/* lines from the file */
static NAGLINE *Flast = NULL;	/* last line from the file */
static EVENT *Evq = NULL;	/* the global event queue */
static char Origlogin[20] = "";	/* login name when program started */
static char Nagfile[ 256 ] = ""; /* full path of the nag file */
static int Debug = FALSE;	/* debugging? */

static char Laststr[ CTIMELEN ]; /* ctime output for last time through */
static char Nowstr[ CTIMELEN ];	/* ctime output for this time through */

/*----------------
 *
 *	Forward and external function definitions
 *
 */

/* library defined */

extern char *getlogin();	/* login name in /etc/utmp */
extern char *getenv();		/* get an environment variable */
extern struct passwd *getpwuid(); /* passwd entry for this user */
extern time_t time();
extern struct tm *localtime();
extern char *fgets();
extern char *index();
extern char *rindex();
extern int sprintf();
extern void perror();		/* int on BSD? */
extern void qsort();		/* int on BSD? */
extern unsigned sleep();
extern void free();
extern void exit();
extern char *ctime();

/* gdate.c defined */

extern char *gdate();		/* date string to time buf struct */
extern char *gtime();		/* time string to time buf struct */
extern time_t tm_to_time();	/* time buf to secs past epoch	*/
extern char *dow[];		/* days of the week names */
extern int find();		/* unambiguous search of string tables */

/* forward function references */

# define forward	extern

forward void nagfile();
forward void setup();

forward int readf();
forward int editf();
forward int writef();

forward int parseline();
forward void zaplines();

forward void buildq();
forward void zapq();
forward void insq();
forward void addevents();
forward int timecmp();
forward void sortq();
forward void runq();

forward void showlines();
forward void dumpline();

forward void showevents();
forward void dumpevent();

forward char *emalloc();
forward char *ecalloc();
forward FILE *efopen();

forward char *nctime();
forward char *nhour();
forward void delay();
forward void lowcase();

/*----------------
 *
 * main() -- Main program.
 *
 * Do one time setup, then go into a loop rebuilding the event queue,
 * executing events in order.  Sleep is done after running the queue.
 *
 */
/*ARGSUSED*/
main(argc, argv)
int argc;
char **argv;
{
    char *cp;

    if(argc > 1)
	Debug = TRUE;

    Myname = (cp = rindex(argv[0], '/')) ? cp + 1 : argv[0] ;
    nagfile();

    if( !strcmp(Myname, "nag") )
    {
	setup();

# ifndef FOREGROUND
	DPRINTF(stderr, "forking to self-backgrounnd");
	if(fork())
	    exit(0);
# endif
	/* pretend we started at the epoch */
	Now = 0;
	(void) strcpy( Nowstr, nctime( &Now ));

	/*
	 * This loop never exits.
	 *
	 * The program terminates in delay() when the user logs
	 * off this terminal.
	 */
	for(;;)
	{
	    (void) strcpy( Laststr, Nowstr );
	    Last = Now;

	    Now = time(NULL);
	    (void) strcpy( Nowstr, nctime( &Now ) );

	    DPRINTF(stderr, "\nLoop:\tLast %s\tNow %s\n", Laststr, Nowstr);

	    if ( readf() )
		buildq();

	    runq();
	}
    }
    else if ( !strcmp(Myname, "okok"))
    {
	Now = time( NULL );
	(void) strcpy( Nowstr, nctime( &Now ));

	if ( readf() )
	{
	    buildq();
	    if ( editf( PENDING ) )
		exit( writef() );
	}
	else
	{
	    (void) fprintf(stderr, "%s: Can't read %s\n", Myname, Nagfile );
	    exit(1);
	}
    }
    else
    {
	(void) fprintf(stderr, "Identity crisis: \"%s\" bad program name\n",
		       argv[0]);
	exit(1);
    }
    exit(0);
    /*NOTREACHED*/
}

/*----------------
 *
 * nagfile -- get the full .nag file path
 *
 */
void
nagfile()
{
    register char *home;
    register char *cp;

    /* remember who you are to check for logout later */

    (void) strcpy(Origlogin, getlogin());

    /* expand the Nagfile name */

    if( cp = getenv("NAGFILE") )
	(void)strcpy( Nagfile, cp );
    else if( home = getenv("HOME") )
	(void) sprintf( Nagfile, "%s/.nag", home );
    else
    {
	(void) fprintf(stderr, "%s: HOME is not set\n", Myname );
	exit(1);
    }

    DPRINTF(stderr, "Origlogin %s, Nagfile %s\n", Origlogin, Nagfile);
}

/*----------------
 *
 * setup() -- one time initialization.
 *
 * Setup signals so we don't go away.
 * accidentally.
 *
 */
void
setup()
{
    if(!Debug)
    {
	(void) signal( SIGQUIT, SIG_IGN );
	(void) signal( SIGTERM, SIG_IGN );
# ifdef SIGTTOU
	(void) signal( SIGTTOU, SIG_IGN );
# endif
    }
}


/*----------------
 *
 * readf() -- read the nagfile and build in memory copy.
 *
 * Returns TRUE if the file was read.
 */
int
readf()
{
    register NAGLINE *lp;
    register FILE *fp;
    char line[ MAXARGS ];
    struct stat newstat;
    static struct stat laststat = { 0 };
    static time_t readtime = 0;

    /* check to see if Nagfile has changed, and reread file. */

    if(stat(Nagfile, &newstat))
    {
	/* set it the epoch, but don't complain */
	newstat.st_mtime = 0;
    }

    /* if file changed, or we read it more than 12 hours ago */

    if ( newstat.st_mtime <= laststat.st_mtime
	|| (readtime && Now > 0 && readtime < (Now - (HRSECS * 12))))
    {
	DPRINTF(stderr, "already read %s\n", Nagfile );
	return FALSE;
    }

    /* rebuild the internal copy of the file */

    DPRINTF(stderr, "reading Nagfile\n");

    laststat = newstat;
    readtime = Now;

    zaplines();

    /* warn, but don't fatal if file can't be opened this time through */

    if ( NULL==(fp = efopen(Nagfile, "r")))
	return FALSE;

    /* build the new incore copy */

    while( NULL != fgets( line, sizeof(line), fp ) )
    {
	/* Lose trailing newline */
	line[ strlen(line) - 1 ] = '\0';

	/*ALIGNOK*/
	lp = (NAGLINE *) ecalloc( sizeof(*lp), 1 );

	if( parseline( line, lp ) )
	{
	    if( lp->type == BAD )
		DPRINTF(stderr, "Parsed OK: %s\n", lp->line );
	    else
		DPRINTF(stderr, "Parsed OK: %s %s %s %s\n",
			lp->datestr,
			lp->timestr,
			lp->intstr,
			lp->cmd );
	}
	else
	{
	    (void) fprintf(stderr, "%s: Can't parse line:\n%s\n%s %s\n",
			   Myname,
			   lp->line,
			   parserrs[ lp->errtype ],
			   lp->err );
	}

	if( !Flist )
	    Flist = lp;
	if( Flast )
	    Flast->next = lp;
	Flast = lp;
    }
    (void) fclose(fp);

    if(Debug)
    {
	(void) fprintf(stderr, "Read file OK\n");
	showlines( "\nLines after file read in:\n" );
    }

    return TRUE;
}


/*----------------
 *
 * editf() -- interactively edit the nag file in memory, then write it out.
 *
 * Used by 'okok' to make PENDING events SILENT; can also be used to
 * make SILENT events PENDING.
 *
 * Goes WAY out of it's way to force i/o to be on the terminal.
 *
 * Returns TRUE if lines were changed.
 */
int
editf( what )
register int what;
{
    register FILE *ifp;
    register FILE *ofp;
    register NAGLINE *lp;
    register EVENT *ep;
    register int changed = FALSE;

    char buf[ 80 ];

    if( ( ifp = efopen( "/dev/tty", "r" ) ) == NULL )
	return( changed );

    if( ( ofp = efopen( "/dev/tty", "w" ) ) == NULL )
	return( changed );

    setbuf( ofp, NULL );	/* force output to be unbuffered */

    for( lp = Flist; lp ; lp = lp->next )
    {
	if( lp->type == what )
	{
	    /* only display events on the queue within 12 hours */

	    for( ep = Evq; ep && ep->lp != lp; ep = ep->next )
		continue;

	    if( !ep || ep->etime > Now + (HRSECS * 12) )
		continue;

	    (void) fprintf( ofp, "Silence %s: %s (y/n/q)? ",
	    		    lp->timestr, lp->cmd ) ;

	    if( fgets( buf, sizeof(buf), ifp ) == NULL )
		break;

	    if( buf[ 0 ] == 'y' || buf[ 0 ] == 'Y' )
	    {
		lp->type = ( what == PENDING ) ? SILENT : PENDING;
		changed = TRUE;
	    }

	    /* stop querying if a 'q' is entered */

	    if( buf[ 0 ] == 'q' || buf[ 0 ] == 'Q' )
	    	break;
	}
    }
    (void) fclose( ifp );
    (void) fclose( ofp );
    return ( changed );
}


/*----------------
 *
 * writef() -- Write the file back out after a change.
 *
 * Returns TRUE if file wrote OK.
 */
int
writef()
{
    char buf[ 80 ];

    register int err;
    register FILE *fp;
    register NAGLINE *lp;

    DPRINTF(stderr, "Writing %s\n", Nagfile );

    if( ( fp = efopen( Nagfile, "w" ) ) == NULL )
	return (FALSE);

    err = 0;
    for( lp = Flist; lp && err >= 0 ; lp = lp->next )
    {
	switch( lp->type )
	{
	case BAD:
	case COMMENT:
	    err = fprintf( fp, "%s\n", lp->line );
	    break;
	default:
	    err = fprintf( fp, "%c%s %s %s %s\n",
			  lp->type == SILENT ? SILCHAR : ' ',
			  lp->datestr,
			  lp->timestr,
			  lp->intstr,
			  lp->cmd );
	    break;
	}
    }

    if( err < 0 )
    {
	DPRINTF( stderr, "err %d\n", err );
	(void) sprintf( buf, "%s: error writing %s", Myname, Nagfile );
	perror( buf );
    }
    else if( (err = fclose( fp ) ) < 0 )
    {
	(void) sprintf( buf, "%s: error closing %s", Myname, Nagfile );
	perror( buf );
	return( FALSE );
    }
    return ( err >= 0 );
}


/*----------------
 *
 * parseline() -- Split text into a NAGLINE more amenable to processing.
 *
 *	Returns TRUE with the NAGLINE all set up if parsed OK.
 *	Returns FALSE with the line->type set to BAD,
 *		      and line->errtype set if undecipherable.
 *
 *
 *	in the code, buf points to the first character not processed,
 *                   cp points to the last character examined.
 *
 *               cp places nulls in likely places.
 *
 *	This is a very ugly function and should be rewritten.
 */
int
parseline( buf, lp )
register char *buf;
register NAGLINE *lp;
{
    register char *cp;
    register int  today;
    register int	i;
    time_t	d;
    time_t	t;
    int		anyday;	
    struct tm ntm;		/* now tm struct */
    struct tm dtm;		/* date tm struct */
    struct tm ttm;		/* time tm struct */

    anyday = FALSE;
    lp->line = strcpy( emalloc( strlen( buf ) + 1 ), buf );

    /*
     * determine line type, and advance buf to first non-blank after
     * the status field
     */

    switch (*buf)
    {
    case COMCHAR:
	lp->type = COMMENT;
	return TRUE;
	/*NOTREACHED*/

    case SILCHAR:
	lp->type = SILENT;
	buf++;
	break;

    default:
	lp->type = PENDING;
	break;
    }

    /* skip to non-whitespace */

    while( *buf && isspace(*buf))
	buf++;

    /* empty line isn't fatal (it's a comment) */

    if (!*buf) {
	lp->type = BAD;
	lp->errtype = EMPTY;
	lp->err = buf;
	return TRUE;
    }

    /* bracket the day/date, and null terminate it */

    for( cp = buf; *cp && !isspace( *cp ); cp++ )
	continue;
    if( *cp ) *cp++ = '\0';
    else *cp = '\0';

    /* cp now positioned at char past null, or on null at the end */

    /*
     * buf points at the day field; figure out the
     * absolute time of "Midnight" of the right day for the event.
     */
    lp->datestr = strcpy( emalloc( strlen( buf ) + 1 ), buf );

    /* figure when midnight of today was */

    ntm = *localtime( &Now );
    ntm.tm_sec = 0;
    ntm.tm_min = 0;
    ntm.tm_hour = 0;

    if (*buf == '*')
    {
	anyday = TRUE;
	dtm = ntm;
    }
    else
    {

	/* parse date */

	if( NULL != gdate( buf, &dtm ) )
	{
	    DPRINTF(stderr, "not a date, maybe a day\n");

	    /* maybe it's a day name... */

	    lowcase( buf );
	    if( (i = find( buf, dow )) >= 0 )
	    {
		i--;
		today = ntm.tm_wday;
		DPRINTF(stderr, "today %s, event %s\n",
			dow[ today ],
			dow[ i ] );
		if( i < today )
		    i += 7;	/* it's next week */
		d = Now + (( i - today ) * HRSECS * 24 );
		dtm = *localtime( &d );
		dtm.tm_sec = 0;
		dtm.tm_min = 0;
		dtm.tm_hour = 0;
	    }
	    else
	    {
		DPRINTF(stderr, "find of %s in dow returned %d\n", buf, i );
		lp->type = BAD;
		lp->errtype = DATEBAD;
		lp->err = buf;
		return FALSE;
	    }
	}
    }

    d = tm_to_time( &dtm );
    DPRINTF(stderr, "parseline: date %s\n", nctime(&d) );

    /* advance to time */

    for( buf = cp ; *buf && isspace(*buf); buf++)	/* skip blanks */
	continue;

    if (!*buf) {
	lp->type = BAD;
	lp->errtype = NOTIME;
	lp->err = buf;
	return FALSE;
    }

    /* bracket the time */

    for( cp = buf; *cp && !isspace( *cp ); cp++ )
	continue;
    if( *cp ) *cp++ = '\0';
    else *cp = '\0';

    /*
     * buf now at time field, figure offset until event,
     * then fill in absolute time.
     *
     * gtime can't fail -- it will say it's 00:00 if it
     * doesn't understand.
     */
    DPRINTF(stderr, "parseline: time buf %s\n", buf );
    lp->timestr = strcpy( emalloc( strlen( buf ) + 1 ), buf );
    (void) gtime( buf, &ttm );
    t = (ttm.tm_hour * HRSECS) + (ttm.tm_min * 60);
    lp->atime = d + t;

    /*
    ** If past the event, and it's for any day, do it tomorrow. 
    ** BUG:  This breaks if there is an interval after the event
    ** This is a rare case, and I haven't yet thought of a clean fix.
    */
    if( anyday && lp->atime < Now )
    	lp->atime += HRSECS * 24;

    DPRINTF(stderr, "parseline: time offset %s is %d seconds, %02d:%02d\n",
	    buf, t, t / HRSECS, t % HRSECS );
    DPRINTF(stderr, "parseline: etime %s\n", nctime(&lp->atime));

    /* advance to intervals */

    for( buf = cp; *buf && isspace(*buf); buf++)
	continue;

    if (!*buf)
    {
	lp->type = BAD;
	lp->errtype = NOINTERVALS;
	lp->err = buf;
	return FALSE;
    }

    /* bracket the intervals */

    for( cp = buf; *cp && !isspace( *cp ); cp++ )
	continue;
    if( *cp ) *cp++ = '\0';
    else *cp = '\0';

    /* save the interval string. */

    lp->intstr = strcpy( emalloc( strlen( buf ) + 1 ), buf );

    /* take rest of the line as the command */

    if (!*cp)
    {
	lp->type = BAD;
	lp->errtype = NOCMD;
	lp->err = strcpy( emalloc ( strlen( cp ) + 1 ), cp );
	return FALSE;
    }

    lp->cmd = strcpy( emalloc ( strlen( cp ) + 1 ), cp );

    return TRUE;
}


/*----------------
 *
 * zaplines() -- delete all NAGLINEs and free their space
 *
 */
void
zaplines()
{
    register NAGLINE *lp;
    register NAGLINE *nlp;

    for( lp = Flist; lp ; lp = nlp )
    {
	nlp = lp->next;

	if( lp->line )
	    free(lp->line);
	if( lp->datestr )
	    free(lp->datestr);
	if( lp->timestr )
	    free(lp->timestr);
	if( lp->intstr )
	    free(lp->intstr);
	if( lp->cmd )
	    free(lp->cmd);

	free( lp );
    }
    Flast = Flist = NULL;
}


/*----------------
 *
 * buildq() --  Rebuild the event queue if the .nag file has changed.
 *
 */
void
buildq()
{
    register NAGLINE *lp;

    DPRINTF(stderr, "buildq: rebuilding the event queue\n");

    zapq();

    for( lp = Flist; lp; lp = lp->next )
    {
	/* add events for silenced lines too. */
	if( lp->type != COMMENT )
	    addevents( lp );
    }

    sortq();

    if(Debug)
	showevents( "Event queue after rebuild and sort\n" );
}


/*----------------
 *
 * zapq() -- Destroy an event queue, setting the head back to NULL.
 *
 * Only the actual element is freed.
 */
void
zapq()
{
    register EVENT *this;
    register EVENT *next;

    for ( this = Evq; this ; this = next )
    {
	next = this->next;
	free( this );
    }
    Evq = NULL;
}

/*----------------
 *
 * insq() -- Add a new EVENT to the head of a queue.
 *
 */
void
insq( etime, offset, lp )
time_t etime;
register int offset;
NAGLINE *lp;
{
    register EVENT *ep;

    etime += (offset * 60);

    /* add events after last time we ran, but no more than 24 hours
       in the future */

    if( ( etime >= Now || ( Last && etime > Last ) )
       && etime < ( Now + ( HRSECS * 24 ) ) )
    {
	DPRINTF(stderr, "insq: Adding %s at %s\n", lp->cmd, nctime(&etime) );
    }
    else			/* too late */
    {
	DPRINTF(stderr, "insq: Dropping %s at %s\n", lp->cmd, nctime(&etime) );
	return;
    }

    /*ALIGNOK*/
    ep = (EVENT *) emalloc( sizeof(*ep) );
    ep->etime = etime;
    ep->offset = offset;
    ep->lp = lp;

    /* splice into the head of the queue */
    ep->next = Evq;		/* NULL, if last event */
    Evq = ep;
}


/*----------------
 *
 * addevents() -- Add pending events for the NAGLINE to the queue.
 *
 * Events in the past are not considered.
 * If the command has been silenced, don't do the command.
 *
 */
void
addevents( lp )
register NAGLINE *lp;
{
    register char *cp;		/* ptr into the interval string */
    int offset;			/* offset in minutes */

    /* for every numeric value in the interval string... */

    for( cp = lp->intstr; cp && *cp ; cp = index( cp, ':' ) )
    {
	if (*cp == ':')		/* skip past optional ':' */
	    cp++;
	if (!*cp)		/* ignore trailing ':' */
	    return;

	/* read (possibly) signed interval value */

	if( 1 != sscanf( cp, "%d", &offset ) )
	{
	    (void) fprintf(stderr, "%s: bad intervals '%s'\n", Myname,
			   lp->intstr );
	    return;
	}
	insq( lp->atime, offset, lp );
    }
}



/*----------------
 *
 * timecmp() -- Compare time of two events.
 *
 * Made slightly tricky since it must return an int, not a time_t.
 *
 */
int
timecmp( a, b )
register EVENT **a;
register EVENT **b;
{
    time_t val = (*a)->etime - (*b)->etime;

    return( val < 0 ? -1 : val > 0 );
}


/*----------------
 *
 * sortq() -- Sort the event queue into chronological order.
 *
 * 1. Create an array of pointers to the events in the queue.
 * 2. Sort the array by time of the pointed-to events.
 * 3. Rebuild the queue in the order of the array.
 *
 */
void
sortq()
{
    register unsigned int n;	/* number of events in the queue */
    register unsigned int i;	/* handy counter */
    register EVENT **events;	/* allocated array of EVENT ptrs */
    register EVENT **ap;	/* ptr into allocated events */
    register EVENT *ep;		/* pointer in event chain */

    forward int timecmp();

    n = 0;
    for( ep = Evq; ep; ep = ep->next )
	n++;

    DPRINTF(stderr, "sortq:  %d events\n", n );

    if ( n < 2 )
	return;

    /* build array of ptrs to events */

    /*ALIGNOK*/
    ap = events = (EVENT **) ecalloc( (unsigned)sizeof(**ap), n );

    /* build array of ptrs to events */
    for( ep = Evq; ep; ep = ep->next )
	*ap++ = ep;

    /* sort by ascending time */
    (void) qsort( events, (unsigned)n, sizeof(*events), timecmp );

    /* rechain the event queue from the sorted array */
    Evq = ep = events[0];
    for ( i = 0 ; i < n ; )
    {
	ep->next = events[i++];
	ep = ep->next;
    }
    ep->next = NULL;

    free( events );
}


/*----------------
 *
 * runq() -- Execute all events that are due.
 *
 * Sleep until the next scheduled event.  If there are none, or
 * next is far away, sleep for MINSLEEP and try again.
 *
 */
void
runq()
{
    char cmd[ 5120 ];
    char now[ CTIMELEN ];
    register EVENT *evq;	/* standin for global Evq in loop */
    register EVENT *ep;		/* next event */
    register NAGLINE *lp;
    int dsecs;

    DPRINTF(stderr, "runq start at %s\n", Nowstr );

    evq = Evq;			/* fast access, be sure to save back */

    /*
     * Execute commands that are due.
     *
     * Keeps head of the queue current by cutting out events as
     * they are processed.
     *
     * The loop breaks out when the queue is gobbled up,
     * or we get to an event that is not due now.
     */

    while( evq && evq->etime <= Now )
    {
	lp = evq->lp;

	DPRINTF(stderr, "due at %s:\n", nctime( &evq->etime ) );

	/* Run a PENDING event */

	if( lp->type == PENDING && lp->cmd )
	{
	    (void)strcpy( now, &Nowstr[ 11 ] );
	    now[ 5 ] = '\0';

	    (void)sprintf( cmd, "pretime=%d;posttime=%d;now=%s;then=%s;%s\n",
			  -evq->offset,
			  evq->offset,
			  now,
			  nhour( &lp->atime ),
			  lp->cmd );

	    DPRINTF(stderr, "executing:\n%s\n", cmd );
	    if( system( cmd ) )
		(void) fprintf( stderr, "%s: Trouble running\n'%s'\n",
			       Myname, cmd );
	}

	/* if it's a SILENT event, is it time to make it PENDING? */

	if( lp->type == SILENT )
	{
	    /* find the queue end or the next event for the line */

	    for( ep = evq->next ; ep && ep->lp != lp ; ep = ep->next )
		continue;

	    /* if match, or it was the last in the queue, turn it on */

	    if ( ep )
	    {
		DPRINTF(stderr, "SILENT event\n");
	    }
	    else
	    {
		DPRINTF(stderr, "Last SILENT event, making PENDING again.\n");
		lp->type = PENDING;

		/*
		 * if the write fails, keep going and hope the user fixes
		 * the nag file.  If we exit, the daemon would need
		 * to be restarted by hand.  Since it won't do anything
		 * but sleep and exit when the user logs off, no harm
		 * is done by sticking around.
		 */
		(void) writef();
	    }
	}
	ep = evq->next;
	free( evq );
	evq = ep;
    }				/* for events on the queue */

    dsecs = evq ? min( evq->etime - Now, MINSLEEP) : MINSLEEP;

    DPRINTF(stderr, "sleeping for %d seconds, next %s\n",
	    dsecs,
	    evq ? nctime( &evq->etime ) : "never" );

    Evq = evq;			/* back to global var */

    delay( dsecs );
}


/*----------------
 *
 * emalloc() -- malloc with error msg.
 *
 */
char *
emalloc( size )
register int size;
{
    register char *ptr;
    extern char *malloc();

    if ( ( ptr = malloc( (unsigned) size ) ) == NULL )
    {
	(void) fprintf(stderr, "%s: Can't malloc %d bytes\n", Myname, size );
	exit(1);
    }
    return( ptr );
}

/*----------------
 *
 * ecalloc() -- calloc with error message.
 *
 */
char *
ecalloc( n, size )
register unsigned int n;
register unsigned int size;
{
    register char *ptr;
    extern char *calloc();

    if ( ( ptr = calloc( (unsigned) size, n ) ) == NULL )
    {
	(void) fprintf(stderr, "%s: Can't calloc %d bytes\n", Myname, size * n);
	exit(1);
    }
    return( ptr );
}

/*
 * efopen()  -- fopen with error message on failure (no fatal error)
 */
FILE *
efopen( file, mode )
char *file;
char *mode;
{
    char buf [ 80 ];
    register FILE * fp;

    if( (fp = fopen( file, mode )) == NULL )
    {
	(void)sprintf( buf, "%s: can't open file %s with mode \"%s\"",
		      Myname, file, mode );
	perror( buf );
    }
    return( fp );
}


/*
 * showline() -- Dump the line list.
 */
void
showlines( msg )
char *msg;
{
    register NAGLINE *lp;

    (void) fprintf(stderr, "%s", msg );
    for( lp = Flist; lp ; lp = lp->next )
	dumpline( lp );
}

/*
 * dumpline() -- dump a NAGLINE for debugging.
 */
void
dumpline( lp )
register NAGLINE *lp;
{
    if( lp == NULL )
    {
	(void) fprintf(stderr, "dumpline: NULL lp\n");
	return;
    }
    (void) fprintf(stderr, "\nline (%s):\n%s\n", linetypes[ lp->type ],
		   lp->line );
    switch( lp->type )
    {
    case BAD:
	(void) fprintf(stderr, "%s %s\n", parserrs[ lp->errtype ], lp->err );
	break;

    case PENDING:
    case SILENT:
	(void) fprintf(stderr, "The event is at %s\n", nctime( &lp->atime ));
    }
}

/*
 * showevents() -- dump the event list, for debugging.
 */
void
showevents( msg )
char *msg;
{
    register EVENT *ep;

    (void) fprintf(stderr, "%s", msg );
    for( ep = Evq; ep; ep = ep->next )
	dumpevent( ep );
}

/*
 * dumpevent() -- print an event, for debugging.
 */
void
dumpevent( ep )
register EVENT *ep;
{
    if( ep == NULL )
	(void) fprintf(stderr, "dumpevent: NULL ep\n");
    else
	(void) fprintf(stderr, "event 0x%x, next 0x%x offset %d time %s\n",
		       ep, ep->next, ep->offset, nctime(&ep->etime) );
}

/*
 * nctime() -- ctime with trailing '\n' whacked off.
 */
char *
nctime( t )
time_t *t;
{
    register char *cp;

    cp = ctime( t );
    cp[ strlen( cp ) - 1 ] = '\0';
    return ( cp );
}

/*
 * nhour() -- return an hh:mm string given a pointer to a time_t.
 */
char *
nhour( t )
time_t *t;
{
    register char *buf = ctime( t );

    /*
     * 012345678901234567890123
     * Wed Dec 31 16:00:00 1969
     */

    buf[ 16 ] = '\0';
    return ( &buf[ 11 ] );
}


/*----------------
 *
 * delay() -- like sleep but knows what 0 means.
 *
 * If user logs out, notices and exit with OK status.
 *
 */
void
delay( secs )
int secs;
{
    char thislogin[20];

    if( secs > 0)
    {
	(void) sleep( (unsigned) secs );
	(void) strcpy(thislogin, getlogin());
	if ( strcmp(Origlogin, thislogin) )
	    exit(0);
    }
}

/*
 * lowcase() -- make a string all lower case.
 */
void
lowcase( s )
char *s;
{
    while ( *s )
    {
	if( isupper( *s ) )
	    *s = tolower( *s );
	s++;
    }
}

# if 0

/*
 * dumptm() -- show contents of a tm structure.
 */
dumptm( tm )
struct tm *tm;
{
    (void) fprintf(stderr, "year : %d month: %d day: %d\n",
		   tm->tm_year,tm->tm_mon,tm->tm_mday);
    (void) fprintf(stderr, "day of month: %d hour: %d minute: %d second: %d\n",
		   tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec) ;
    (void) fprintf(stderr, "day of year: %d day of week: %d dst: %d\n",
		   tm->tm_yday, tm->tm_wday, tm->tm_isdst) ;
}

# endif

/* end of nag.c */


