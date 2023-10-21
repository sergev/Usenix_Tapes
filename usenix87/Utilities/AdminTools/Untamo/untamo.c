#ifdef PUCC
#include <sys/types.h>
#include <rld.h>
#endif PUCC
#include <utmp.h>
#include <signal.h>
#include <sys/file.h>
#ifndef F_OK
#    define F_OK 0
#endif F_OK
#include <sys/ioctl.h>
#include "untamo.h"
#include <sys/stat.h>

#ifdef PUCC
#define PWIDLE	6	/*accounting bit for no idle time logout     */
#define PWMULT	7	/*accounting bit for multiple logins allowed */
#include <passwd.h>
struct usrpwd *getupnam();
#else PUCC
#include <pwd.h>
#endif PUCC

#include "y.tab.h"

struct user users[MAXUSERS];
struct user *pusers[MAXUSERS];

extern char *malloc(), *strcpy(), *ctime() , *strcat();
extern unsigned strlen();
extern time_t time();

struct qelem *rules, 
	     *session,
	     *exmpt;	/* lists for timeouts, session limits, and exemptions */

jmp_buf env_buf;
int sleeptime;		/* time to sleep between checks			  */
FILE *logfd;		/* log file file descriptor pointer		  */
int m_threshold;	/* number of users before multiple limits 	  */
int s_threshold;	/* number of users for session limits		  */
int warn_flags = IS_IDLE | IS_MULT | IS_LIMIT; 
			/* what sorts of warnings should be accepted    */

main(n_args, ppch_args)
	int n_args;
	char **ppch_args;
{
	struct utmp utmpbuf;
	struct stat statbuf;
#ifdef PUCC
	struct usrpwd *pswd;
#else PUCC
	struct passwd *pswd;
#endif PUCC
	struct user *user;
	char pathbuf[20];
	int utmptr, utmpfd;
	time_t conf_oldstamp;
	int userptr;
	int res, td;
	int new;		/* if the configuration file is new */
	time_t tempus;
	int finish(), wakeup();
	FILE *conffd, *freopen(), *fopen();
	/* command line flags */
	int fl_multiple = 1, fl_session = 1, fl_idle = 1;

	while( --n_args && *++ppch_args && **ppch_args == '-' ) {
		while( *++(*ppch_args) ){
			switch( **ppch_args ){
			case 'm': /* don't even think about multiple logins */
				fl_multiple = 0;
				break;
			case 'i': /* don't even think about idle timeouts */
				fl_idle = 0;
				break;
			case 's': /* don't even think about session limits */
				fl_session = 0;
				break;
			default:
				fprintf( stderr, 
				"untamo: bad flag -%c\n", **ppch_args );
				break;
			}
		}
	}

	if( !fl_multiple && !fl_idle && !fl_session ){
		/* do absolutely nothing!! */
		exit(0);
	}

#ifdef PUCC
	if ( access( "/flags/testsys" , F_OK ) == 0 ) 
	    exit(0);				/* dont run in test mode */
#endif PUCC
	(void) signal(SIGHUP,  SIG_IGN);	
	(void) signal(SIGQUIT, SIG_IGN);
	(void) signal(SIGINT,  SIG_IGN);
#ifdef BSD4_2
	(void) signal(SIGTTOU, SIG_IGN);
	(void) signal(SIGTSTP, SIG_IGN);
#endif BSD4_2

	(void) signal(SIGTERM, finish);
	(void) signal(SIGALRM, wakeup);
	conf_oldstamp = 1;			/* a very old stamp */
	/*
	 * set up new header nodes for each of the lists.
	 * The forw and back pointers must point to them
	 * selves so the system insque routine can be used
	 */
	rules = (struct qelem *) malloc( sizeof(struct qelem) );
	exmpt = (struct qelem *) malloc( sizeof(struct qelem) );
	session = (struct qelem *) malloc( sizeof(struct qelem) );
	rules->q_forw = rules->q_back = rules;
	exmpt->q_forw = exmpt->q_back = exmpt;
	session->q_forw = session->q_back = session;
	rules->q_item = session->q_item = exmpt->q_item = NULL;

	if ( (logfd = fopen(LOGFILE,"a")) > 0)  {
		(void) time(&tempus);
		(void) fprintf(logfd,"%24.24s  Untamo started\n",ctime(&tempus) );
		(void) fclose(logfd);
	} else {
		(void) fprintf( stderr , "Untamo: couldn't open log file: %s\n", LOGFILE );
		exit(1);
	}

	if ( (res = fork()) < 0)	{
		(void) fprintf(stderr,"Untamo: couldn't start\n");
	}
	if (res){  /* if the parent */
#ifdef DEBUG
		exit(res);
#else
		exit(0);
#endif DEBUG
	}

	/*
	 * lose our controlling terminal
	 */
#ifdef BSD2_9
	td = open("/dev/tty", O_RDWR);
#else BSD2_9
	td = open("/dev/tty", O_RDWR, 0600);
#endif BSD2_9
	if (td >= 0){
		(void) ioctl(td, TIOCNOTTY, (char *)0);
		(void) close( td );
	}

	/*
	 * now sit in an infinite loop and work
	 */

while (1){
	if ( stat(CONFIG,&statbuf) < 0)  {
		(void) error("Untamo: couldn't stat conf file");
		exit(1);
	}

	if ( statbuf.st_mtime > conf_oldstamp ) {
		conf_oldstamp = statbuf.st_mtime;

		if ( (conffd = freopen(CONFIG, "r", stdin)) < 0) {
			(void) error("Untamo: can't open configuration file");
			exit(1);
		}

		/*
		 * get rid of the old rules and exempt lists
		 */
		(void) freelist(rules);
		(void) freelist(exmpt);
		(void) freelist(session);
		m_threshold = 0;
		s_threshold = 0;
		/*
		 * now read the configuration file and set up the
		 * rules and exemption lists
		 */
		(void) yyparse();
		new = 1;
	} else
		new = 0;
#ifdef BSD2_9
	if ( (utmpfd = open(UTMP, O_RDONLY)) < 0) {
#else BSD2_9
	if ( (utmpfd = open(UTMP, O_RDONLY, 0)) < 0) {
#endif BSD2_9
		(void) error("Untamo: can't open /etc/utmp");
		exit(1);
	} /* } <-- to match ifdefed open... */

	utmptr = 0;
	userptr = 0;
	/*
	 * look through the utmp file, compare each entry to the users
	 * array to see if an entry has changed.  If it has, build a new
	 * record for that person, if it hasn't, see  if it is time to
	 * examine him again.
	 */
	while ( (res = read(utmpfd, (char *)&utmpbuf, sizeof(struct utmp)) ) > 0 ) {

		if (res != sizeof(struct utmp)) {
			(void) error("Untamo: error reading utmp file, continuing");
			continue;
		}
		(void) time(&tempus);
		if (utmpbuf.ut_name[0] != '\0')   {
			user = &users[utmptr];
			if ( !(strcmp(user->uid,utmpbuf.ut_name)) &&
			   (user->time_on == utmpbuf.ut_time) )	{
				if (new)
					(void) setlimits(utmptr);
				if (fl_idle && tempus > user->next) {
					(void) checkidle(utmptr);
				}
			} else {
				/*
				 * build a new record
				 */
				user->warned  = 0;
				(void) strcpy(pathbuf,DEV);
				(void) strcat(pathbuf,utmpbuf.ut_line);
#ifdef PUCC
				user->rld = findrld(pathbuf);
				(void) strcpy(user->clust, findcluster(user->rld));
#endif PUCC

				(void) strcpy(user->line, pathbuf);
				(void) stat(pathbuf,&statbuf);
				(void) strcpy(user->uid, utmpbuf.ut_name);

#ifdef PUCC
				pswd = getupnam(utmpbuf.ut_name);
				user->ugroup = pswd->up_gid;
#else PUCC
				pswd = getpwnam(utmpbuf.ut_name);
				user->ugroup = pswd->pw_gid;
#endif PUCC

				user->time_on = utmpbuf.ut_time;
				(void) setlimits(utmptr);

#ifdef PUCC
				if( pswd->up_flags & (1l << PWMULT ))
				    user->exempt |= IS_MULT;
				if( pswd->up_flags & (1l << PWIDLE ))
				    user->exempt |= IS_IDLE;
#endif PUCC
				user->next = tempus;
 			}
			pusers[userptr++] = user;
		}
		utmptr++;
	}

	(void) close(utmpfd);
	(void) fclose(conffd);

#ifdef PUCC
	/*
	** check session limits
	*/

	if( fl_session ){
		(void) chk_session(userptr);
	}
#endif PUCC

	/*
	** check for and warn multiple logins
	*/

	if( fl_multiple ){
		(void) chk_multiple(userptr);
	}

	/*
	** wait sleeptime minutes
	*/

	(void) sleep( (unsigned) sleeptime * 60);
    }
}

#ifdef PUCC
/*
 * chk_session( users ) 
 * find out how many people are on sds ports, 
 * and try to warn enough people to get below the threshold
 */

chk_session( n_users )
	register int n_users;
{
	register int which_user;
	time_t tempus;
	register int n_sds_ports = 0;
	static int fl_sessionlimits = 0;

	(void) time(&tempus);
	for( which_user = 0; which_user < n_users ; which_user++ ){
		if( pusers[which_user]->warned & IS_LIMIT ){
			(void) warn(which_user,IS_LIMIT);
		} else if( is_sds_port( pusers[which_user]->rld ) ){
			n_sds_ports++;
		}
	}

	if( n_sds_ports > s_threshold && !fl_sessionlimits ){
		(void) close( creat( "/flags/sessionlimits", 0600 ));
		fl_sessionlimits = 1;
	}

	if( n_sds_ports < s_threshold && fl_sessionlimits ){
		unlink( "/flags/sessionlimits" );
		fl_sessionlimits = 0;
	}
		
	while( n_sds_ports>s_threshold  && s_threshold>0 && which_user>0 ){
		which_user--;
		if( tempus-pusers[which_user]->time_on > pusers[which_user]->session 
		    && pusers[which_user]->session > 2*60 
		    && !(pusers[which_user]->warned & IS_LIMIT)  ){

			(void) warn(which_user,IS_LIMIT);
			n_sds_ports--;
		}
	}
}


int 
is_sds_port( rld_number )
	int rld_number;
{
	int fd, res;
	struct rld_data rdat;

	/*
	 * Get to the right place in /etc/rlds
	 * Complements of Jeff Smith...
	 */
	if( fd = open (RLD_FILE, O_RDONLY, 0) >= 0 ) {
		lseek (fd, (long) rld_number * sizeof (struct rld_data), L_SET);
		res = read (fd, (char *) &rdat, sizeof (struct rld_data));
		(void) close(fd);
		if (res == sizeof(struct rld_data) && rdat.rld_tio != -1) {
			return 1;
		}
	}
	return 0;
}
#endif PUCC

/*
 * chk_multiple -- given the number of users (i), warn any of 
 *	       them  that have multiple logins.  Calls qsort(3)
 *	       to sort them by id.
 */
chk_multiple(i)
int i;
{
	int j, comp(); 
	int match, skip = -1;
	int wait = 0;

	if( i < m_threshold && m_threshold > 0 ) { /* below threshold...*/
		return;
	}
	(void) qsort( (char *) pusers, i, sizeof(struct user *), comp);
	for (j=0; j<i-1; j++)	{
		/*
		 * if not all the multiple logins logged out,
		 * decide on one not to kill, clear his warned
		 * bit, and continue.  But don't look again until
		 * we have passed all the guys with the same login.
		 */
		if ( wait == 0 )  {
			match = 0;
			skip = decide(j, i, &wait);
		} else
			wait--;

		if ( ( (*pusers[j]).exempt & IS_MULT) || (j == skip) )  {
			continue;	/* he's exempt ! */
		}

		if ( !strcmp( (*pusers[j]).uid, (*pusers[j+1]).uid) )	{
			match = 1;
			(void) warn(j,IS_MULT);
		} else {
			if ( match )
				(void) warn(j,IS_MULT);
			match = 0;
		}
	}
}


/*
 * decide -- given a bunch of multiply logged on terminals that did
 * 	     not heed the warning, decide returns the index into the 
 *	     *pusers array of the user NOT to log off.  Wait is the
 *  	     number of ids that chk_multiple must skip before calling
 *	     decide again.  Admittedly this is gross, but it works.
 */
decide(j, num, wait)
int j, num, *wait;
{
	int i;
	int count = 1;
	int warned = 1;
	int skip;

	/* 
	 * look through the users and find how many 
	 * of login (*pusers[i]).uid are logged on
	 * and whether or not they have been warned
	 */
	for ( i=j; i<num; i++)  {
		if ( !((*pusers[i]).warned & IS_MULT) )
			warned = 0;

		if ( !strcmp( (*pusers[i]).uid, (*pusers[i+1]).uid) )
			count++;
		else
			break;
	}
	/*
	 * now, if there is a need to skip someone, do it
	 */
	*wait = count-1;
	if ( (warned) && (count > 1) )  {
		skip = j;
		/*
		 * set skip to the alpha-numerical least tty
		 */
		for(i=j+1; i<j+count; i++)
			if (strcmp((*pusers[skip]).line,(*pusers[i]).line)>0)
				skip = i;
		(*pusers[skip]).warned &= ~IS_MULT;
		return(skip);
	}
	return(-1);
}

	
/*
 * finish -- end Untamo
 */
finish()
{
	time_t tempus;
	FILE *logfd;

	(void) signal(SIGTERM, SIG_IGN);
	(void) time(&tempus);
	(void) unlink( "/flags/sessionlimits");
	if ( (logfd = fopen(LOGFILE,"a")) > 0)  {
		(void) time(&tempus);
		(void) fprintf(logfd,"%24.24s  Untamo killed.\n",ctime(&tempus) );
		(void) fclose(logfd);
	}
	exit(0);
}


/*
 * comp -- used by qsort to sort by id
 */
comp(h1, h2)
struct user **h1, **h2;
{
	return( strcmp((**h1).uid, (**h2).uid) );
}


/* 
 * checkidle -- given a user, see if we want to warn him about idle time.
 *		first check the exempt vector to see if he is exempt.
 */
#define min(a,b) ( (a)<(b)?(a):(b) )

checkidle(i)
int i;
{
	struct stat statbuf;
	time_t tempus;

	(void) time(&tempus);
	(void) stat(users[i].line,&statbuf);
#ifdef DEBUG
	{ static char debugprint[80];
	  sprintf(debugprint,"**debug: checkidle(%d); %d %d %x\n",
		i, users[i].session, users[i].idle, users[i].exempt);
	  error(debugprint);
	  sprintf(debugprint," *debug: %s %s\n", users[i].line, users[i].uid);
	  error(debugprint);
	}
#endif DEBUG
	if (( tempus - statbuf.st_atime) < users[i].idle ) {
		users[i].warned &= ~IS_IDLE;
	} else {
		if (users[i].idle > 2*60 && !(users[i].exempt & IS_IDLE)) {
			(void) warn(i,IS_IDLE);
		}
	}
	users[i].next = min( statbuf.st_atime + users[i].idle,
			     users[i].time_on + users[i].session );
}
