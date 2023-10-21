
/*
 *	uptime.c -- Print how long the system has been up 
 *				System V Implmenentation
 *
 *	8/19/86	- Version 1.0
 *		(sjm@dayton)	Steven McDowall
 *
 *	9/19/86 - Version 1.1
 *	  Get boot time much faster from times() call
 *		(pej@cuuxb)		Paul Jatkowski
 *
 *	cc -O -o uptime uptime.c
 */

# include <sys/types.h>			/* system types */
# include <sys/param.h>			/* for HZ */
# include <sys/times.h>
# include <time.h>

#define	MINUTES	60
#define HOURS	(MINUTES * 60)
#define DAYS	(HOURS * 24)

int		memfd;

main(argc, argv)
int  argc;
char *argv[];
{
	time_t	uptime, times();
	struct  tm  *ts;
	struct	tms tbuf;
	void	ptime();

	
/* 	Get the number of clock cycles the system has been running
	and divide by the clock rate (HZ) to get seconds		 */

  	uptime = (times(&tbuf) / HZ);

/* 	Print out information in one of 2 formats			 */

	if (argc > 1)				/* assume we want boot date */
	{
		uptime = time((long *) 0) - uptime;
		ts = localtime(&uptime);
		printf("System was last booted on %s", asctime(ts));
	}
	else					/* we want old uptime format */
		ptime(uptime);

}
/* 	main 								*/
/* 	Print the time in a nice format 				*/

void ptime(secs)
time_t secs;
{
short	days, hours, minutes;

secs -= (days = secs / DAYS) * DAYS;
secs -= (hours = secs / HOURS) * HOURS;
secs -= (minutes = secs / MINUTES) * MINUTES;

printf("The system has been up for ");
if (days != 0)
	printf("%d %s ", days, (days == 1 ? "day" : "days"));
if (hours != 0)
	printf("%d %s ", hours, (hours == 1 ? "hour" : "hours"));
if (minutes != 0)
	printf("%d %s ", minutes, (minutes == 1 ? "minute" : "minutes"));
	printf("and %d %s.\n", secs, (secs == 1 ? "second" : "seconds"));
}


