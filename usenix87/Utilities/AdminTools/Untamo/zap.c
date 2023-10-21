#ifdef PUCC
#include <sys/types.h>
#endif PUCC
#include <utmp.h>

#ifdef BSD2_9
#include <wait.h>
#else BSD2_9
#include <sys/wait.h>
#endif BSD2_9

#include <sys/ioctl.h>
#include <sys/file.h>
#include "untamo.h"

#ifdef BSD2_9
#include <sys/param.h>
#define getdtablesize() NOFILE
#endif BSD2_9

extern char *malloc() , *ctime() , *strcpy();
time_t time();

/*
 * zap -- disconnect the person logged on to tty "dev".
 * 	  makes use of ioctl(2) to get a new controlling
 *	  terminal and the infinitely clever vhangup(2)
 * 	  to disconnect it.  
 */
zap(him)
struct user *him;
{
	static char message[] = "\n\n\007Logged out by the system.\n";
	int dts, td;
	time_t tempus;

	/*
	 * close all the child's descriptors 
	 */
	dts = getdtablesize();
	for ( dts--; dts>=0; dts--){
		(void) close(dts);
	}
	/*
	 * now tell him it's over, and disconnect.
	 */
#ifdef BSD2_9
	td = open(him->line, O_RDWR);
#else BSD2_9
	td = open(him->line, O_RDWR, 0600);
#endif BSD2_9
	(void) ioctl(td, TIOCSTART, (char *)0);
	if (td >= 0)  {
		(void) write(td, message, sizeof(message) );
		(void) fsync(td);
		(void) ioctl(td, TIOCFLUSH, (char *)0);
		td = vhangup();
		(void) time(&tempus);
		if ( (logfd = fopen(LOGFILE,"a")) != NULL)  {
			(void)fprintf(logfd,
			"%19.19s : %s on %s because %s, vhangup returned %d\n",
			ctime(&tempus), him->uid, him->line, 
			( (him->warned & IS_MULT) ? "multiple" : (him->warned & IS_IDLE ? "idle" : "session" )), td );
			(void)fclose(logfd);
		}
	} else {
		(void) time(&tempus);
		if ( (logfd = fopen(LOGFILE,"a")) != NULL)  {
			(void)fprintf(logfd,
			"%19.19s : couldn't open %s on %s\n",
			ctime(&tempus), him->line, him->uid );
			(void)fclose(logfd);
		}
	}
}


error(sb)
char *sb;
{
	if ( (logfd = fopen(LOGFILE,"a")) != NULL)   {
		(void)fprintf(logfd,"%s",sb);
		(void)fclose(logfd);
	}
}
