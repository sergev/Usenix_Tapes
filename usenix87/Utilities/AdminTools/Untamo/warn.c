#include <signal.h>
#include <sys/ioctl.h>
#include "untamo.h"
#include "y.tab.h"

extern char *malloc() , *strcpy() , *ctime();
extern unsigned strlen();

/*
 * warn -- warn a user that he is logged in multiply, or has
 *	   been idle past his limit.  Uses the magic of setjmp
 *	   and longjmp to avoid a timeout on a terminal write.
 */
warn(i,type)
int i, type;
{
	int res;
	int opened;
	struct user *him;
	FILE *termf;

	if( !(type & warn_flags )) { /* we are not doing this warning now.. */
		return 0;
	}

	if (type == IS_IDLE )
		him = &users[i];
	else  /* type == IS_MULT || type == IS_LIMIT */
		him = pusers[i];

	if ( (res=vfork()) < 0)	{
		(void) error("couldn't fork in warn");
		exit(0);
	}
	/* 
	 * the parent returns after it
	 * has modified the global data structures
	 * that the child obviously can't
	 */
	if (res)   {
		if ( (type == IS_MULT) && !(him->warned & IS_MULT) )
			him->warned |= IS_MULT;
		if ( (type == IS_IDLE) && !(him->warned & IS_IDLE) )
			him->warned |= IS_IDLE;
#ifdef PUCC
		if ( (type == IS_LIMIT) && !(him->warned & IS_LIMIT) )
			him->warned |= IS_LIMIT;
#endif PUCC
		return( wait(0) );
	}
	
	/*
	 * child continues here
	 */
	if (setjmp(env_buf) == 0) {
		opened = 0;
		(void) alarm(5);
		if ((termf = fopen( him->line, "w")) != NULL)  {
			opened = 1;
			/*
			 *  start the terminal if stopped
			 */
			(void) ioctl(fileno(termf),TIOCSTART,(char *) 0);
			if (type == IS_MULT)  {
				if (him->warned & IS_MULT)  {
					(void) zap( him );
				} else {
				(void)fprintf(termf,"\007\r\n\r\nThis user id is ");
				(void)fprintf(termf,"logged on more than ");
				(void)fprintf(termf,"once, please end\r\n");
				(void)fprintf(termf,"all but one of your ");
				(void)fprintf(termf,"logins in the next");
				(void)fprintf(termf," %1d minutes\r\n",
						       sleeptime);
				(void)fprintf(termf,"or you will be logged ");
				(void)fprintf(termf,"out by the system.\r\n\r\n\007");
				}
			}
			else if (type == IS_IDLE)  {
				if( him->warned & IS_IDLE )	{
					(void) zap( him );
				} else {
				(void)fprintf(termf,
				"\007\r\n\r\nThis terminal has been idle %2d ",
				him->idle/60);

				(void)fprintf(termf,
				"minutes.  If it remains\r\nidle for %1d ",
				sleeptime);

				(void)fprintf(termf,
				"minutes it will be logged out by the system.");
				(void)fprintf(termf,"\r\n\r\n\007");
				}
			}
			else if (type == IS_LIMIT)  {
				if( him->warned & IS_LIMIT )	{
					(void) zap( him );
				} else {
				(void)fprintf(termf,
				"\007\r\n\r\nThis terminal has been in use %2d",
				him->session/60);

				(void)fprintf(termf, " minutes.\nIn %1d ",
				sleeptime);

				(void)fprintf(termf,
				"minutes it will be logged out by the system.");
				(void)fprintf(termf,"\r\n\r\n\007");
				}
			}
			(void) fclose(termf);
			opened = 0;
		} 
		(void) alarm(0);
	} else {
		/* we timed out */

		if ( opened ) {
		   /* free FILE without write()   */
			termf->_ptr = termf->_base;
			(void) fclose(termf);
		}
	}
	exit(0);	/* child exits here */
	return(0);	/* lint doesnt believe in exit()... */
}


/*
 * wakeup -- signal handler for SIGALRM
 */
wakeup()
{
	(void) longjmp(env_buf, 1);
}
