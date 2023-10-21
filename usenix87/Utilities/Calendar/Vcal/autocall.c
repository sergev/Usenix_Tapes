/*
 * Module:	autocall.c
 *
 * Purpose:	notifies user at specified day and time of an
 *		appointment
 *
 * Author:	Mike Essex
 *
 * Date:	Sep 19, 1986
 *
 * Includes:
 *	time.h, stdio.h, signal.h
 *
 * Discussion:
 *	This program spawns background processes for each of the
 *	current date's entries in the user's ~/.appointments file.
 *      At the specified time of day the process outputs the
 *	message associated with the entry and dies.
 * 
 *
 * Edit History
 * ==== =======
 *
 * Date      Who	What
 * ----      ---	----------------------------------------------
 * 12/1/86   me		added multi-datafile capability
 * 12/1/86   me		changed 'home' from *home[80] to home[80]
 * 12/1/86   me		changed int maxentries to #define MAXENTRIES
 *
 */

#include <time.h>
#include <stdio.h>
#include <signal.h>

#define		MAXENTRIES 1000

int		monthdata[MAXENTRIES];
int		daydata[MAXENTRIES];
int		yeardata[MAXENTRIES];
int		timedata[MAXENTRIES];
char		*msgdata[MAXENTRIES];
int		month,day,year,hour,mhour,min,sec,dow,ampm;
int		cntr;
int		argvindex;


/*
 * Procedure:	main()
 *
 * Function:	initializes variables, calls data load, calculates
 *		sleep time for message, forks message process
 */

main(argc,argv)

int	argc;
char	*argv[];

{
    int		cnt,timeout,status,nowtime;
    int		loaddata();

    signal(SIGINT,SIG_IGN);
    signal(SIGQUIT,SIG_IGN);

    if (argc > 1) {
	argvindex = 1;
    }
    else {
	argvindex = 0;
    }
    while (argvindex <= argc) {
	cntr = 0;
	if ((cnt = loaddata(argv[argvindex])) != 0 ) {
	    timeset();
	    while (cntr < cnt) {
		nowtime = (mhour * 60) + min;
		if ((monthdata[cntr] == month) && (daydata[cntr] == day)
		&& (yeardata[cntr] == year) && (timedata[cntr] > nowtime)) {
		    timeout = (timedata[cntr] - nowtime) * 60;
		    if ( timeout > 300 ) {
			timeout -= 300;
		    }
		    if ((status = fork()) == 0) {
			ringer(timeout,msgdata[cntr]);
			exit(0);
		    }
		    else {
			if (status < 0) {
			    printf("Error:  Cannot start new autocall process\n");
			}
		    }
		}
		cntr++;
	    }
	}
	argvindex++;
    }
    exit(0);
} /* main */



/*
 * Procedure:	timeset()
 *
 * Function:	sets current date and time
 *
 * Return Values:
 *	sets global varialbes to date and time
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
 * Procedure:	loaddata()
 *
 * Function:	load appointments data
 *
 * Return Values:
 *	loads various global arrays with appointment data
 *
 * Discussion:
 *	opens ~/.appointments file and inputs data, then puts
 *	it into appropriate arrays
 */

loaddata(datafile)

char	*datafile;
{
    char	basedata[80];
    char	tmpbuf[80];
    char	*getenv();
    FILE 	*fptr;
    char	home[80];
    int		msgtime,i,j,k,l,field;
    char	*malloc();

    i = 0;
    while (i < MAXENTRIES) {
	daydata[i] = 0;
	monthdata[i] = 0;
	yeardata[i] = 0;
	msgdata[i] = 0;
	i++;
    }

    if (argvindex == 0) {
	strcpy(home,getenv("HOME"));
	strcat(home,"/.appointments");
    }
    else {
	strcpy(home,datafile);
    }

    if ((fptr = fopen(home,"r")) != NULL) {
	i = 0;
	while((fgets(basedata,80,fptr) != NULL)) {

	    basedata[strlen(basedata)-1] = NULL;

	    j = 0;
	    k = 0;
	    field = 0;
	    while (basedata[j] != NULL ) {
                 
                if (basedata[j] != ',') {

		    tmpbuf[k++] = basedata[j];
		}
		else {
		    switch (field) {

			case 0 : {
			    tmpbuf[k] = NULL;
			    monthdata[i] = atoi(tmpbuf);
			    k = 0;
			    break;
			}
			case 1 : {
			    tmpbuf[k] = NULL;
			    daydata[i] = atoi(tmpbuf);
			    k = 0;
			    break;
			}
			case 2 : {
			    tmpbuf[k] = NULL;
			    yeardata[i] = atoi(tmpbuf);
			    k = 0;
			    break;
			}
			case 3 : {
			    tmpbuf[k] = NULL;
			    msgtime = atoi(tmpbuf);
			    timedata[i] = ((msgtime / 100 ) * 60) + (msgtime % 100);
			    k = 0;
			    break;
			}
		    }
		    field++;
		}
		j++;
	    }
	    tmpbuf[k] = NULL;
	    msgdata[i] = malloc(40);
	    strcpy(msgdata[i],tmpbuf);

	    if (i >= MAXENTRIES) {
		printf("Warning:  Over 1000 entries in %s file.  Data truncated.\n",datafile);
		break;
	    }
	    i++;
	}
	fclose(fptr);
    }
    else {
	printf("Error:  cannot open %s file.\n",datafile);
    }
    return(i);
} /* loaddata */


/*
 * Procedure:	ringer(delay,message)
 *
 * Function:	outputs appointment message at appropriate time
 *
 * Parameters:
 *	p1	- int - delay time in seconds
 *	p2	- char pointer - pointer to appointment message
 *
 * Discussion:
 *	Takes in delay time and sleeps until delay is completed.
 *	Then it outputs the message to standard out and dies.
 */

ringer(delay,message)

    int		delay;
    char	*message;
{
    sleep(delay);
    timeset();
    printf("\r\nTime %d:%02d %2s    ", hour, min, (ampm == 0 ? "AM" : "PM"));
    printf("Activity scheduled:  %s \r\n\n",message);
} /* ringer */
