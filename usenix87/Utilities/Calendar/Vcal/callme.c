/*
 * Module:	callme.c
 *
 * Purpose:	prompts user at time of an appointment
 *
 * Author:	Mike Essex
 *
 * Date:	Sep. 16, 1986
 *
 * Includes:
 *	time.h, stdio.h, signal.h
 *
 * Discussion:
 *	Takes in time and message from commands line, sleeps an
 *	appropriate amount of time and then displays message.
 * 
 *
 * Edit History
 * ==== =======
 *
 * Date      Who	What
 * ----      ---	----------------------------------------------
 *
 */


#include <time.h>
#include <stdio.h>
#include <signal.h>

int		mindata,timedata;
char		msgdata[512];
int		month,day,year,hour,mhour,min,sec,dow,ampm;


/*
 * Procedure:	main.c
 *
 * Function:	parses command line, calculates delay and starts
 *		background process for message.
 */

main(argc,argv)

int		argc;
char		*argv[];

{
    int		i,timeout,status,nowtime;

    signal(SIGINT,SIG_IGN);
    signal(SIGQUIT,SIG_IGN);

    if (argc < 3) {
	printf("\r\n          Syntax error:  Incorrect number of arguments          \r\n");
	printf("          Syntax:  callme (time) (message)                        \r\n\n");
	exit(1);
    }

    timedata = atoi(argv[1]);
    mindata = (60*(timedata/100)) + (timedata % 100);
    i = 2;
    while (i < argc) {
        strcat(msgdata,argv[i++]);
	strcat(msgdata," ");
    }
    timeset();
    nowtime = (mhour * 60) + min;
    if ( mindata > nowtime) {
	timeout = (mindata - nowtime) * 60;
	if ( timeout > 300 ) {
	    timeout -= 300;
	}
	if ((status = fork()) == 0) {
	    ringer(timeout,msgdata);
	    exit(0);
	}
	else {
	    if (status < 0) {
		printf("* Cannot start new sleeper process *\n");
	    }
	}
    }
    else {
	printf("It is already past %d\n",timedata);
    }
    exit(0);
} /* main */



/*
 * Procedure:	timeset()
 *
 * Function:	loads current time
 *
 * Return Values:
 *	sets global variables for time and date
 */

timeset()

{
    struct	tm *localtime();

    struct tm *tp;		/* time structure */
    long	tloc;		/* number of seconds since 1970 */

    time(&tloc);	/* fills tloc */

    tp = localtime(&tloc);

    dow =	tp->tm_wday;
    year =	tp->tm_year;
    month =	tp->tm_mon + 1;
    day =	tp->tm_mday;
    hour = tp->tm_hour;
    mhour = tp->tm_hour;
    min = tp->tm_min;
    sec = tp->tm_sec;

    year += 1900;

    if (sec >= 30)
    {
	min += 1;	/* round up minutes */
	if (min == 60)
	{
	mhour += 1;
	hour += 1;
	min = 0;
	}
    }

    if (hour > 12)
    {
	hour -= 12;
    }
    if (mhour >= 12)
    {
	ampm = 1;
    }
    else {
	ampm = 0;
    }
} /* timeset */



/*
 * Procedure:	ringer(delay,message)
 *
 * Function:	prompts user with message at appropriate time
 *
 * Parameters:
 *	p1	- int - delay time
 *	p2	- character pointer - pointer to message
 * Discussion:
 *	sleeps specified amount of time, then outputs message and dies.
 */

ringer(delay,message)

    int		delay;
    char	*message;
{
    sleep(delay);
    timeset();
    printf("\r\n                                                                               \r\n"); 
    printf("Time %d:%02d %2s   ", hour, min, (ampm == 0 ? "AM" : "PM"));
    printf("Activity scheduled:  %-41.41s ",message);
    printf("\r\n                                                                               \r\n"); 
} /* ringer */
