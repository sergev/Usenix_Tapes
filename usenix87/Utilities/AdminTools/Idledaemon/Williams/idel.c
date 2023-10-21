/* idles deamon - kills off users who have not used their terminals for
 * a specified time.
 * Tested and run for 6 month on 4.2 BSD at site Erix.
 * Change LMESSAGE, WMESSAGE, DAYOUY, NIGHTO, EVENING, MORNING and SLEEP
 * to what you want locally (see comments in code) compile and run (as root).
 * preferably started in /etc/rc.local by lines something like this:
 *	if [ -f /usr/local/lib/id ]; then
 *		/usr/local/lib/id & echo -n ' idle'	>/dev/console
 *	fi
 * when starting local daemons.
 * The daemon wakes up every SLEEP minutes, reads UTMP and stats the terminals
 * to see if when they were last used. A warning (WMESSAGE) is sent if the 
 * time elapsed excedes time WARN minutes. The user is logged out and a message
 * (LMESSAGE) is sent if the user has been idle for more than DAYOUT minutes
 * between the hours MORNING to EVENING or else NIGHTO minutes. Error messages
 * are sent to syslog LOG_CRIT. A log of users logged out is sent to syslog
 * LOG_INFO. Useful for defending system managers from irate users who do 
 * something stupid and log themselves out and then blame the daemon!
 */
#include <utmp.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sgtty.h>
#include <syslog.h>
#include <setjmp.h>
#include <signal.h>
#include <strings.h>
#define UTMP "/etc/utmp"
#define LMESSAGE "\007\007\rAuto logout - too long idle time\r\n"
#define WMESSAGE "\007\007\rThe idle deamon has spotted you\r\n"
#define DAYOUT 480 /* logout after this number of idle minutes - daytime*/
#define NIGHTO 120 /* logout after this number of idle minutes - nightime */
#define MORNING 7 /* start of daytime */
#define EVENING 18 /* start of evening time */
#define WARN 90 /* start sending warnings after this time */
#define SLEEP 10 /* time (minutes) between each check */

jmp_buf env;
struct utmp ut;
struct stat sbuf;
struct timeval tv;
struct timezone tz;
struct tm *ltime;
struct stat sbuf;
main()
{
	int fd, minutes, logout;
	char fname[12];
#ifndef DEBUG
	if (fork() != 0) exit(0); /* kill off parent */
	/* close all files */
	fd = getdtablesize();
	for ( fd--; fd>=0; fd--){
		(void) close(fd);
	}
	/* create files for std{in,out,err} if they are used by syslog or some 
	   such stupidity */
	if ((fd = open("/dev/null", O_RDWR)) < 0 ) exit(1);
	if ((fd = open("/dev/null", O_RDWR)) < 0 ) exit(1);
	if ((fd = open("/dev/null", O_RDWR)) < 0 ) exit(1);
	/* now remove the controling tty (if there is one) */
	if ( (fd = open("/dev/tty", O_RDWR)) >= 0 ) {
		(void) ioctl(fd, TIOCNOTTY, 0);
		(void) close(fd);
	}
#endif
	
	while (1) {
		/* open the utmp file */
		if ((fd = open(UTMP, O_RDONLY, 0)) < 0) {
			syslog(LOG_CRIT, "Idles can't open utmp");
			exit(1);
		}
		/* get the time */
		if (gettimeofday(&tv, &tz) != 0) {
			syslog(LOG_CRIT, "Idles can't get timeofday");
			exit(1);
		}
		ltime = localtime(&tv.tv_sec);
		/* work out the logout time limit */
		if ((ltime->tm_hour > MORNING) && (ltime->tm_hour < EVENING)) {
			logout = DAYOUT;
		}
		else logout = NIGHTO;
		
		/* read the utmp file, record by record until finished */
		while (read(fd, &ut, sizeof(struct utmp)) == sizeof(struct utmp)) {
			if (ut.ut_line[0] != '\0' && ut.ut_name[0] != '\0') {
				/* get the last time of use of the terminal by
				   stat-ing the terminal */
				strcpy(fname, "/dev/");
				strcat(fname, ut.ut_line);
				if (stat(fname, &sbuf) != 0) {
					perror("idles: stat: ");
					continue;
				}
				/* calculate the number of minutes since the terminal was last
				   acessed */
				minutes = (tv.tv_sec - sbuf.st_atime ) / 60;
				/* kill him off or warn him if he hasn't used his terminal */
				if (minutes >= logout) {
					zap(fname, LMESSAGE, 0);
					syslog(LOG_INFO, "Auto logout %s %s ",
					  ut.ut_name, ut.ut_line);
				}
				else if (minutes >= WARN) {
					zap(fname, WMESSAGE, 1);
				}
			}
		}
		(void) close(fd);
		/* sleep until its time to start again */
		sleep(SLEEP * 60);
	}
}
/* kill or warn a user */
zap(fname, message, warn)
char *fname, *message;
{
	int allarmed();
	struct sgttyb ttyb;
	int tfd;
	if (fork() == 0) {
		/* child kills the offender */
		/* close all files */
		tfd = getdtablesize();
		for ( tfd--; tfd>=0; tfd--){
			(void) close(tfd);
		}
		/* now remove the controling tty (if there is one) */
		if ( (tfd = open("/dev/tty", O_RDWR)) >= 0 ) {
			(void) ioctl(tfd, TIOCNOTTY, 0);
			(void) close(tfd);
		}
		/* this open will also set a new controling terminal */
		tfd = open(fname, O_RDWR, 0600) ;
		if (tfd < 0) exit(1);
		/* setup a timeout if against all odds we get hung */
		signal(SIGALRM,allarmed);
		alarm(2);
		if (setjmp(env) == 0) {
			/* do this to prevent ^S from hanging the process */
			(void) ioctl(tfd, TIOCSTART, (char *)0);
			/* toggle cbreak to disrupt any terminal paging */
			(void) ioctl(tfd, TIOCGETP, &ttyb);
			ttyb.sg_flags ^= CBREAK;
			(void) ioctl(tfd, TIOCSETP, &ttyb);
			ttyb.sg_flags ^= CBREAK;
			(void) ioctl(tfd, TIOCSETP, &ttyb);
			
			/* write out the logout message */
			write(tfd, message, strlen(message));
		
			/* flush it out for safety's sake */
			(void) ioctl(tfd, TIOCFLUSH, (char *)0);
		}
		alarm(0);
		(void) close(tfd);
		/* kill off the offender's login and exit */
		if (!warn) vhangup();
		exit(0);
	}
	/*parent: wait for the forked process*/
	while (wait(0) == 0);
}

/* just longjmp out if timed out (SIGALRM received) */
allarmed(sig)
int sig;
{
longjmp(env,1);
}

